#include "stdafx.h"
#include "LibraryFilesControl.h"
#include "core/filesystem/MPQFileSystem.h"
#include "core/modeling/Model.h"
#include <execution>
#include <QtConcurrent>

using namespace core;

LibraryFilesControl::LibraryFilesControl(QWidget* parent)
	: WidgetUsesScene(),
	WidgetUsesGameClient(),
	QWidget(parent)
{
	ui.setupUi(this);

	ui.treeWidgetFiles->setDisabled(true);

	connect(ui.treeWidgetFiles, &QTreeWidget::itemExpanded, this, [&](QTreeWidgetItem* item) {
		loadingFiles = true;
		tryLoadSubtree(item);
		loadingFiles = false;
	}, Qt::QueuedConnection);

	connect(ui.treeWidgetFiles, &QTreeWidget::itemActivated, [&](QTreeWidgetItem* item) {

		if (gameFS == nullptr) {
			return;
		}

		bool is_model = item->text(0).endsWith(".M2", Qt::CaseInsensitive);

		if (is_model) {
			FileTreeItem info = FileTreeItem::fromVariant(item->data(2, Qt::UserRole));

			if (scene != nullptr && gameDB != nullptr) {

				Log::message("Loading model: " + info.fileName);
				try {
					auto m = std::make_unique<Model>();
					m->initialise(info.fileName, modelSupport.m2Factory, gameFS, gameDB, scene->textureManager);

					Log::message("Vertices: " + QString::number(m->model->getVertices().size()));
					Log::message("Render Passes: " + QString::number(m->model->getRenderPasses().size()));
	
					scene->addModel(std::move(m));
				}
				catch (std::exception e) {
					Log::message("Exception caught loading model:");
					Log::message(e.what());
					QMessageBox::warning(this, 
						"Model Data Error", 
						QString("An error occured while loading model data. \n%1").arg(e.what()),
						QMessageBox::Ok);
				}
			}
		}
	});

	delayedSearch = new Debounce(this);
	connect(delayedSearch, &Debounce::triggered, [&]() {
		if (!loadingFiles) {
			auto search = ui.lineEditSearch->text();
			const auto search_len = search.length();

			int limit = fileListSource.size() > 100000 ? 4 : 3;

			if (search_len > limit) {
				loadingFiles = true;

				std::function<bool(QTreeWidgetItem*)> revealTree = [&revealTree, &search](QTreeWidgetItem* item) -> bool {
					bool visible_child = false;
					for (auto j = 0; j < item->childCount(); j++) {
						if (revealTree(item->child(j))) {
							visible_child = true;
						}
					}


					if (!visible_child) {
						if (item->text(0).contains(search, Qt::CaseInsensitive)) {
							visible_child = true;
						}
					}


					item->setExpanded(visible_child);
					item->setHidden(!visible_child);

					return visible_child;
				};

				for (auto i = 0; i < ui.treeWidgetFiles->topLevelItemCount(); i++) {
					QTreeWidgetItem* child = ui.treeWidgetFiles->topLevelItem(i);
					revealTree(child);
				}

				loadingFiles = false;
			}
			else if(search_len == 0) {
				
				QTreeWidgetItemIterator it(ui.treeWidgetFiles);
				while (*it) {
					(*it)->setExpanded(false);
					(*it)->setHidden(false);
					++it;
				}
	
			}
		}
	});

	connect(ui.lineEditSearch, &QLineEdit::textChanged, delayedSearch, &Debounce::absorb);

}

LibraryFilesControl::~LibraryFilesControl()
{}

void LibraryFilesControl::loadFiles() {

	loadingFiles = true;
	fileListSource.clear();

	QMetaObject::invokeMethod(this, [&] {
		ui.treeWidgetFiles->clear();
		ui.treeWidgetFiles->setUniformRowHeights(true);	//speed improvement

		ui.lineEditSearch->setDisabled(true);
		ui.treeWidgetFiles->setDisabled(true);
		ui.treeWidgetFiles->setUpdatesEnabled(false);
	});	

	// key is top level name, set is second level name
	std::map<QString, QSet<QString>> topLevelNames;

	{
		Log::message("Loading file list.");

		std::mutex mut;
		auto file_list = gameFS->fileList();

		fileListSource.reserve(file_list->size() / 10);

		std::for_each(std::execution::par, file_list->cbegin(), file_list->cend(), [&](const GameFileUri::path_t& file_path) {
			if (file_path.endsWith(".M2", Qt::CaseInsensitive)) {

				auto parts = file_path.split(gameFS->seperator());

				FileTreeItem item;
				item.fileName = file_path;
				item.displayName = parts.last();

				std::scoped_lock lock(mut);
				fileListSource.push_back(item);


				if (!topLevelNames.contains(parts[0])) {
					topLevelNames[parts[0]] = {};
				}

				if (parts.length() > 2) {
					topLevelNames[parts[0]].insert(parts[1]);
				}
			}
			
		});

		Log::message("Loaded file list.");
	}

	QMetaObject::invokeMethod(this, [&, topLevelNames = std::move(topLevelNames)] {
		for (const auto& topName : topLevelNames) {
			auto item = new QTreeWidgetItem(ui.treeWidgetFiles);
			item->setText(0, topName.first);
			item->setData(1, Qt::UserRole, false);
			ui.treeWidgetFiles->addTopLevelItem(item);

			for (const auto& innerName : topName.second) {
				auto inner_item = new QTreeWidgetItem(item);
				inner_item->setText(0, innerName);
				inner_item->setData(1, Qt::UserRole, false);
				item->addChild(inner_item);
			}

			item->sortChildren(0, Qt::AscendingOrder);

		}

		ui.treeWidgetFiles->sortItems(0, Qt::AscendingOrder);
	});


	QMetaObject::invokeMethod(this, [&] {
		ui.treeWidgetFiles->setUpdatesEnabled(true);
		ui.treeWidgetFiles->setDisabled(false);
		ui.lineEditSearch->setDisabled(false);
	});

	loadingFiles = false;
}

void LibraryFilesControl::tryLoadSubtree(QTreeWidgetItem* item, size_t depth, QVector<FileTreeItem>* item_tree_src)
{
	if (item_tree_src == nullptr) {
		item_tree_src = &fileListSource;
	}

	bool is_subtree_loaded = item->data(1, Qt::UserRole).toBool();

	if (!is_subtree_loaded) {

		const auto separator = gameFS->seperator();
		QString search_path = item->text(0) + separator;
		QTreeWidgetItem* parent_item = item->parent();
		while (parent_item != nullptr) {
			search_path.prepend(parent_item->text(0) + gameFS->seperator());
			parent_item = parent_item->parent();
		}
		
		//big speed up by only looking at fileListSource once, and using this for recursive calls.
		QVector<FileTreeItem> deep_sub_items;

		QVector<FileTreeItem> sub_items;
		QSet<QString> sub_names;
		std::mutex mut;

		std::for_each(std::execution::par, item_tree_src->cbegin(), item_tree_src->cend(), [&mut, &sub_items, &sub_names, &search_path, &separator, &deep_sub_items, depth](const FileTreeItem& tree_item) {
			if (tree_item.fileName.startsWith(search_path)) {
				QString right_path = tree_item.fileName.mid(search_path.length());
				std::scoped_lock lock(mut);
				auto parts = right_path.split(separator);
				if (parts.size() > 1) {
					sub_names.insert(parts.first());
					if (depth == 0) {
						deep_sub_items.push_back(tree_item);
					}
				}
				else {
					sub_items.push_back(tree_item);
				}
			}
			});
		
		QList<QTreeWidgetItem*> items_buffer;
		auto item_name_exists = [&item, &items_buffer](QString name) -> bool {
			for (auto i = 0; i < item->childCount(); i++) {
				if (item->child(i)->text(0) == name) {
					return true;
				}
			}
			
			for (auto tmp : items_buffer) {
				if (tmp->text(0) == name) {
					return true;
				}
			}

			return false;
		};
		

		for (const auto& sub_name : sub_names) {
			if (!item_name_exists(sub_name)) {
				auto ui_item = new QTreeWidgetItem();
				ui_item->setText(0, sub_name);
				ui_item->setData(1, Qt::UserRole, false);
				items_buffer.push_back(ui_item);
			}
		}

		for (const auto& sub_item : sub_items) {
			
			if (!item_name_exists(sub_item.displayName)) {
				assert(sub_item.displayName.endsWith(".M2", Qt::CaseInsensitive));
				auto ui_item = new QTreeWidgetItem();
				ui_item->setText(0, sub_item.displayName);
				ui_item->setData(1, Qt::UserRole, true);
				ui_item->setData(2, Qt::UserRole, sub_item.toVariant());
				
				items_buffer.push_back(ui_item);
			}
		}

		if (!items_buffer.isEmpty()) {
			item->addChildren(items_buffer);
			item->sortChildren(0, Qt::AscendingOrder);
		}
		
		QMetaObject::invokeMethod(this, [&] {
			if (depth < 1) {
				item->setData(1, Qt::UserRole, true);
				for (auto i = 0; i < item->childCount(); i++) {
					tryLoadSubtree(item->child(i), depth + 1, &deep_sub_items);
				}
			}
		});
	}
}


void LibraryFilesControl::onGameConfigLoaded(GameDatabase* db, GameFileSystem* fs, ModelSupport& ms)
{
	WidgetUsesGameClient::onGameConfigLoaded(db, fs, ms);

	if (gameDB != nullptr && gameFS != nullptr) {
		if (this->isVisible() && !loadingFiles) {
			QtConcurrent::run(std::bind(&LibraryFilesControl::loadFiles, this));
		}
	}
}
