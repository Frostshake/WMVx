#include "../../stdafx.h"
#include "CharacterCustomization.h"

#include "../filesystem/GameFileSystem.h"
#include "../database/GameDatabase.h"
#include "../database/GameDataset.h"

#include "../modeling/Model.h"

#include "../database/DFRecordDefinitions.h"


namespace core {

	void LegacyCharacterCustomizationProvider::initialise(const CharacterDetails& details) {

		//TODO use constants for known keys

		const auto known_keys = {
			"Skin", "Face", "HairColor", "FacialColor", "HairStyle", "FacialStyle"
		};

		for (const auto& key : known_keys) {
			known_options[key] = 0;
		}

		auto filterCustomizationOptions = [&]<typename T>(const T * adaptor) -> bool {

			constexpr auto hasHD = requires(const T & t) {
				t.isHD();
			};

			if constexpr (hasHD) {
				if (adaptor->isHD() != details.isHd) {
					return false;
				}
			}

			return adaptor->getRaceId() == details.raceId &&
				adaptor->getSexId() == details.gender;
		};

		const auto matching_char_sections = gameDB->characterSectionsDB->where(filterCustomizationOptions);

		// load available customisations
		for (auto charSectionRecord : matching_char_sections) {

			//only checking for variationIndex 0 to avoid duplicates being included.
			auto section_type = charSectionRecord->getType();
			if (section_type == CharacterSectionType::Skin) {
				known_options["Skin"]++;
			}
			else if (section_type == CharacterSectionType::Face) {
				if (charSectionRecord->getVariationIndex() == 0) {
					known_options["Face"]++;
				}
			}
			else if (section_type == CharacterSectionType::Hair) {
				if (charSectionRecord->getVariationIndex() == 0) {
					known_options["HairColor"]++;
				}
			}
			else if (section_type == CharacterSectionType::FacialHair) {
				if (charSectionRecord->getVariationIndex() == 0 /* && charSectionRecord->getSection() == 0 */) {	//TODO check logic
					known_options["FacialColor"]++;
				}
			}
		}

		known_options["HairStyle"] += gameDB->characterHairGeosetsDB->count(filterCustomizationOptions);
		known_options["FacialStyle"] += gameDB->characterFacialHairStylesDB->count(filterCustomizationOptions);
	}

	void LegacyCharacterCustomizationProvider::reset() {
		known_options.clear();
	}

	bool LegacyCharacterCustomizationProvider::apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices) {
		model->characterCustomizationChoices = choices;

		//updating records...

		auto found = 0;

		auto filterCustomizationOptions = [&]<typename T>(const T * adaptor) -> bool {

			constexpr auto hasHD = requires(const T & t) {
				t.isHD();
			};

			if constexpr (hasHD) {
				if (adaptor->isHD() != details.isHd) {
					return false;
				}
			}

			return adaptor->getRaceId() == details.raceId &&
				adaptor->getSexId() == details.gender;
		};

		const auto matching_char_sections = gameDB->characterSectionsDB->where(filterCustomizationOptions);


		//TODO THIS CHECK CURRENTLY NOT WORKING FOR VANILLA
	//#ifdef _DEBUG
	//	{
	//		//sanity check available options - multiple hits on a variation indicate an issue with the underlying adaptor data.
	//
	//		// variation_index -> frequency
	//		auto unique_skins = std::map<uint32_t, size_t>();
	//		for (auto charSectionRecord : matching_char_sections) {
	//			if (CharacterSectionType::Skin == charSectionRecord->getType()) {
	//				unique_skins[charSectionRecord->getVariationIndex()]++;
	//			}
	//		}
	//
	//		for (const auto& skin : unique_skins) {
	//			assert(skin.second == 1);
	//		}
	//	}
	//#endif


		for (auto charSectionRecord : matching_char_sections) {
			auto section_type = charSectionRecord->getType();
			if (section_type == CharacterSectionType::Skin) {
				if (charSectionRecord->getVariationIndex() == choices.at("Skin")) {
					model->characterCustomization.skin = charSectionRecord;
					found++;
				}

			}
			else if (section_type == CharacterSectionType::Face) {
				if (charSectionRecord->getVariationIndex() == choices.at("Skin")) {
					if (charSectionRecord->getSection() == choices.at("Face")) {
						model->characterCustomization.face = charSectionRecord;
						found++;
					}
				}
			}
			else if (section_type == CharacterSectionType::Hair) {
				if (charSectionRecord->getVariationIndex() == choices.at("HairColor") &&
					charSectionRecord->getSection() == choices.at("HairStyle")) {
					model->characterCustomization.hairColour = charSectionRecord;
					found++;
				}
			}
			else if (section_type == CharacterSectionType::FacialHair) {
				if (charSectionRecord->getVariationIndex() == choices.at("HairColor") &&
					charSectionRecord->getSection() == choices.at("FacialColor")) {
					model->characterCustomization.facialColour = charSectionRecord;
					found++;
				}
			}

			if (found >= 4) {
				//exit early if all have been found.
				break;
			}
		}

		auto hair_style_index = 0;

		for (auto& hairStyleRecord : gameDB->characterHairGeosetsDB->where(filterCustomizationOptions)) {
			if (hair_style_index == choices.at("HairStyle")) {
				model->characterCustomization.hairStyle = hairStyleRecord;
				break;
			}
			hair_style_index++;
		}

		auto facial_style_index = 0;

		for (auto& facialHairStyleRecord : gameDB->characterFacialHairStylesDB->where(filterCustomizationOptions)) {
			if (facial_style_index == choices.at("FacialStyle")) {
				model->characterCustomization.facialStyle = facialHairStyleRecord;
				break;
			}
			facial_style_index++;
		}



		//TODO update model.

		//TODO validate cusotmizations.
	
		return true;
	}



	void ModernCharacterCustomizationProvider::initialise(const CharacterDetails& details) {
		auto* const cascFS = (CascFileSystem*)(gameFS);

		auto model_id = getModelIdForCharacter(details);
		
		//TODO handle error
		assert(model_id > 0);

		auto customs = DB2File<DFDB2ChrCustomizationRecord>("dbfilesclient/chrcustomization.db2");
		customs.open(cascFS);

		auto custom_opts = DB2File<DFDB2ChrCustomizationOptionRecord>("dbfilesclient/chrcustomizationoption.db2");
		custom_opts.open(cascFS);

		auto custom_choices = DB2File<DFDB2ChrCustomizationChoiceRecord>("dbfilesclient/chrcustomizationchoice.db2");
		custom_choices.open(cascFS);

		for (const auto& custom_section : customs.getSections()) {
			for (const auto& custom_row : custom_section.records) {
				if ((custom_row.data.sex == (uint32_t)details.gender || custom_row.data.sex == 3) &&
					(custom_row.data.raceId == details.raceId || custom_row.data.raceId == 0)) {

					const auto customization_str = customs.getString(custom_row.data.nameLang, &custom_section.view, custom_row.recordIndex, 0).toStdString();
					

					for (const auto& option_section : custom_opts.getSections()) {
						for (const auto& option_row : option_section.records) {
							if (option_row.data.chrCustomizationId == custom_row.data.id && option_row.data.chrModelId == model_id) {
								auto opt_str = custom_opts.getString(option_row.data.nameLang,
									&option_section.view,
									option_row.recordIndex,
									0).toStdString();

								int count = 0;

								for (const auto& choice_section : custom_choices.getSections()) {
									for (const auto& choice_row : choice_section.records) {
										if (choice_row.data.chrCustomizationOptionId == option_row.data.id) {

											// might be useful to include name in options...
						/*					auto choice_str = custom_choices.getString(choice_row.data.nameLang,
												&choice_section.view,
												choice_row.recordIndex,
												0);*/

											count++;
										}
									}
								}

								if (count > 0) {
									known_options[opt_str] = count;
								}
							}
						}
					}
				}
			}
		}
	}

	void ModernCharacterCustomizationProvider::reset() {
		known_options.clear();
	}

	bool ModernCharacterCustomizationProvider::apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices) {
		//TODO

		return true;
	}

	uint32_t ModernCharacterCustomizationProvider::getModelIdForCharacter(const CharacterDetails& details) {
		auto table = DB2File<DFDB2ChrRaceXChrModelRecord>("dbfilesclient/chrracexchrmodel.db2");
		table.open((CascFileSystem*)(gameFS));

		for (const auto& table_section : table.getSections()) {
			for (const auto& row : table_section.records) {
				if (row.data.raceId == details.raceId && row.data.sex == (uint32_t)details.gender) {
					return row.data.chrModelId;
				}
			}
		}

		return 0;
	}



}