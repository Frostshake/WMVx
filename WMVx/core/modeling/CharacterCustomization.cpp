#include "../../stdafx.h"
#include "CharacterCustomization.h"

#include "../filesystem/GameFileSystem.h"
#include "../database/GameDatabase.h"
#include "../database/GameDataset.h"

#include "../modeling/Model.h"
#include "../modeling/Scene.h"

#include "../database/DFRecordDefinitions.h"


namespace core {

	//TODO remove this (its a duplicate from CharacterControl)
	void setGeosetVisibility(Model* model, CharacterGeosets geoset, uint32_t flags)
	{
		//formula for converting a geoset flag into an id
		//xx1 id's look to be the default, so +1 gets added to the flags
		auto geoset_id = (geoset * 100) + 1 + flags;

		auto id_range_start = geoset * 100;
		auto id_range_end = (geoset + 1) * 100;

		assert(id_range_start <= geoset_id);
		assert(id_range_end >= geoset_id);

		if (model != nullptr) {
			for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
				auto& record = model->model->getGeosetAdaptors()[i];
				if (record->getId() == geoset_id) {
					model->visibleGeosets[i] = true;
				}
				else if (id_range_start < record->getId() && record->getId() < id_range_end) {
					model->visibleGeosets[i] = false;
				}
			}
		}
	}


	int32_t bitMaskToSectionType(int32_t mask) {
		if (mask == -1 || mask == 0) {
			return mask;
		}

		auto val = 1;

		while (((mask = mask >> 1) & 0x01) == 0)
			val++;


		return val;

	}



	bool CharacterCustomizationProvider::apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices) {
		model->characterCustomizationChoices = choices;
		return updateContext(details, choices);
	}

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
		context.reset();
	}

	bool LegacyCharacterCustomizationProvider::updateContext( const CharacterDetails& details, const ChrCustomization& choices) {

		//updating records...

		if (!context) {
			context = std::make_unique<Context>();
		}

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
					context->skin = charSectionRecord;
					found++;
				}

			}
			else if (section_type == CharacterSectionType::Face) {
				if (charSectionRecord->getVariationIndex() == choices.at("Skin")) {
					if (charSectionRecord->getSection() == choices.at("Face")) {
						context->face = charSectionRecord;
						found++;
					}
				}
			}
			else if (section_type == CharacterSectionType::Hair) {
				if (charSectionRecord->getVariationIndex() == choices.at("HairColor") &&
					charSectionRecord->getSection() == choices.at("HairStyle")) {
					context->hairColour = charSectionRecord;
					found++;
				}
			}
			else if (section_type == CharacterSectionType::FacialHair) {
				if (charSectionRecord->getVariationIndex() == choices.at("HairColor") &&
					charSectionRecord->getSection() == choices.at("FacialColor")) {
					context->facialColour = charSectionRecord;
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
				context->hairStyle = hairStyleRecord;
				break;
			}
			hair_style_index++;
		}

		auto facial_style_index = 0;

		for (auto& facialHairStyleRecord : gameDB->characterFacialHairStylesDB->where(filterCustomizationOptions)) {
			if (facial_style_index == choices.at("FacialStyle")) {
				context->facialStyle = facialHairStyleRecord;
				break;
			}
			facial_style_index++;
		}


		auto tmp_underwear = gameDB->characterSectionsDB->find([&](const CharacterSectionRecordAdaptor* adaptor) ->bool {
				return adaptor->getRaceId() == details.raceId &&
				adaptor->getSexId() == details.gender &&
				adaptor->getVariationIndex() == context->skin->getVariationIndex() &&
				adaptor->isHD() == details.isHd &&
				adaptor->getType() == CharacterSectionType::Underwear;
			});

		context->underwear = const_cast<CharacterSectionRecordAdaptor*>(tmp_underwear);
	
		return context->isValid();
	}

	bool LegacyCharacterCustomizationProvider::update(Model* model, CharacterTextureBuilder* builder, Scene* scene) {
		assert(context);

		std::shared_ptr<Texture> hairtex = nullptr;
		std::shared_ptr<Texture> furtex = nullptr;

		{
			const auto& skin = context->skin->getTextures();

			if (!skin[0].isEmpty()) {
				builder->setBaseLayer(skin[0]);
			}

			if (!skin[1].isEmpty()) {
				furtex = scene->textureManager.add(skin[1], gameFS);
			}

			if (model->characterOptions.showUnderWear) {
				if (context->underwear != nullptr) {
					const auto& underwear_skins = context->underwear->getTextures();
					if (!underwear_skins[0].isEmpty()) {
						builder->addLayer(underwear_skins[0], CharacterRegion::LEG_UPPER, 1);
					}
					if (!underwear_skins[1].isEmpty()) {
						builder->addLayer(underwear_skins[1], CharacterRegion::TORSO_UPPER, 1);
					}
				}
			}
		}


		auto& hairStyle = context->hairStyle;

		const auto hair_geoset_id = std::max(1u, hairStyle->getGeoset());
		for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
			if (model->model->getGeosetAdaptors()[i]->getId() == hair_geoset_id) {
				model->visibleGeosets[i] = model->characterOptions.showHair;
			}
		}

		{
			const auto& face = context->face->getTextures();

			if (!face[0].isEmpty()) {
				builder->addLayer(face[0], CharacterRegion::FACE_LOWER, 1);
			}

			if (!face[1].isEmpty()) {
				builder->addLayer(face[1], CharacterRegion::FACE_UPPER, 1);
			}
		}


		if (model->characterOptions.showFacialHair) {
			const auto& facial_geoset = context->facialStyle;

			//must be atleast 1, can be problematic if it doesnt get shown at all.
			//NOTE records ate in 100, 300, 200 order
			//TODO check logic, is the adaptor returing data in incorrect order?
			setGeosetVisibility(model, CharacterGeosets::CG_GEOSET100, facial_geoset->getGeoset100());
			setGeosetVisibility(model, CharacterGeosets::CG_GEOSET200, facial_geoset->getGeoset300());
			setGeosetVisibility(model, CharacterGeosets::CG_GEOSET300, facial_geoset->getGeoset200());

			if (context->facialColour != nullptr)
			{
				const auto& face_feature = context->facialColour->getTextures();

				if (!face_feature[0].isEmpty()) {
					builder->addLayer(face_feature[0], CharacterRegion::FACE_LOWER, 2);
				}

				if (!face_feature[1].isEmpty()) {
					builder->addLayer(face_feature[1], CharacterRegion::FACE_UPPER, 2);
				}
			}
		}

		{
			const auto& hair = context->hairColour->getTextures();
			
			if (!hair[0].isEmpty()) {
				hairtex = scene->textureManager.add(hair[0], gameFS);
			}

			if (!hairStyle->isBald())
			{
				if (!hair[1].isEmpty()) {
					builder->addLayer(hair[1], CharacterRegion::FACE_LOWER, 3);
				}


				if (!hair[2].isEmpty()) {
					builder->addLayer(hair[2], CharacterRegion::FACE_UPPER, 3);
				}
			}

			if (hairtex == nullptr) {
				//TODO need to use alternative texture?
			}
		}



		if (hairtex != nullptr) {
			model->replacableTextures[TextureType::HAIR] = hairtex;
		}
		else {
			model->replacableTextures.erase(TextureType::HAIR);
		}

		if (furtex != nullptr) {
			model->replacableTextures[TextureType::FUR] = furtex;
		}
		else {
			model->replacableTextures.erase(TextureType::FUR);
		}

		//TODO GAMEOBJECT1

		//TODO geosets	

		return true;
	}

	CharacterComponentTextureAdaptor* LegacyCharacterCustomizationProvider::getComponentTextureAdaptor(const CharacterDetails& details) {
		auto raceInfo = gameDB->characterRacesDB->findById(details.raceId);
		const bool is_hd_model = details.isHd;

		if (raceInfo != nullptr && raceInfo->getComponentTextureLayoutId(is_hd_model).has_value()) {
			const auto raceLayoutId = raceInfo->getComponentTextureLayoutId(is_hd_model).value();
			auto temp_componentAdaptor = gameDB->characterComponentTexturesDB->find([raceLayoutId](const CharacterComponentTextureAdaptor* componentAdaptor) -> bool {
				return componentAdaptor->getLayoutId() == raceLayoutId;
				});

			if (temp_componentAdaptor != nullptr) {
				return const_cast<CharacterComponentTextureAdaptor*>(temp_componentAdaptor);
			}
		}
		
		return nullptr;
	}

	ModernCharacterCustomizationProvider::ModernCharacterCustomizationProvider(GameFileSystem* fs, GameDatabase* db)
		: CharacterCustomizationProvider(),
		gameFS(fs), gameDB(db) {
		fileDataDB = dynamic_cast<FileDataGameDatabase*>(gameDB);
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

								std::vector<uint32_t> choice_ids;
								for (const auto& choice_section : custom_choices.getSections()) {
									for (const auto& choice_row : choice_section.records) {
										if (choice_row.data.chrCustomizationOptionId == option_row.data.id) {

											// might be useful to include name in options...
						/*					auto choice_str = custom_choices.getString(choice_row.data.nameLang,
												&choice_section.view,
												choice_row.recordIndex,
												0);*/

											count++;
											choice_ids.push_back(choice_row.data.id);
										}
									}
								}

								if (count > 0) {
									assert(count == choice_ids.size());
									known_options[opt_str] = count;
									cacheOptions[opt_str] = option_row.data.id;
									cacheChoices[option_row.data.id] = choice_ids;
								}
							}
						}
					}
				}
			}
		}

		assert(known_options.size() == cacheOptions.size());
		assert(known_options.size() == cacheChoices.size());
	}

	void ModernCharacterCustomizationProvider::reset() {
		known_options.clear();
		cacheOptions.clear();
		cacheChoices.clear();

		context.reset();
	}

	bool ModernCharacterCustomizationProvider::updateContext(const CharacterDetails& details, const ChrCustomization& choices) {
		
		auto* const cascFS = (CascFileSystem*)(gameFS);

		if (!context) {
			context = std::make_unique<Context>();
		}

		auto elements = DB2File<DFDB2ChrCustomizationElementRecord>("dbfilesclient/chrcustomizationelement.db2");
		elements.open(cascFS);


		auto geosets = DB2File<DFDB2ChrCustomizationGeosetRecord>("dbfilesclient/chrcustomizationgeoset.db2");
		geosets.open(cascFS);

		auto models = DB2File<DFDB2ChrCustomizationSkinnedModelRecord>("dbfilesclient/chrcustomizationskinnedmodel.db2");
		models.open(cascFS);

		auto materials = DB2File<DFDB2ChrCustomizationMaterialRecord>("dbfilesclient/chrcustomizationmaterial.db2");
		materials.open(cascFS);


		auto layers = DB2File<DFDB2ChrModelTextureLayerRecord>("dbfilesclient/chrmodeltexturelayer.db2");
		layers.open(cascFS);

		//TODO shouldnt need to fully reset contet each timne.
		context->geosets.clear();
		context->materials.clear();


		const auto textureLayoutId = getTextureLayoutId(details);


		for (const auto& choice : choices) {
			const auto option_id = cacheOptions[choice.first];
			const auto choice_id = cacheChoices[option_id][choice.second];


			for (const auto& element_section : elements.getSections()) {
				for (const auto& element_row : element_section.records) {
					if (element_row.data.chrCustomizationChoiceId == choice_id) {
						
						if (element_row.data.chrCustomizationGeosetId > 0) {
							auto* tmp = findRecordById(geosets, element_row.data.chrCustomizationGeosetId);
							if (tmp != nullptr) {
								context->geosets.push_back(*tmp);
							}
						}

						if (element_row.data.chrCustomizationSkinnedModelId > 0) {
							int a = 5;
							a++;
							//TODO
						//	auto* tmp = findRecordById(models, element_row.data.chrCustomizationSkinnedModelId);
						//	if (tmp != nullptr) {
						//		opt.models.push_back(*tmp);
						//		valid = true;
						//	}
						}

						if (element_row.data.chrCustomizationMaterialId > 0) {
							auto* tmp = findRecordById(materials, element_row.data.chrCustomizationMaterialId);
							if (tmp != nullptr) {

								Context::Material mat;
								mat.custMaterialId = tmp->data.id;
								mat.uri = findTextureFileByMaterialId(tmp->data.materialResourcesId);

								for (const auto& layer_section : layers.getSections()) {
									for (const auto& layer : layer_section.records) {
										//TODO does [1] need to be checked too?
										
										if (layer.data.chrModelTextureTargetId[0] == tmp->data.chrModelTextureTargetId &&
											layer.data.chrComponentTextureLayoutId == textureLayoutId) {

											mat.textureType = layer.data.textureType;
											mat.layer = layer.data.layer;
											mat.blendMode = layer.data.blendMode;
											mat.region = bitMaskToSectionType(layer.data.textureSectionTypeBitMask);

											context->materials.push_back(mat);
										}
									}
								}


								
							}
						}
					
					}
				}
			}

		}

		std::sort(context->materials.begin(), context->materials.end());

		//TODO sort materials

		return true;
	}

	bool ModernCharacterCustomizationProvider::update(Model* model, CharacterTextureBuilder* builder, Scene* scene) {
		assert(context);

		//TODO handle model->characterOptions.showFacialHair


		for (const auto& geo : context->geosets) {
			setGeosetVisibility(model, (core::CharacterGeosets)geo.data.geosetType, geo.data.geosetId);
		}

		for (const auto& mat : context->materials) {
			if (mat.region == -1) {	//TODO cracthyr base == 11?
				builder->pushBaseLayer(mat.uri);	
			}
			else {
				builder->addLayer(mat.uri, (core::CharacterRegion)mat.region, mat.layer, (BlendMode)mat.blendMode);
			}
		}

			
		

		//TODO models




		return true;
	}

	CharacterComponentTextureAdaptor* ModernCharacterCustomizationProvider::getComponentTextureAdaptor(const CharacterDetails& details) {

		const auto layoutId = getTextureLayoutId(details);

		auto temp_componentAdaptor = gameDB->characterComponentTexturesDB->find([layoutId](const CharacterComponentTextureAdaptor* componentAdaptor) -> bool {
			return componentAdaptor->getLayoutId() == layoutId;
			});

		if (temp_componentAdaptor != nullptr) {
			return const_cast<CharacterComponentTextureAdaptor*>(temp_componentAdaptor);
		}

		return nullptr;
	}

	uint32_t ModernCharacterCustomizationProvider::getTextureLayoutId(const CharacterDetails& details) {
		auto model_id = getModelIdForCharacter(details);

		//TODO handle error
		assert(model_id > 0);

		auto models_file = DB2File<DFDB2ChrModelRecord>("dbfilesclient/chrmodel.db2");
		models_file.open((CascFileSystem*)gameFS);

		for (const auto& section : models_file.getSections()) {
			for (const auto& row : section.records) {
				if (row.data.id == model_id) {
					return row.data.charComponentTextureLayoutID;
				}
			}
		}

		return 0;
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


	GameFileUri::id_t ModernCharacterCustomizationProvider::findTextureFileByMaterialId(uint32_t materialResId) {
		return fileDataDB->findByMaterialResId(materialResId);
	}
}