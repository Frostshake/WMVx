#pragma once

#include "GameDatasetAdaptors.h"
#include "FileDataGameDatabase.h"
#include "GenericDB2Dataset.h"
#include <WDBReader/Database.hpp>

namespace core {

	template<WDBReader::Database::TRecord T>
	class ModernAnimationDataRecordAdaptor : public AnimationDataRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		ModernAnimationDataRecordAdaptor(T&& record, QString&& known_name) :
			GenericDB2RecordAdaptor<T>(std::move(record)),
			knownName(std::move(known_name))
		{}
		ModernAnimationDataRecordAdaptor(ModernAnimationDataRecordAdaptor&&) = default;
		virtual ~ModernAnimationDataRecordAdaptor() = default;


		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		QString getName() const override {
			return !knownName.isEmpty() ? knownName : QString::number(this->_record.data.id);
		}

	protected:
		QString knownName;
	};

	template<WDBReader::Database::TRecord T>
	class ModernCharRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getId() const override {
			return  this->_record.data.id;
		}

		QString getClientPrefix() const override {
			return QString(this->_record.data.clientPrefix.get());
		}

		QString getClientFileString() const override {
			return QString(this->_record.data.clientFileString.get());
		}

		std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const override {

			constexpr auto has_fields = requires(T t) {
				t.data.charComponentTextureLayoutId;
				t.data.charComponentTexLayoutHiResId;
			};

			if constexpr (has_fields) {
				if (hd) {
					return this->_record.data.charComponentTexLayoutHiResId;
				}

				return this->_record.data.charComponentTextureLayoutId;
			}
			
			return std::nullopt;
		}

		virtual std::optional<CharacterRelationSearchContext> getModelSearchContext(Gender gender) const override {
			assert(gender == Gender::MALE || gender == Gender::FEMALE);


			switch (gender)
			{
			case Gender::MALE:
				return CharacterRelationSearchContext::make(
					gender,
					this->_record.data.id,
					this->_record.data.maleModelFallbackSex,
					this->_record.data.maleModelFallbackRaceId,
					this->getClientPrefix()
				);
				break;
			case Gender::FEMALE:
				return CharacterRelationSearchContext::make(
					gender,
					this->_record.data.id,
					this->_record.data.femaleModelFallbackSex,
					this->_record.data.femaleModelFallbackRaceId,
					this->getClientPrefix()
				);
				break;
			}

			return std::nullopt;
		}

		virtual std::optional<CharacterRelationSearchContext> getTextureSearchContext(Gender gender) const override {
			assert(gender == Gender::MALE || gender == Gender::FEMALE);


			switch (gender)
			{
			case Gender::MALE:
				return CharacterRelationSearchContext::make(
					gender,
					this->_record.data.id,
					this->_record.data.maleTextureFallbackSex,
					this->_record.data.maleTextureFallbackRaceId,
					this->getClientPrefix()
				);
				break;
			case Gender::FEMALE:
				return CharacterRelationSearchContext::make(
					gender,
					this->_record.data.id,
					this->_record.data.femaleTextureFallbackSex,
					this->_record.data.femaleTextureFallbackRaceId,
					this->getClientPrefix()
				);
				break;
			}

			return std::nullopt;
		}
	};

	template<WDBReader::Database::TRecord T>
	class ModernCharacterFacialHairStylesDataset : public CharacterFacialHairStyleRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}

		constexpr uint32_t getGeoset100() const override {
			return this->_record.data.geoset[0];
		}

		constexpr uint32_t getGeoset200() const override {
			return this->_record.data.geoset[2];
		}

		constexpr uint32_t getGeoset300() const override {
			return this->_record.data.geoset[1];
		}
	};

	template<WDBReader::Database::TRecord T>
	class ModernCharHairGeosetsRecordAdaptor : public CharacterHairGeosetRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}

		constexpr uint32_t getGeoset() const override {
			return this->_record.data.geosetId;
		}

		constexpr bool isBald() const override {
			return this->_record.data.showscalp;
		}
	};

	template<WDBReader::Database::TRecord LayoutsRecord, WDBReader::Database::TRecord SectionsRecord>
	class ModernCharacterComponentTextureAdaptor : public CharacterComponentTextureAdaptor {
	public:
		using Record = LayoutsRecord;
		ModernCharacterComponentTextureAdaptor(LayoutsRecord&& layout,
			std::vector<const SectionsRecord*>&& sections)
			: _layout(std::move(layout)), _sections(std::move(sections))
		{}
		ModernCharacterComponentTextureAdaptor(ModernCharacterComponentTextureAdaptor&&) = default;
		virtual ~ModernCharacterComponentTextureAdaptor() = default;

		constexpr uint32_t getLayoutId() const override {
			return this->_layout.data.id;
		}

		constexpr int32_t getLayoutWidth() const override {
			return this->_layout.data.width;
		}

		constexpr int32_t getLayoutHeight() const override {
			return this->_layout.data.height;
		}

		std::map<CharacterRegion, CharacterRegionCoords> getRegions() const override {
			auto regions = std::map<CharacterRegion, CharacterRegionCoords>();

			for (const auto& section : _sections) {
				CharacterRegionCoords coords = {
					section->data.x, section->data.y, section->data.width, section->data.height
				};

				regions.emplace((CharacterRegion)section->data.sectionType, coords);
			}

			return regions;
		}

	protected:
		LayoutsRecord _layout;
		std::vector<const SectionsRecord*> _sections;
	};

	template<WDBReader::Database::TRecord T>
	class ModernCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		GameFileUri getModelUri() const override {
			return this->_record.data.fileDataId;
		}
	};

	template<WDBReader::Database::TRecord T>
	class ModernCreatureDisplayInfoExtraRecordAdaptor : public CreatureDisplayExtraRecordAdaptor, public GenericDB2RecordAdaptor<T> {
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.displayRaceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.displaySexId);
		}

		constexpr uint32_t getSkinId() const override {
			return this->_record.data.skinId;
		}

		constexpr uint32_t getFaceId() const override {
			return this->_record.data.faceId;
		}

		constexpr uint32_t getHairStyleId() const override {
			return this->_record.data.hairStyleId;
		}

		constexpr uint32_t getHairColorId() const override {
			return this->_record.data.hairColorId;
		}

		constexpr uint32_t getFacialHairId() const override {
			return this->_record.data.facialHairId;
		}

		std::map<ItemInventorySlotId, uint32_t> getItemDisplayIds() const override {
			return {};	// not implemented - there doesnt seem to be enough client data available on npcs to make this worth while.
		}
	};

	template<WDBReader::Database::TRecord T>
	class ModernCreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		ModernCreatureDisplayRecordAdaptor(T&& record,
			std::unique_ptr<CreatureDisplayExtraRecordAdaptor> extra) :
			GenericDB2RecordAdaptor<T>(std::move(record)),
			_extra_adaptor(std::move(extra))
		{}
		ModernCreatureDisplayRecordAdaptor(ModernCreatureDisplayRecordAdaptor<T>&&) = default;
		virtual ~ModernCreatureDisplayRecordAdaptor() = default;
	
		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->_record.data.modelId;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				this->_record.data.textureVariationFileDataId[0],
				this->_record.data.textureVariationFileDataId[1],
				this->_record.data.textureVariationFileDataId[2],
			};
		}

		const CreatureDisplayExtraRecordAdaptor* getExtra() const override {
			return _extra_adaptor.get();
		}

	protected:
		std::unique_ptr<CreatureDisplayExtraRecordAdaptor> _extra_adaptor;
	};

	template<WDBReader::Database::TRecord T>
	class ModernNPCRecordAdaptor : public NPCRecordAdaptor, public GenericDB2RecordAdaptor<T> {
	public:
		using GenericDB2RecordAdaptor<T>::GenericDB2RecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->_record.data.displayId[0];	//TODO handle multiple display id's
		}

		constexpr uint32_t getType() const override {
			return this->_record.data.creatureType;
		}

		QString getName() const override {
			return QString(this->_record.data.nameLang.get());
		}
	};


	template<WDBReader::Database::TRecord ItemRecord, WDBReader::Database::TRecord ItemSparseRecord, WDBReader::Database::TRecord ItemAppearanceRecord>
	class ModernItemRecordAdaptor : public ItemRecordAdaptor {
	public:
		using Record = ItemRecord;
		ModernItemRecordAdaptor(ItemRecord&& item, const ItemSparseRecord* sparse, const ItemAppearanceRecord* appearance) : 
			_item(std::move(item)), _sparse(sparse), _appearance(appearance), ItemRecordAdaptor() {}
		ModernItemRecordAdaptor(ModernItemRecordAdaptor&&) = default;
		virtual ~ModernItemRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return this->_item.data.id;
		}

		constexpr std::vector<uint32_t> getItemDisplayInfoId() const override {
			return { _appearance->data.itemDisplayInfoId };
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return (ItemInventorySlotId)this->_item.data.inventoryType;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)this->_item.data.sheatheType;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return (ItemQualityId)_sparse->data.overallQualityId;
		}

		QString getName() const override {
			return QString(_sparse->data.displayLang.get());
		}

	protected:
		ItemRecord _item;
		const ItemSparseRecord* _sparse;
		const ItemAppearanceRecord* _appearance;
	};

	template<WDBReader::Database::TRecord DisplayRecord, WDBReader::Database::TRecord MatResRecord>
	class ModernItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor {
	public:
		using Record = DisplayRecord;
		ModernItemDisplayInfoRecordAdaptor(DisplayRecord&& display,
			std::vector<const MatResRecord*>&& materials,
			const IFileDataGameDatabase* fdDB) : 
		_display(std::move(display)), _materials(std::move(materials)), fileDataDB(fdDB)
		{}
		ModernItemDisplayInfoRecordAdaptor(ModernItemDisplayInfoRecordAdaptor&&) = default;
		virtual ~ModernItemDisplayInfoRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return this->_display.data.id;
		}

		std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			const bool same_resource = this->_display.data.modelResourcesId[0] == this->_display.data.modelResourcesId[1];

			std::array<uint32_t, 2> in{
				this->_display.data.modelResourcesId[0],
				this->_display.data.modelResourcesId[1]
			};

			auto temp = fileDataDB->findByModelResIdFixed(in, search);

			if (same_resource && temp[1] == 0) {
				temp[1] = temp[0];
			}

			return GameFileUri::arrayConvert(std::move(temp));
		}

		constexpr uint32_t getGeosetGlovesFlags() const override {
			return this->_display.data.geosetGroup[0];
		}

		constexpr uint32_t getGeosetBracerFlags() const override {
			return this->_display.data.geosetGroup[1];
		}

		constexpr uint32_t getGeosetRobeFlags() const override {
			return this->_display.data.geosetGroup[2];
		}

		std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			const bool same_resource = this->_display.data.modelMaterialResourcesId[0] == this->_display.data.modelMaterialResourcesId[1];
			std::array<uint32_t, 2> in{
				this->_display.data.modelMaterialResourcesId[0],
				this->_display.data.modelMaterialResourcesId[1]
			};

			auto temp = fileDataDB->findByMaterialResIdFixed(in, search);

			if (same_resource && temp[1] == 0) {
				temp[1] = temp[0];
			}

			return GameFileUri::arrayConvert(std::move(temp));
		}

		GameFileUri getTextureUpperArm() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 0) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerArm() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 1) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureHands() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 2) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperChest() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 3) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerChest() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 4) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperLeg() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 5) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerLeg() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 6) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureFoot() const override {
			for (auto* mat : _materials) {
				if (mat->data.componentSection == 7) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		constexpr uint32_t getItemVisualId() const override {
			return this->_display.data.itemVisual;
		}

	protected:
		inline GameFileUri::id_t findTextureFileId(uint32_t id) const {
			return fileDataDB->findByMaterialResId(id, -1, std::nullopt); //TODO confirm if search context needs to be used here too.
		}

		DisplayRecord _display;
		std::vector<const MatResRecord*> _materials;
		const IFileDataGameDatabase* fileDataDB;
	};


};