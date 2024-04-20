#include "stdafx.h"
#include "EffectChoiceDialog.h"
#include <QtConcurrent>

using namespace core;

EffectChoiceDialog::EffectChoiceDialog(GameDatabase* db, CharacterSlot slot, QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	gameDB = db;
	characterSlot = slot;

	ui.listWidgetChoices->setDisabled(true);
	ui.pushButtonChoose->setDisabled(true);

	connect(ui.pushButtonRemove, &QPushButton::pressed, [&]() {
		emit chosen(characterSlot, std::nullopt);
		reject();
	});

	connect(ui.listWidgetChoices, &QListWidget::itemSelectionChanged, [&]() {
		ui.pushButtonChoose->setDisabled(ui.listWidgetChoices->selectedItems().length() != 1);
	});

	connect(ui.pushButtonChoose, &QPushButton::pressed, [&]() {
		if (ui.listWidgetChoices->selectedItems().length() == 1) {
			auto item = ui.listWidgetChoices->selectedItems()[0];

			uint32_t enchant_id = item->data(Qt::UserRole).toUInt();

			emit chosen(characterSlot, enchant_id);
		}

		accept();
	});

	if (gameDB != nullptr) {

		// spell effects arent currently implemented for all expansions.
		if (gameDB->spellEnchantmentsDB == nullptr) {
			return;
		}

		ui.listWidgetChoices->setUpdatesEnabled(false);

		QtConcurrent::run([&]() {
			for (auto& enchant : gameDB->spellEnchantmentsDB->all()) {
				if (enchant->getItemVisualId() > 0) {
					auto temp = new QListWidgetItem(ui.listWidgetChoices);
					temp->setText(enchant->getName());
					temp->setData(Qt::UserRole, QVariant(enchant->getId()));
					ui.listWidgetChoices->addItem(temp);
				}
			}

			ui.listWidgetChoices->sortItems();

			QMetaObject::invokeMethod(this, [&] {
				ui.listWidgetChoices->setUpdatesEnabled(true);
				ui.listWidgetChoices->setDisabled(false);
			});
		});
	}
	else {
		ui.listWidgetChoices->setDisabled(false);
	}
}

EffectChoiceDialog::~EffectChoiceDialog()
{}
