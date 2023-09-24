#pragma once
#include "GameDatasetAdaptors.h"
#include "BFARecordDefinitions.h"
#include "DB2BackedDataset.h"

namespace core {

	class BFAAnimationDataRecordAdaptor : public AnimationDataRecordAdaptor, public DB2BackedAdaptor<BFADB2AnimationDataRecord> {
	public:

		BFAAnimationDataRecordAdaptor(const BFADB2AnimationDataRecord* handle,
			const DB2File<BFADB2AnimationDataRecord>* db2,
			const DB2File<BFADB2AnimationDataRecord>::SectionView* section_view,
			QString known_name) :
			DB2BackedAdaptor<BFADB2AnimationDataRecord>(handle, db2, section_view) 
		{
			knownName = known_name;
		}

		BFAAnimationDataRecordAdaptor(BFAAnimationDataRecordAdaptor&&) = default;
		virtual ~BFAAnimationDataRecordAdaptor(){}

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		QString getName() const override {
			return !knownName.isEmpty() ? knownName : QString::number(this->handle->data.id);	
		}

	protected:

		QString knownName;
	};

	class BFACharRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public DB2BackedAdaptor<BFADB2ChrRacesRecord> {
	public:
		using DB2BackedAdaptor<BFADB2ChrRacesRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		QString getClientPrefix() const override {
			return this->db2->getString(this->handle->data.clientPrefix, this->section_view, this->handle->recordIndex, 0);
		}

		QString getClientFileString() const override {
			return this->db2->getString(this->handle->data.clientFileString, this->section_view, this->handle->recordIndex, 1);
		}

		std::optional<uint32_t> getComponentTextureLayoutId() const override {
			return this->handle->data.charComponentTextureLayoutID;
		}
	};

	class BFACreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public DB2BackedAdaptor<BFADB2CreatureModelDataRecord> {
	public:
		using DB2BackedAdaptor<BFADB2CreatureModelDataRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		GameFileUri getModelUri() const override {
			return this->handle->data.fileDataID;
		}
	};

	class BFACreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor, public DB2BackedAdaptor<BFADB2CreatureDisplayInfoRecord> {
	public:
		using DB2BackedAdaptor<BFADB2CreatureDisplayInfoRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->handle->data.modelId;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				this->handle->data.textureVariationFileDataID[0],
				this->handle->data.textureVariationFileDataID[1],
				this->handle->data.textureVariationFileDataID[2]
			};
		}
	};

	class BFACharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor, public DB2BackedAdaptor<BFADB2CharSectionsRecord> {
	public:
		BFACharSectionsRecordAdaptor(const BFADB2CharSectionsRecord* handle, 
			const DB2File<BFADB2CharSectionsRecord>* db2, 
			const DB2File<BFADB2CharSectionsRecord>::SectionView* section_view, 
			const DB2File<BFADB2TextureFileDataRecord>* textureFileDatadb2) :
			DB2BackedAdaptor<BFADB2CharSectionsRecord>(handle, db2, section_view),
			textureFileDatadb2(textureFileDatadb2) {}
		BFACharSectionsRecordAdaptor(BFACharSectionsRecordAdaptor&&) = default;
		virtual ~BFACharSectionsRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return handle->data.id;
		}

		constexpr uint32_t getRaceId() const override {
			return handle->data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(handle->data.sexId);
		}

		constexpr CharacterSectionType getType() const override {
			return static_cast<CharacterSectionType>(handle->data.type);
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return findTextureFileIds(
					this->handle->data.textures[0],
					this->handle->data.textures[1],
					this->handle->data.textures[2]
				);
		}

		constexpr uint32_t getSection() const override {
			return handle->data.section;
		}

		constexpr uint32_t getVariationIndex() const override {
			return handle->data.variationIndex;
		}

	protected:
		const DB2File<BFADB2TextureFileDataRecord>* textureFileDatadb2;

		inline std::array<GameFileUri, 3> findTextureFileIds(uint32_t id1, uint32_t id2, uint32_t id3) const {
			
			std::array<GameFileUri, 3> result = {
				0ul,
				0ul,
				0ul
			};

			const auto& sections = textureFileDatadb2->getSections();
			for (const auto& section : sections) {
				for (const auto& record : section.records) {
					if (record.data.materialResourcesId == id1) {
						result[0] = record.data.fileDataId;
					}
					else if (record.data.materialResourcesId == id2) {
						result[1] = record.data.fileDataId;
					}
					else if (record.data.materialResourcesId == id3) {
						result[2] = record.data.fileDataId;
					}

					if (!result[0].isEmpty() && !result[1].isEmpty() && !result[2].isEmpty()) {
						//exit early.
						return result;
					}
				}
			}
			
			return result;
		}

	};

	class BFACharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public DB2BackedAdaptor<BFADB2CharacterFacialHairStylesRecord> {
	public:
		using DB2BackedAdaptor<BFADB2CharacterFacialHairStylesRecord>::DB2BackedAdaptor;
		constexpr uint32_t getRaceId() const override {
			return this->handle->data.raceId;
		}
		 
		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->handle->data.sexId);
		}

		constexpr uint32_t getGeoset100() const override {
			return this->handle->data.geosets[0];
		}

		constexpr uint32_t getGeoset200() const override {
			return this->handle->data.geosets[1];
		}

		constexpr uint32_t getGeoset300() const override {
			return this->handle->data.geosets[2];
		}

	};

	class BFACharHairGeosetsRecordAdaptor: public CharacterHairGeosetRecordAdaptor, public DB2BackedAdaptor<BFADB2CharHairGeosetsRecord> {
	public:
		using DB2BackedAdaptor<BFADB2CharHairGeosetsRecord>::DB2BackedAdaptor;

		constexpr uint32_t getRaceId() const override  {
			return this->handle->data.raceId;
		}

		constexpr Gender getSexId() const override  {
			return static_cast<Gender>(this->handle->data.sexId);
		}

		constexpr uint32_t getGeoset() const override {
			return this->handle->data.geosetId;
		}

		constexpr bool isBald() const override {
			return this->handle->data.showScalp;
		}
	};

	class BFAItemRecordAdaptor : public ItemRecordAdaptor, public DB2BackedAdaptor<BFADB2ItemRecord> {
	public:
		BFAItemRecordAdaptor(const BFADB2ItemRecord* handle, 
			const DB2File<BFADB2ItemRecord>* db2, 
			const DB2File<BFADB2ItemRecord>::SectionView* section_view, 
			const BFADB2ItemSparseRecord* sparse, 
			const BFADB2ItemAppearanceRecord* appearance)
			: DB2BackedAdaptor(handle, db2, section_view),
			sparseHandle(sparse), 
			appearanceHandle(appearance) {}
		BFAItemRecordAdaptor(BFAItemRecordAdaptor&&) = default;
		virtual ~BFAItemRecordAdaptor(){}

		constexpr uint32_t getId() const override {
			return handle->data.id;
		}

		constexpr uint32_t getItemDisplayInfoId() const override {
			return appearanceHandle->data.itemDisplayInfoId;
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return (ItemInventorySlotId)handle->data.inventoryType;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)handle->data.sheatheType;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return (ItemQualityId)sparseHandle->data.overallQualityId;
		}

		QString getName() const override {
			return sparseHandle->inlineStrings[4];
		}

	protected:
		const BFADB2ItemSparseRecord* sparseHandle;
		const BFADB2ItemAppearanceRecord* appearanceHandle;
	};

	class BFAItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor, public DB2BackedAdaptor<BFADB2ItemDisplayInfoRecord> {
	public:
		
		BFAItemDisplayInfoRecordAdaptor(const BFADB2ItemDisplayInfoRecord* handle,
			const DB2File<BFADB2ItemDisplayInfoRecord>* db2,
			const DB2File< BFADB2ItemDisplayInfoRecord>::SectionView* section_view,
			const std::vector<const BFADB2ItemDisplayInfoMaterialResRecord*>& materials, 
			const DB2File<BFADB2ModelFileDataRecord>* modelFileDatadb2,
			const DB2File<BFADB2TextureFileDataRecord>* textureFileDatadb2
			) 
			: DB2BackedAdaptor<BFADB2ItemDisplayInfoRecord>(handle, db2, section_view),
			materials(materials),
			modelFileDatadb2(modelFileDatadb2),
			textureFileDatadb2(textureFileDatadb2) {}

		BFAItemDisplayInfoRecordAdaptor(BFAItemDisplayInfoRecordAdaptor&&) = default;
		virtual ~BFAItemDisplayInfoRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return handle->data.id;
		}

		std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot) const override {
			return { 
				findModelFileId(this->handle->data.modelResourcesId[0]),
				findModelFileId(this->handle->data.modelResourcesId[1])
			}; 
		}

		constexpr uint32_t getGeosetGlovesFlags() const override {
			return this->handle->data.geosetGroup[0]; 
		}

		constexpr uint32_t getGeosetBracerFlags() const override {
			return this->handle->data.geosetGroup[1]; 
		}

		constexpr uint32_t getGeosetRobeFlags() const override {
			return this->handle->data.geosetGroup[2]; 
		}

		std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot) const override {
			return { 
				findTextureFileId(this->handle->data.modelMaterialResourcesId[0]),
				findTextureFileId(this->handle->data.modelMaterialResourcesId[1])
			}; 
		}

		GameFileUri getTextureUpperArm() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 0) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerArm() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 1) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureHands() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 2) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperChest() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 3) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerChest() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 4) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperLeg() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 5) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerLeg() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 6) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureFoot() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 7) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		constexpr uint32_t getItemVisualId() const override {
			return handle->data.itemVisualId;
		}

	protected:

		std::vector<const BFADB2ItemDisplayInfoMaterialResRecord*> materials;

		const DB2File<BFADB2ModelFileDataRecord>* modelFileDatadb2;
		const DB2File<BFADB2TextureFileDataRecord>* textureFileDatadb2;

		GameFileUri::id_t findModelFileId(uint32_t id) const {
			const auto& sections = modelFileDatadb2->getSections();
			for (const auto& section : sections) {
				for (const auto& record : section.records) {
					if (record.data.modelResourcesId == id) {
						return record.data.fileDataId;
					}
				}
			}

			return 0u;
		}

		GameFileUri::id_t findTextureFileId(uint32_t id) const {

			const auto& sections = textureFileDatadb2->getSections();
			for (const auto& section : sections) {
				for (const auto& record : section.records) {
					if (record.data.materialResourcesId == id) {
						return record.data.fileDataId;
					}
				}
			}

			return 0u;
		}
	};

	class BFACharacterComponentTextureAdaptor : public CharacterComponentTextureAdaptor {
	public:
		BFACharacterComponentTextureAdaptor(const BFADB2CharComponentTextureLayoutsRecord* layout,
			const std::vector<const BFADB2CharComponentTextureSectionsRecord*>& sections) 
			: layoutHandle(layout), sectionHandles(sections) {}

		BFACharacterComponentTextureAdaptor(BFACharacterComponentTextureAdaptor&&) = default;
		virtual ~BFACharacterComponentTextureAdaptor() {}

		constexpr uint32_t getLayoutId() const override {
			return this->layoutHandle->data.id;
		}

		constexpr int32_t getLayoutWidth() const override {
			return this->layoutHandle->data.width;
		}

		constexpr int32_t getLayoutHeight() const override {
			return this->layoutHandle->data.height;
		}

		std::map<CharacterRegion, CharacterRegionCoords> getRegions() const override {
			auto regions = std::map<CharacterRegion, CharacterRegionCoords>();

			for (const auto& section : sectionHandles) {
				CharacterRegionCoords coords = {
					section->data.x, section->data.y, section->data.width, section->data.height
				};

				regions.emplace((CharacterRegion)section->data.sectionType, coords);
			}

			return regions;
		}

	protected:
		const BFADB2CharComponentTextureLayoutsRecord* layoutHandle;
		std::vector<const BFADB2CharComponentTextureSectionsRecord*> sectionHandles;
	};


	/* --- */


	class BFANPCRecordAdaptor : public NPCRecordAdaptor, public DB2BackedAdaptor<BFADB2CreatureRecord> {
	public:
		using DB2BackedAdaptor<BFADB2CreatureRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->handle->data.displayId;
		}

		constexpr uint32_t getType() const override {
			return this->handle->data.creatureType;
		}

		QString getName() const override {
			return this->db2->getString(this->handle->data.nameLang, this->section_view, this->handle->recordIndex, 0);
		}
	};

}