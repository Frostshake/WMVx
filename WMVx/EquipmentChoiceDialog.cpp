#include "stdafx.h"
#include "EquipmentChoiceDialog.h"
#include "core/database/WOTLKGameDatabase.h"
#include <QtConcurrent>

using namespace core;

EquipmentChoiceDialog::EquipmentChoiceDialog(GameDatabase* db, CharacterSlot slot, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	gameDB = db;
	characterSlot = slot;

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
		if (ui.listWidgetChoices->selectedItems().length() == 1) {
			auto item = ui.listWidgetChoices->selectedItems()[0];
			auto entry = variantToItem(item->data(Qt::UserRole));

			for (const auto* adaptor : gameDB->itemsDB->all()) {
				if (adaptor->getId() == entry.first) {
					emit chosen(characterSlot, adaptor);
					break;
				}
			}
		}

		accept();
	});

	connect(ui.pushButtonRemove, &QPushButton::pressed, [&]() {
		emit chosen(characterSlot, nullptr);
		reject();
	});

	connect(ui.listWidgetChoices, &QListWidget::itemSelectionChanged, [&]() {
		ui.pushButtonChoose->setDisabled(ui.listWidgetChoices->selectedItems().length() != 1);
	});

	connect(ui.lineEditSearch, &QLineEdit::textChanged, delayedSearch, &Debounce::absorb);

	const auto& search_types = Mapping::CharacterSlotItemInventory.at(characterSlot);

	if (gameDB != nullptr) {

		ui.listWidgetChoices->setUpdatesEnabled(false);

		QtConcurrent::run([&, search_types]() {

			for (const auto* itemRecord : gameDB->itemsDB->all()) {
				if (std::find(search_types.begin(), search_types.end(), itemRecord->getInventorySlotId()) != search_types.end()) {
					auto temp = new QListWidgetItem(ui.listWidgetChoices);
					QString label = QString("%1 [%2]").arg(itemRecord->getName()).arg(itemRecord->getId());
					temp->setText(label);
					temp->setData(Qt::UserRole, itemToVariant(itemRecord)); 
					ui.listWidgetChoices->addItem(temp);
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
