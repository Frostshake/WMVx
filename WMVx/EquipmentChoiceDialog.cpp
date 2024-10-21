#include "stdafx.h"
#include "EquipmentChoiceDialog.h"
#include "core/database/WOTLKGameDatabase.h"
#include <QtConcurrent>

using namespace core;

EquipmentChoiceDialog::EquipmentChoiceDialog(GameDatabase* db, CharacterSlot slot, std::optional<core::CharacterItemWrapper> existing, QWidget *parent)
	: QDialog(parent), characterSlot(slot), existing_item(existing)
{
	ui.setupUi(this);
	gameDB = db;

	ui.lineEditSearch->setDisabled(true);
	ui.listWidgetChoices->setDisabled(true);
	ui.pushButtonChoose->setDisabled(true);
	ui.groupBoxItemQuality->setDisabled(true);

	delayedSearch = new Debounce(this);
	connect(delayedSearch, &Debounce::triggered, [&]() {
		auto search = ui.lineEditSearch->text();

		bool has_search = !search.isEmpty();
		bool is_id_search;
		uint32_t id_search = search.toUInt(&is_id_search);
		std::vector<core::ItemQualityId> active_quality_filters;

		for (const auto& filter_pair : itemQualityFilters) {
			if (filter_pair.second->isChecked()) {
				active_quality_filters.push_back(filter_pair.first);
			}
		}

		for (auto i = 0; i < ui.listWidgetChoices->count(); i++) {
			QListWidgetItem* target_item = ui.listWidgetChoices->item(i);
			auto  item_data = variantToItem(target_item->data(Qt::UserRole));

			bool match_quality = std::any_of(active_quality_filters.begin(), active_quality_filters.end(), [&target_item, &item_data](core::ItemQualityId filter) {
				return filter == item_data.second;
			});

			bool show = true;

			if (has_search) {
				if (is_id_search) {
					show = id_search == item_data.first;
				}
				else {
					show = target_item->text().contains(search, Qt::CaseInsensitive);
				}
			}

			show = show && match_quality;

			target_item->setHidden(!show);
		}
	});

	std::for_each(core::Mapping::itemQualityString.cbegin(), core::Mapping::itemQualityString.cend(), [&](const auto& mapped_quailty) {

		QCheckBox* check = new QCheckBox(this);
		check->setChecked(true);
		check->setText(mapped_quailty.second);
		connect(check, &QCheckBox::stateChanged, delayedSearch, &Debounce::absorb);

		ui.groupBoxItemQuality->layout()->addWidget(check);
		itemQualityFilters[mapped_quailty.first] = check;
	});

	connect(ui.pushButtonChoose, &QPushButton::pressed, [&]() {
		if (ui.listWidgetChoices->selectedItems().length() == 1 && ui.comboBoxDisplay->currentIndex() >= 0) {
			auto item = ui.listWidgetChoices->selectedItems()[0];
			auto entry = variantToItem(item->data(Qt::UserRole));

			auto* adaptor = gameDB->itemsDB->findById(entry.first);
			if (adaptor != nullptr) {
				uint32_t display_id = ui.comboBoxDisplay->currentText().toUInt();
				emit chosen(DialogChoiceMethod::NEW, characterSlot, CharacterItemWrapper::make(adaptor, gameDB, display_id));
			}
		}

		accept();
	});

	connect(ui.pushButtonRemove, &QPushButton::pressed, [&]() {
		emit chosen(DialogChoiceMethod::NEW, characterSlot, std::nullopt);
		reject();
	});

	connect(ui.listWidgetChoices, &QListWidget::itemSelectionChanged, [&]() {
		ui.comboBoxDisplay->clear();

		const bool enable = ui.listWidgetChoices->selectedItems().length() == 1;
		ui.pushButtonChoose->setDisabled(!enable);
		ui.comboBoxDisplay->setDisabled(!enable);

		if (!enable) {
			ui.comboBoxDisplay->clear();
		}
	});

	connect(ui.listWidgetChoices, &QListWidget::itemClicked, [&](const QListWidgetItem* item) {
		ui.comboBoxDisplay->clear();

		auto entry = variantToItem(item->data(Qt::UserRole));

		auto* adaptor = gameDB->itemsDB->findById(entry.first);
		if (adaptor != nullptr) {

			auto displays = adaptor->getItemDisplayInfoId();
			for (const auto& display_id : displays) {
				ui.comboBoxDisplay->addItem(QString::number(display_id));
			}

			if (displays.size() > 0) {
				ui.comboBoxDisplay->setCurrentIndex(0);
			}
		}
	});

	connect(ui.comboBoxDisplay, &QComboBox::currentIndexChanged, [&](int index) {
		const auto* item = ui.listWidgetChoices->currentItem();

		if (index >= 0 && item != nullptr) {
			auto entry = variantToItem(item->data(Qt::UserRole));

			auto* adaptor = gameDB->itemsDB->findById(entry.first);
			if (adaptor != nullptr) {
				uint32_t display_id = ui.comboBoxDisplay->currentText().toUInt();
				emit chosen(DialogChoiceMethod::PREVIEW, characterSlot, CharacterItemWrapper::make(adaptor, gameDB, display_id));
			}
		}
	});

	connect(ui.lineEditSearch, &QLineEdit::textChanged, delayedSearch, &Debounce::absorb);

	const auto& search_types = Mapping::CharacterSlotItemInventory.at(characterSlot);

	if (gameDB != nullptr) {

		ui.listWidgetChoices->setUpdatesEnabled(false);

		QtConcurrent::run([&, search_types]() {

			if (gameDB->itemsDB && gameDB->itemDisplayDB) {
				for (const auto* itemRecord : gameDB->itemsDB->all()) {
					if (std::find(search_types.begin(), search_types.end(), itemRecord->getInventorySlotId()) != search_types.end()) {
						auto temp = new QListWidgetItem(ui.listWidgetChoices);
						QString label = QString("%1 [%2]").arg(itemRecord->getName()).arg(itemRecord->getId());
						temp->setText(label);
						temp->setData(Qt::UserRole, itemToVariant(itemRecord));
						ui.listWidgetChoices->addItem(temp);
					}
				}
			}

			ui.listWidgetChoices->sortItems();

			QMetaObject::invokeMethod(this, [&] {
				ui.listWidgetChoices->setUpdatesEnabled(true);
				ui.listWidgetChoices->setDisabled(false);
				ui.lineEditSearch->setDisabled(false);
				ui.groupBoxItemQuality->setDisabled(false);
			});
		});
	}
	else {
		ui.listWidgetChoices->setDisabled(false);
	}	
}

EquipmentChoiceDialog::~EquipmentChoiceDialog()
{}


void EquipmentChoiceDialog::closeEvent(QCloseEvent* e) {
	emit chosen(DialogChoiceMethod::RESTORE, characterSlot, existing_item);
	QDialog::closeEvent(e);
}