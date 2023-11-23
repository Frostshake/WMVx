#pragma once
#include "GameDatasetAdaptors.h"
#include "BFARecordDefinitions.h"
#include "DB2BackedDataset.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"

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

		std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const override {
			if (hd) {
				return this->handle->data.charComponentTexLayoutHiResID;
			}

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
			const IFileDataGameDatabase* fdDB,
			const BFADB2CharBaseSectionRecord* baseSectionRec) :
			DB2BackedAdaptor<BFADB2CharSectionsRecord>(handle, db2, section_view),
			fileDataDB(fdDB),
			baseSectionRecord(baseSectionRec){}
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
			return static_cast<CharacterSectionType>(baseSectionRecord ? baseSectionRecord->data.variationEnum : 0);
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

		constexpr bool isHD() const override {
			if (baseSectionRecord) {
				return baseSectionRecord->data.layoutResType == 1;
			}

			return false;
		}


	protected:
		const BFADB2CharBaseSectionRecord* baseSectionRecord;
		const IFileDataGameDatabase* fileDataDB;

		inline std::array<GameFileUri, 3> findTextureFileIds(uint32_t id1, uint32_t id2, uint32_t id3) const {
			
			std::array<GameFileUri, 3> result = {
				fileDataDB->findByMaterialResId(id1),
				fileDataDB->findByMaterialResId(id2),
				fileDataDB->findByMaterialResId(id3),
			};
			
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

	using BFAItemRecordAdaptor = ModernItemRecordAdaptor<BFADB2ItemRecord, BFADB2ItemSparseRecord, BFADB2ItemAppearanceRecord>;

	using BFAItemDisplayInfoRecordAdaptor = ModernItemDisplayInfoRecordAdaptor<BFADB2ItemDisplayInfoRecord, BFADB2ItemDisplayInfoMaterialResRecord>;

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