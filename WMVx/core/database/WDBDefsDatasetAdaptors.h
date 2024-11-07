#pragma once
#include "GameDatasetAdaptors.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"
#include <WDBReader/Database/Schema.hpp>

namespace core {


	class ModernWDBDefsAnimationDataRecordAdaptor : public AnimationDataRecordAdaptor {
	public:
		ModernWDBDefsAnimationDataRecordAdaptor(std::shared_ptr<WDBR::Database::RuntimeSchema> schema, WDBR::Database::RuntimeRecord&& record, QString&& known_name) :
			knownName(std::move(known_name))
		{
			std::tie(_id) = (*schema)(record).get<uint32_t>("ID");
		}


		constexpr uint32_t getId() const override {
			return _id;
		}

		QString getName() const override {
			return !knownName.isEmpty() ? knownName : QString::number(_id);
		}

	protected:
		uint32_t _id;
		QString knownName;
	};


	class ModernWDBDefsCharRacesRecordAdaptor : public CharacterRaceRecordAdaptor {
	public:
		ModernWDBDefsCharRacesRecordAdaptor(std::shared_ptr<WDBR::Database::RuntimeSchema> schema, WDBR::Database::RuntimeRecord&& record) : 
			_schema(schema), _record(std::move(record)) {

		}

		uint32_t getId() const override {
			auto [id] = (*_schema)(_record).get<uint32_t>("ID");
			return id;
		}

		QString getClientPrefix() const override {
			auto [str] = (*_schema)(_record).get<WDBReader::Database::string_data_ref_t>("ClientPrefix");
			return QString(str);
		}

		QString getClientFileString() const override {
			auto [str] = (*_schema)(_record).get<WDBReader::Database::string_data_ref_t>("ClientFileString");
			return QString(str);
		}

		std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const override {

			auto hd_name = std::find(_schema->names().begin(), _schema->names().end(), "CharComponentTexLayoutHiResID");
			if (hd_name != _schema->names().end()) {
				auto [res] = (*_schema)(_record).get<uint32_t>("CharComponentTexLayoutHiResID");
				return res;
			} 

			auto plain_name = std::find(_schema->names().begin(), _schema->names().end(), "CharComponentTextureLayoutID");
			if (plain_name != _schema->names().end()) {
				auto [res] = (*_schema)(_record).get<uint32_t>("CharComponentTextureLayoutID");
				return res;
			}

			return std::nullopt;
		}

		virtual std::optional<CharacterRelationSearchContext> getModelSearchContext(Gender gender) const override {
			assert(gender == Gender::MALE || gender == Gender::FEMALE);


			switch (gender)
			{
			case Gender::MALE:
				{
					auto [id, fsex, frace] = (*_schema)(_record).get<uint32_t, int8_t, uint32_t>("ID", "MaleModelFallbackSex", "MaleModelFallbackRaceID");
					return CharacterRelationSearchContext::make(gender, id, fsex, frace, this->getClientPrefix());
				}
				break;
			case Gender::FEMALE:
				{
					auto [id, fsex, frace] = (*_schema)(_record).get<uint32_t, int8_t, uint32_t>("ID", "FemaleModelFallbackSex", "FemaleModelFallbackRaceID");
					return CharacterRelationSearchContext::make(gender, id, fsex, frace, this->getClientPrefix());
				}
				break;
			}

			return std::nullopt;
		}

		virtual std::optional<CharacterRelationSearchContext> getTextureSearchContext(Gender gender) const override {
			assert(gender == Gender::MALE || gender == Gender::FEMALE);

			switch (gender)
			{
			case Gender::MALE:
				{
					auto [id, fsex, frace] = (*_schema)(_record).get<uint32_t, int8_t, uint32_t>("ID", "MaleTextureFallbackSex", "MaleTextureFallbackRaceID");
					return CharacterRelationSearchContext::make(gender, id, fsex, frace, this->getClientPrefix());
				}
				break;
			case Gender::FEMALE:
				{
					auto [id, fsex, frace] = (*_schema)(_record).get<uint32_t, int8_t, uint32_t>("ID", "FemaleTextureFallbackSex", "FemaleTextureFallbackRaceID");
					return CharacterRelationSearchContext::make(gender, id, fsex, frace, this->getClientPrefix());
				}
				break;
			}

			return std::nullopt;
		}

	protected:
		std::shared_ptr<WDBR::Database::RuntimeSchema> _schema;
		WDBR::Database::RuntimeRecord _record;
	};

	
	class ModernWDBDefsCharacterComponentTextureAdaptor : public CharacterComponentTextureAdaptor {
	public:
		ModernWDBDefsCharacterComponentTextureAdaptor(
			std::shared_ptr<WDBR::Database::RuntimeSchema> schema, 
			WDBR::Database::RuntimeRecord&& record,
			std::map<CharacterRegion, CharacterRegionCoords>&& regions) : _regions(std::move(regions))
		{
			std::tie(_layout_id, _width, _height) = (*schema)(record).get<uint32_t, int32_t, int32_t>("ID", "Width", "Height");
		}

		constexpr uint32_t getLayoutId() const override {
			return _layout_id;
		}

		constexpr int32_t getLayoutWidth() const override {
			return _width;
		}

		constexpr int32_t getLayoutHeight() const override {
			return _height;
		}

		std::map<CharacterRegion, CharacterRegionCoords> getRegions() const override {
			return _regions;
		}

	protected:
		uint32_t _layout_id;
		int32_t _width;
		int32_t _height;
		std::map<CharacterRegion, CharacterRegionCoords> _regions;
	};

	class ModernWDBDefsCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor {
	public:
		ModernWDBDefsCreatureModelDataRecordAdaptor(std::shared_ptr<WDBR::Database::RuntimeSchema> schema, WDBR::Database::RuntimeRecord&& record) {
			std::tie(_id, _file_data_id) = (*schema)(record).get<uint32_t, uint32_t>("ID", "FileDataID");
		}

		constexpr uint32_t getId() const override {
			return _id;
		}

		GameFileUri getModelUri() const override {
			return _file_data_id;
		}

	protected:
		uint32_t _id;
		uint32_t _file_data_id;
	};


	class ModernWDBDefsCreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor {
	public:
		ModernWDBDefsCreatureDisplayRecordAdaptor(std::shared_ptr<WDBR::Database::RuntimeSchema> schema, WDBR::Database::RuntimeRecord&& record) :
			_schema(schema), _record(std::move(record)) {
			std::tie(_id, _model_id) = (*_schema)(_record).get<uint32_t, uint32_t>("ID", "ModelID");
		}

		constexpr uint32_t getId() const override {
			return _id;
		}

		constexpr uint32_t getModelId() const override {
			return _model_id;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			std::array<GameFileUri, 3> result;

			auto fids = (*_schema)(_record)["TextureVariationFileDataID"];
			for (auto i = 0; i < 3 && i < fids.size(); i++) {
				std::visit([&result, &i](const auto& val) {
					if constexpr (std::is_convertible_v<std::decay_t<decltype(val)>, GameFileUri::id_t>) {
						result[i] = (GameFileUri::id_t)val;
					}
					else {
						result[i] = 0u;
					}
				}, fids[i]);
			}

			return result;
		}

		const CreatureDisplayExtraRecordAdaptor* getExtra() const override {
			return nullptr; // //TODO_extra_adaptor.get();
		}

	protected:
		uint32_t _id;
		uint32_t _model_id;

		std::shared_ptr<WDBR::Database::RuntimeSchema> _schema;
		WDBR::Database::RuntimeRecord _record;

		//std::unique_ptr<CreatureDisplayExtraRecordAdaptor> _extra_adaptor;
	};

	
	class ModernWDBDefsItemRecordAdaptor : public ItemRecordAdaptor {
	public:
		ModernWDBDefsItemRecordAdaptor(
			std::shared_ptr<WDBReader::Database::RuntimeSchema> schema, WDBReader::Database::RuntimeRecord&& record,
			std::pair<std::shared_ptr<WDBReader::Database::RuntimeSchema>, std::shared_ptr<WDBReader::Database::RuntimeRecord>>&& sparse,
			std::vector<uint32_t>&& display_ids)
			 : ItemRecordAdaptor(), _item_display_info_ids(std::move(display_ids))
		{

			std::tie(_id, _inv_id, _sheath) = (*schema)(record).get<uint32_t, ItemInventorySlotId, SheathTypes>("ID", "InventoryType", "SheatheType");
			std::tie(_quality, _name) = (*sparse.first)(*sparse.second).get<ItemQualityId, WDBReader::Database::string_data_ref_t>("OverallQualityID", "Display_lang");
		}

		constexpr uint32_t getId() const override {
			return _id;
		}

		constexpr std::vector<uint32_t> getItemDisplayInfoId() const override {
			return _item_display_info_ids;
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return _inv_id;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return _sheath;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return _quality;
		}

		QString getName() const override {
			return _name;
		}

	protected:

		uint32_t _id;
		ItemInventorySlotId _inv_id;
		SheathTypes _sheath;
		ItemQualityId _quality;
		QString _name;
		std::vector<uint32_t> _item_display_info_ids;
	};


	class ModernWDBDefsItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor {
	public:
		ModernWDBDefsItemDisplayInfoRecordAdaptor(
			std::shared_ptr<WDBReader::Database::RuntimeSchema> schema, 
			WDBReader::Database::RuntimeRecord&& record,
			std::shared_ptr<WDBReader::Database::RuntimeSchema> material_schema,
			std::vector<WDBReader::Database::RuntimeRecord>&& materials,
			const IFileDataGameDatabase* fdDB) :
			_schema(schema), _record(std::move(record)), fileDataDB(fdDB)
		{
		
			for (auto& rec : materials) {
				if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
					auto [comp_section, mat_res_id] = (*material_schema)(rec).get<uint8_t, uint32_t>("ComponentSection", "MaterialResourcesID");
					_materials.emplace(comp_section, mat_res_id);
				}
			}
		}

		uint32_t getId() const override {
			auto [id] = (*_schema)(_record).get<uint32_t>("ID");
			return id;
		}

		std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			auto [res_ids] = (*_schema)(_record).get<std::array<uint32_t, 2>>("ModelResourcesID");

			if (res_ids[0] == 0 && res_ids[1] == 0) {
				return { 0u, 0u };
			}

			const bool same_resource = res_ids[0] == res_ids[1];
			auto temp = fileDataDB->findByModelResIdFixed(res_ids, search);

			if (same_resource && temp[1] == 0) {
				temp[1] = temp[0];
			}

			return GameFileUri::arrayConvert(std::move(temp));
		}

		uint32_t getGeosetGlovesFlags() const override {
			auto [geoset_group] = (*_schema)(_record).get<std::array<uint32_t, 3>>("GeosetGroup");
			return geoset_group[0];
		}

		uint32_t getGeosetBracerFlags() const override {
			auto [geoset_group] = (*_schema)(_record).get<std::array<uint32_t, 3>>("GeosetGroup");
			return geoset_group[1];
		}

		uint32_t getGeosetRobeFlags() const override {
			auto [geoset_group] = (*_schema)(_record).get<std::array<uint32_t, 3>>("GeosetGroup");
			return geoset_group[2];
		}

		std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			auto [res_ids] = (*_schema)(_record).get<std::array<uint32_t, 2>>("ModelMaterialResourcesID");

			if (res_ids[0] == 0 && res_ids[1] == 0) {
				return { 0u, 0u };
			}

			const bool same_resource = res_ids[0] == res_ids[1];

			auto temp = fileDataDB->findByMaterialResIdFixed(res_ids, search);

			if (same_resource && temp[1] == 0) {
				temp[1] = temp[0];
			}

			return GameFileUri::arrayConvert(std::move(temp));
		}

		GameFileUri getTextureUpperArm() const override {
			return findMaterialBySection(0);
		}

		GameFileUri getTextureLowerArm() const override {
			return findMaterialBySection(1);
		}

		GameFileUri getTextureHands() const override {
			return findMaterialBySection(2);
		}

		GameFileUri getTextureUpperChest() const override {
			return findMaterialBySection(3);
		}

		GameFileUri getTextureLowerChest() const override {
			return findMaterialBySection(4);
		}

		GameFileUri getTextureUpperLeg() const override {
			return findMaterialBySection(5);
		}

		GameFileUri getTextureLowerLeg() const override {
			return findMaterialBySection(6);
		}

		GameFileUri getTextureFoot() const override {
			return findMaterialBySection(7);
		}

		uint32_t getItemVisualId() const override {
			auto [id] = (*_schema)(_record).get<uint32_t>("ItemVisual");
			return id;
		}

	protected:

		inline GameFileUri::id_t findMaterialBySection(uint8_t section) const {
			auto found = _materials.find(section);
			if (found != _materials.end()) {
				return findTextureFileId(found->second);
			}

			return 0u;
		}

		inline GameFileUri::id_t findTextureFileId(uint32_t id) const {
			return fileDataDB->findByMaterialResId(id, -1, std::nullopt); //TODO confirm if search context needs to be used here too.
		}


		std::shared_ptr<WDBR::Database::RuntimeSchema> _schema;
		WDBR::Database::RuntimeRecord _record;
		std::map<uint8_t, uint32_t> _materials; // section -> mat_res_id
		const IFileDataGameDatabase* fileDataDB;
	};

	class ModernWDBDefsNPCRecordAdaptor : public NPCRecordAdaptor {
	public:

		ModernWDBDefsNPCRecordAdaptor(std::shared_ptr<WDBR::Database::RuntimeSchema> schema, WDBR::Database::RuntimeRecord&& record) :
			_schema(schema), _record(std::move(record)) {
		}

		uint32_t getId() const override {
			auto [id] = (*_schema)(_record).get<uint32_t>("ID");
			return id;
		}

		uint32_t getModelId() const override {
			//this->_record.data.displayId[0];	//TODO handle multiple display id's
			auto [display_id] = (*_schema)(_record).get<uint32_t>("DisplayID");
			return display_id;
		}

		uint32_t getType() const override {
			auto [type] = (*_schema)(_record).get<uint32_t>("CreatureType");
			return type;
		}

		QString getName() const override {
			auto [str] = (*_schema)(_record).get<WDBReader::Database::string_data_ref_t>("Name_lang");
			return QString(str);
		}

	protected:
		std::shared_ptr<WDBR::Database::RuntimeSchema> _schema;
		WDBR::Database::RuntimeRecord _record;
	};

}