#include "../../stdafx.h"
#include "CharacterCustomization.h"

#include "../filesystem/GameFileSystem.h"
#include "../database/GameDatabase.h"
#include "../database/GameDataset.h"

#include "../modeling/Model.h"
#include "../modeling/Scene.h"

#include "../game/GameClientAdaptor.h"

#include <ranges>
#include <functional>
#include <algorithm>


namespace core {

	int32_t bitMaskToSectionType(int32_t mask) {
		if (mask == -1 || mask == 0) {
			return mask;
		}

		auto val = 1;
		while (((mask = mask >> 1) & 0x01) == 0)
			val++;

		return val;
	}

	bool CharacterDetails::detect(const Model* model, const GameDatabase* gameDB, CharacterDetails& out) {

		if (!gameDB->characterRacesDB) {
			return false;
		}

		if (model->model->getModelPathInfo().isCharacter()) {
			const auto& path_info = model->model->getModelPathInfo();
			auto charRaceRecord = gameDB->characterRacesDB->find([&](const CharacterRaceRecordAdaptor* item) -> bool {
				auto recordName = item->getClientFileString();
				return recordName.compare(path_info.raceName(), Qt::CaseInsensitive) == 0;
			});

			if (charRaceRecord != nullptr) {
				out.gender = GenderUtil::fromString(path_info.genderName());
				out.raceId = charRaceRecord->getId();
				out.isHd = model->model->getModelPathInfo().isHdCharacter();
				return true;
			}
		}

		return false;
	}

	void CharacterEyeGlowCustomization::enumBasedHandler(core::Model* model) {
		model->setGeosetVisibility(CharacterGeosets::CG_EYEGLOW, (uint32_t)model->characterOptions.eyeGlow);
	}

	void CharacterEyeGlowCustomization::geosetBasedHandler(core::Model* model) {
		if (model->characterOptions.eyeGlow == CharacterRenderOptions::EyeGlow::DEATH_KNIGHT) {
			model->setGeosetVisibility(CharacterGeosets::CG_EYEGLOW, 0);
		}
		else {
			model->clearGeosetVisibility(CharacterGeosets::CG_EYEGLOW);
		}
	}

	bool CharacterCustomizationProvider::apply(Model* model, const CharacterDetails& details, const CharacterCustomizations& choices) {
		model->characterCustomizationChoices = choices;
		return updateContext(details, choices);
	}

	void LegacyCharacterCustomizationProvider::initialise(const CharacterDetails& details) {

		for (const auto& key : LegacyCharacterCustomization::All) {
			known_options[key] = {};
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

		auto choice_incrementer = [&](const auto& choice_name) {
			known_options[choice_name].push_back(
				std::to_string(known_options[choice_name].size())
			);
		};

		// load available customisations
		for (auto charSectionRecord : matching_char_sections) {

			//only checking for variationIndex 0 to avoid duplicates being included.
			auto section_type = charSectionRecord->getType();
			if (section_type == CharacterSectionType::Skin) {
				choice_incrementer(LegacyCharacterCustomization::Name::Skin);
			}
			else if (section_type == CharacterSectionType::Face) {
				if (charSectionRecord->getVariationIndex() == 0) {
					choice_incrementer(LegacyCharacterCustomization::Name::Face);
				}
			}
			else if (section_type == CharacterSectionType::Hair) {
				if (charSectionRecord->getVariationIndex() == 0) {
					choice_incrementer(LegacyCharacterCustomization::Name::HairColor);
				}
			}
			else if (section_type == CharacterSectionType::FacialHair) {
				if (charSectionRecord->getVariationIndex() == 0 /* && charSectionRecord->getSection() == 0 */) {	//TODO check logic
					choice_incrementer(LegacyCharacterCustomization::Name::FacialColor);
				}
			}
		}

		const auto hair_style_count = gameDB->characterHairGeosetsDB->count(filterCustomizationOptions);
		const auto facial_hair_count = gameDB->characterFacialHairStylesDB->count(filterCustomizationOptions);

		for (auto i = 0; i < hair_style_count; i++) {
			choice_incrementer(LegacyCharacterCustomization::Name::HairStyle);
		}

		for (auto i = 0; i < facial_hair_count; i++) {
			choice_incrementer(LegacyCharacterCustomization::Name::FacialStyle);
		}
	}

	void LegacyCharacterCustomizationProvider::reset() {
		known_options.clear();
		context.reset();
	}

	bool LegacyCharacterCustomizationProvider::updateContext( const CharacterDetails& details, const CharacterCustomizations& choices) {

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
				if (charSectionRecord->getVariationIndex() == choices.at(LegacyCharacterCustomization::Name::Skin)) {
					context->skin = charSectionRecord;
					found++;
				}

			}
			else if (section_type == CharacterSectionType::Face) {
				if (charSectionRecord->getVariationIndex() == choices.at(LegacyCharacterCustomization::Name::Skin)) {
					if (charSectionRecord->getSection() == choices.at(LegacyCharacterCustomization::Name::Face)) {
						context->face = charSectionRecord;
						found++;
					}
				}
			}
			else if (section_type == CharacterSectionType::Hair) {
				if (charSectionRecord->getVariationIndex() == choices.at(LegacyCharacterCustomization::Name::HairColor) &&
					charSectionRecord->getSection() == choices.at(LegacyCharacterCustomization::Name::HairStyle)) {
					context->hairColour = charSectionRecord;
					found++;
				}
			}
			else if (section_type == CharacterSectionType::FacialHair) {
				if (charSectionRecord->getVariationIndex() == choices.at(LegacyCharacterCustomization::Name::HairColor) &&
					charSectionRecord->getSection() == choices.at(LegacyCharacterCustomization::Name::FacialColor)) {
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
			if (hair_style_index == choices.at(LegacyCharacterCustomization::Name::HairStyle)) {
				context->hairStyle = hairStyleRecord;
				break;
			}
			hair_style_index++;
		}

		auto facial_style_index = 0;

		for (auto& facialHairStyleRecord : gameDB->characterFacialHairStylesDB->where(filterCustomizationOptions)) {
			if (facial_style_index == choices.at(LegacyCharacterCustomization::Name::FacialStyle)) {
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

		eyeGlowHandler(model);

		if(context->skin != nullptr)
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


		
		if (context->hairStyle != nullptr) {
			const auto hair_geoset_id = std::max(1u, context->hairStyle->getGeoset());
			for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
				if (model->model->getGeosetAdaptors()[i]->getId() == hair_geoset_id) {
					model->forceGeosetVisibilityByIndex(i, model->characterOptions.showHair);
				}
			}

			{
				if (context->face != nullptr) {
					const auto& face = context->face->getTextures();

					if (!face[0].isEmpty()) {
						builder->addLayer(face[0], CharacterRegion::FACE_LOWER, 1);
					}

					if (!face[1].isEmpty()) {
						builder->addLayer(face[1], CharacterRegion::FACE_UPPER, 1);
					}
				}
			}
		}


		if (model->characterOptions.showFacialHair) {
			const auto* facial_geoset = context->facialStyle;

			if (facial_geoset != nullptr) {
				//must be atleast 1, can be problematic if it doesnt get shown at all.
				//NOTE records arent in 100, 300, 200 order
				//TODO check logic, is the adaptor returing data in incorrect order?
				model->setGeosetVisibility(CharacterGeosets::CG_GEOSET100, facial_geoset->getGeoset100());
				model->setGeosetVisibility(CharacterGeosets::CG_GEOSET200, facial_geoset->getGeoset300());
				model->setGeosetVisibility(CharacterGeosets::CG_GEOSET300, facial_geoset->getGeoset200());

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
		}

		if(context->hairColour != nullptr)
		{
			const auto& hair = context->hairColour->getTextures();
			
			if (!hair[0].isEmpty()) {
				hairtex = scene->textureManager.add(hair[0], gameFS);
			}

			if (context->hairStyle && !context->hairStyle->isBald())
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
			model->replacableTextures[TextureType::SKIN_EXTRA] = furtex;
		}
		else {
			model->replacableTextures.erase(TextureType::SKIN_EXTRA);
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

	ModernCharacterCustomizationProvider::ModernCharacterCustomizationProvider(GameFileSystem* fs, GameDatabase* db, const WDBReader::GameVersion& version)
		: CharacterCustomizationProvider(),
		gameFS(fs), 
		gameDB(db)
	{
		fileDataDB = dynamic_cast<IFileDataGameDatabase*>(gameDB);

		auto* const cascFS = (CascFileSystem*)(gameFS);

		auto make_db = [&](const auto& db_name, const auto& def_name) {
			auto schema = make_wbdr_schema(def_name, version);
			auto file = cascFS->openFile(db_name);
			auto casc_source = file->release();
			auto memory_source = std::make_unique<WDBReader::Filesystem::MemoryFileSource>(*casc_source);
			auto db = WDBReader::Database::makeDB2File(schema, std::move(memory_source));
			return std::make_pair(std::move(schema), std::move(db));
		};

		_schema_chr_custom = make_wbdr_schema("ChrCustomization.dbd", version);
		_schema_chr_option = make_wbdr_schema("ChrCustomizationOption.dbd", version);
		_schema_chr_choice = make_wbdr_schema("ChrCustomizationChoice.dbd", version);

		elementsDB = make_db(
			"dbfilesclient/chrcustomizationelement.db2",
			"ChrCustomizationElement.dbd"
		); 


		geosetsDB = make_db(
			"dbfilesclient/chrcustomizationgeoset.db2",
			"ChrCustomizationGeoset.dbd"
		); 

		skinnedModelsDB = make_db(
			"dbfilesclient/chrcustomizationskinnedmodel.db2",
			"ChrCustomizationSkinnedModel.dbd"
		); 

		materialsDB = make_db(
			"dbfilesclient/chrcustomizationmaterial.db2",
			"ChrCustomizationMaterial.dbd"
		); 

		textureLayersDB = make_db(
			"dbfilesclient/chrmodeltexturelayer.db2",
			"ChrModelTextureLayer.dbd"
		); 

		modelsDB = make_db(
			"dbfilesclient/chrmodel.db2",
			"ChrModel.dbd"
		); 
		
		raceModelsDB = make_db(
			"dbfilesclient/chrracexchrmodel.db2",
			"ChrRaceXChrModel.dbd"
		); 

		eyeGlowHandler = CharacterEyeGlowCustomization::enumBasedHandler;
	}

	void ModernCharacterCustomizationProvider::initialise(const CharacterDetails& details) {
		auto* const cascFS = (CascFileSystem*)(gameFS);

		auto model_id = getModelIdForCharacter(details);
		assert(model_id > 0);

		auto custom_file = cascFS->openFile("dbfilesclient/chrcustomization.db2");
		auto customs = WDBReader::Database::makeDB2File(
			_schema_chr_custom,
			custom_file->release()
		);

		auto opts_file = cascFS->openFile("dbfilesclient/chrcustomizationoption.db2");
		auto custom_opts = WDBReader::Database::makeDB2File(
			_schema_chr_option,
			opts_file->release()
		);

		auto choice_file = cascFS->openFile("dbfilesclient/chrcustomizationchoice.db2");
		auto custom_choices = WDBReader::Database::makeDB2File(
			_schema_chr_choice,
			choice_file->release()
		);

		// for whatever reason, returned string can contain invalid characters, likely an error in the record reading.
		//TODO investigate
		auto safe_format_choice_str = [](std::string& str, uint32_t index) {
			str.erase(remove_if(str.begin(), str.end(), [](char c) {return !(c >= 32 && c <= 126); }), str.end());

			if (str.size() <= 2) {
				str = std::to_string(index);
			}
		};
	
		for (auto& custom_row : *customs) {
			if (custom_row.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
				continue;
			}

			auto [cust_id, cust_sex, cust_race, cust_str] = _schema_chr_custom(custom_row)
				.get<uint32_t, uint32_t, uint32_t, WDBReader::Database::string_data_ref_t>(
					"ID", "Sex", "RaceID", "Name_lang"
				);

			if ((cust_sex == (uint32_t)details.gender || cust_sex == (uint32_t)Gender::ANY) &&
				(cust_race == details.raceId || cust_race == 0)) {

				const auto customization_str = std::string(cust_str);

				for (auto& option_row : *custom_opts) {
					if (option_row.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
						continue;
					}

					auto [opt_id, opt_cust_id, opt_model_id, opt_str_val] = _schema_chr_option(option_row)
						.get<uint32_t, uint32_t, uint32_t, WDBReader::Database::string_data_ref_t>(
						"ID", "ChrCustomizationID", "ChrModelID", "Name_lang"
					);

					if (opt_cust_id == cust_id && opt_model_id == model_id) {
						auto opt_str = std::string(opt_str_val);

						std::vector<uint32_t> choice_ids;
						std::vector<std::string> choice_strings;

						for (auto& choice_row : *custom_choices) {
							if (choice_row.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
								continue;
							}

							auto [choice_id, choice_opt_id, choice_str_val] = _schema_chr_choice(choice_row)
								.get<uint32_t, uint32_t, WDBReader::Database::string_data_ref_t>(
									"ID", "ChrCustomizationOptionID", "Name_lang"
								);

							if (choice_opt_id == opt_id) {

								auto choice_str = std::string(choice_str_val);

								choice_ids.push_back(choice_id);
								choice_strings.push_back(std::move(choice_str));
							}
						}

						if (choice_ids.size() > 0) {
							assert(choice_ids.size() == choice_strings.size());
	
							auto i = 0;
							for (auto& choice_string : choice_strings) {
								safe_format_choice_str(choice_string, i++);
							}

							known_options[opt_str] = std::move(choice_strings);
							cacheOptions[opt_str] = opt_id;
							cacheChoices[opt_id] = std::move(choice_ids);
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

	bool ModernCharacterCustomizationProvider::updateContext(const CharacterDetails& details, const CharacterCustomizations& choices) {
		
		auto* const cascFS = (CascFileSystem*)(gameFS);

		if (!context) {
			context = std::make_unique<Context>();
		}


		//TODO shouldnt need to fully reset contet each timne.
		context->geosets.clear();
		context->materials.clear();
		context->models.clear();


		const auto textureLayoutId = getTextureLayoutId(details);
		std::vector<uint32_t> selected_choices_ids;
		selected_choices_ids.reserve(choices.size());

		for (const auto& choice : choices) {
			const auto option_id = cacheOptions[choice.first];
			const auto choice_id = cacheChoices[option_id][choice.second];
			selected_choices_ids.push_back(choice_id);
		}


		for (const auto& choice : choices) {
			const auto option_id = cacheOptions[choice.first];
			const auto choice_id = cacheChoices[option_id][choice.second];

			bool choice_found_elements = false;

			for (auto& element : *elementsDB.second) {
				if (element.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
					continue;
				}

				const auto [el_choice_id, el_rel_choice, el_geoset_id, el_sk_model_id, el_mat_id] = elementsDB.first(element)
					.get<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>(
						"ChrCustomizationChoiceID", "RelatedChrCustomizationChoiceID", "ChrCustomizationGeosetID",
						"ChrCustomizationSkinnedModelID", "ChrCustomizationMaterialID"
					);

				if (el_choice_id == choice_id) {

					if (el_rel_choice != 0) {
						if (std::ranges::count(selected_choices_ids, el_rel_choice) == 0) {
							continue;
						}
					}

					choice_found_elements = true;
						
					if (el_geoset_id > 0) {
						auto tmp = findRecordById(geosetsDB, el_geoset_id);
						if (tmp.has_value()) {
							auto [geoset_id, geoset_type] = geosetsDB.first(*tmp).get<uint32_t, uint32_t>("GeosetID", "GeosetType");
							context->geosets.emplace_back(
								geoset_type,
								geoset_id
							);
						}
					}

				if (el_sk_model_id > 0) {
						auto tmp = findRecordById(skinnedModelsDB, el_sk_model_id);
						if (tmp.has_value()) {
							auto [file_id, geoset_id, geoset_type] = skinnedModelsDB.first(*tmp).get<uint32_t, uint32_t, uint32_t>(
								"CollectionsFileDataID", "GeosetID", "GeosetType"
							);
							
							const auto& model_uri = file_id;
							if (model_uri > 0) {
								context->models.emplace_back(
									file_id,	//TODO not sure if needs needs to be the record id?
									model_uri,
									geoset_type,
									geoset_id
								);
							}
						}
					}

					if (el_mat_id > 0) {
						auto tmp = findRecordById(materialsDB, el_mat_id);
						if (tmp.has_value()) {
							auto [mat_id, mat_res_id, mat_tex_target] = materialsDB.first(*tmp).get<uint32_t, uint32_t, uint32_t>(
								"ID", "MaterialResourcesID", "ChrModelTextureTargetID"
							);
							Context::Material mat;
							mat.custMaterialId = mat_id;
							mat.uri = fileDataDB->findByMaterialResId(mat_res_id, -1, std::nullopt);

							for (auto& layer : *textureLayersDB.second) {
								if (layer.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
									continue;
								}

								auto [layer_tex_target, layer_tex_layout] = textureLayersDB.first(layer).get<uint32_t, uint32_t>(
									"ChrModelTextureTargetID", "CharComponentTextureLayoutsID"
								);

								//TODO does [1] need to be checked too? (chrModelTextureTargetId is uint32_t[2])
								if (layer_tex_target == mat_tex_target &&
									layer_tex_layout == textureLayoutId) {

									auto [tex_type, layer_val, blend_mode, section_type] = textureLayersDB.first(layer).get<uint32_t, uint32_t, uint32_t, uint32_t>(
										"TextureType", "Layer", "BlendMode", "TextureSectionTypeBitMask"
									);

									mat.textureType = tex_type;
									mat.layer = layer_val;
									mat.blendMode = blend_mode;
									mat.region = bitMaskToSectionType(section_type);

									context->materials.push_back(mat);
								}
							}

						}
					}
					
				}
			}
		
			if (!choice_found_elements) {
				Log::message("Unable to find character elements for option " + QString::number(option_id) + "'" + QString::fromStdString(choice.first) + "' / choice " + QString::number(choice_id));
			}
		}

		std::sort(context->materials.begin(), context->materials.end());

		return true;
	}

	bool ModernCharacterCustomizationProvider::update(Model* model, CharacterTextureBuilder* builder, Scene* scene) {
		assert(context);

		if (!context) {
			return false;
		}

		//TODO handle model->characterOptions.showFacialHair

		for (const auto& geo : context->geosets) {
			model->setGeosetVisibility((core::CharacterGeosets)geo.geosetType, geo.geosetId, false);
		}

		// force the character face to be shown.
		model->setGeosetVisibility(core::CharacterGeosets::CG_FACE, 1);

		eyeGlowHandler(model);

		for (const auto& mat : context->materials) {
			if (mat.region == -1) {	//TODO dracthyr base == 11? 

				const bool can_be_replacable = std::ranges::count(std::ranges::views::values(model->specialTextures), (TextureType)mat.textureType) > 0;

				if (mat.textureType <= 1 || !can_be_replacable) {
					builder->pushBaseLayer(mat.uri);
				}
				else {
					auto tex = scene->textureManager.add(mat.uri, gameFS);
					model->replacableTextures[(TextureType)mat.textureType] = tex;
					Log::message("Replaceable texture: " + mat.uri.toString());
				}
			}
			else {
				builder->addLayer(mat.uri, (CharacterRegion)mat.region, mat.layer, (CharacterTextureBuilder::BlendMode)mat.blendMode);
			}
		}

		if (model != nullptr) {

			// note that multiple context->models can share the same id (e.g when model gets used twice, with different geosets)

			std::unordered_set<MergedModel::id_t> merge_checked;

			for (const auto& merged : model->getMerged()) {
				if (merged->getType() == MergedModel::Type::CHAR_MODEL_ADDITION) {
					merge_checked.insert(merged->getId());
				}
			}

			// add or update
			for (const auto& model_in : context->models) {

				MergedModel* existing = nullptr;
				const auto merged_id = model_in.custSkinnedModelId;

				for (auto* merged : model->getMerged()) {
					if (merged->getType() == MergedModel::Type::CHAR_MODEL_ADDITION && merged->getId() == merged_id) {
						existing = merged;
						break;
					}
				}

				if (existing != nullptr) {
					existing->setGeosetVisibility((CharacterGeosets)model_in.geosetType, model_in.geosetId, false);
				}
				else {
					M2Model::Factory factory = &M2Model::make; //TODO should use factory from clientinfo.

					auto custom = std::make_unique<MergedModel>(
						model,
						MergedModel::Type::CHAR_MODEL_ADDITION,
						merged_id
					);

					custom->initialise(model_in.uri, factory, gameFS, gameDB, scene->textureManager);
					custom->merge(MergedModel::RESOLUTION_FINE);

					custom->setGeosetVisibility((CharacterGeosets)model_in.geosetType, model_in.geosetId, false);

					Log::message("Loaded merged model / char addition - " + QString::number(custom->getId()));

					{
						auto* tmp = custom.get();
						model->addRelation(std::move(custom));
						scene->addComponent(tmp);
					}
				}

				merge_checked.erase(merged_id);
			}

			// cleanup any removed
			for (const auto& merged_id : merge_checked) {
				model->removeRelation(MergedModel::Type::CHAR_MODEL_ADDITION, merged_id);
				Log::message("Removed merged model / char addtion - " + QString::number(merged_id));
			}
		}
	
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
		assert(model_id > 0);

		for (auto& rec : *modelsDB.second) {
			if (rec.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
				continue;
			}

			auto [id, layout_id] = modelsDB.first(rec).get<uint32_t, uint32_t>("ID", "CharComponentTextureLayoutID");
			if (id == model_id) {
				return layout_id;
			}
		}

		return 0;
	}

	uint32_t ModernCharacterCustomizationProvider::getModelIdForCharacter(const CharacterDetails& details) {
		for (auto& rec : *raceModelsDB.second) {
			if (rec.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
				continue;
			}

			auto [race_id, sex, model_id] = raceModelsDB.first(rec).get<uint32_t, uint32_t, uint32_t>("ChrRacesID", "Sex", "ChrModelID");
			if (race_id == details.raceId && sex == (uint32_t)details.gender) {
				return model_id;
			}
		}

		return 0;
	}
}