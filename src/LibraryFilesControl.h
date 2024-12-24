#pragma once

#include <QWidget>
#include "ui_LibraryFilesControl.h"
#include "core/utility/Logger.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"
#include "Debounce.h"

class LibraryFilesControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	LibraryFilesControl(QWidget *parent = nullptr);
	~LibraryFilesControl();

	void onGameConfigLoaded(core::GameDatabase* db, core::GameFileSystem* fs, core::ModelSupport& ms) override;

private:

	struct FileTreeItem {
		QString fileName;
		QString displayName;

		bool operator<(const FileTreeItem& i) const {
			return fileName < i.fileName;
		}

		bool operator>(const FileTreeItem& i) const {
			return fileName > i.fileName;
		}

		QVariant toVariant() const {
			QList<QString> list;

			list.push_back(fileName);
			list.push_back(displayName);

			return QVariant(list);
		}

		static FileTreeItem fromVariant(QVariant variant) {
			const auto list = variant.toList();

			assert(list.count() == 2);

			FileTreeItem item;
			item.fileName = list[0].toString();
			item.displayName = list[1].toString();

			return item;
		}
	};

	Ui::LibraryFilesControlClass ui;

	void loadFiles();

	void tryLoadSubtree(QTreeWidgetItem* item, size_t depth = 0, QVector<FileTreeItem>* item_tree_src = nullptr);

	QVector<FileTreeItem> fileListSource;

	Debounce* delayedSearch;
	std::atomic<bool> loadingFiles;
};
