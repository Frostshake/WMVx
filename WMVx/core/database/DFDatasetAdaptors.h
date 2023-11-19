#pragma once
#include "GameDatasetAdaptors.h"
#include "DFRecordDefinitions.h"
#include "DB2BackedDataset.h"

#include "BFADatasetAdaptors.h"

namespace core {

	using DFAnimationDataRecordAdaptor = BFAAnimationDataRecordAdaptor;

	class DFCharRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public DB2BackedAdaptor<DFDB2ChrRacesRecord> {
	public:
		using DB2BackedAdaptor<DFDB2ChrRacesRecord>::DB2BackedAdaptor;

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
			return std::nullopt;
		}
	};

	class DFCharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor, public DB2BackedAdaptor<DFDB2CharSectionConditionRecord> {
	public:
		DFCharSectionsRecordAdaptor(const DFDB2CharSectionConditionRecord* handle,
			const DB2File<DFDB2CharSectionConditionRecord>* db2,
			const DB2File<DFDB2CharSectionConditionRecord>::SectionView* section_view,
			const FileDataGameDatabase* fdDB,
			const DFDB2CharBaseSectionRecord* baseSectionRec) :
			DB2BackedAdaptor<DFDB2CharSectionConditionRecord>(handle, db2, section_view),
			fileDataDB(fdDB),
			baseSectionRecord(baseSectionRec) {}
		DFCharSectionsRecordAdaptor(DFCharSectionsRecordAdaptor&&) = default;
		virtual ~DFCharSectionsRecordAdaptor() {}

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
			return { 0u,0u,0u };	//TODODF
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
		const DFDB2CharBaseSectionRecord* baseSectionRecord;
		const FileDataGameDatabase* fileDataDB;

	};

	class DFCharacterComponentTextureAdaptor : public CharacterComponentTextureAdaptor {
	public:
		DFCharacterComponentTextureAdaptor(const DFDB2CharComponentTextureLayoutsRecord* layout,
			const std::vector<const DFDB2CharComponentTextureSectionsRecord*>& sections)
			: layoutHandle(layout), sectionHandles(sections) {}

		DFCharacterComponentTextureAdaptor(DFCharacterComponentTextureAdaptor&&) = default;
		virtual ~DFCharacterComponentTextureAdaptor() {}

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
		const DFDB2CharComponentTextureLayoutsRecord* layoutHandle;
		std::vector<const DFDB2CharComponentTextureSectionsRecord*> sectionHandles;
	};

	using DFCharacterFacialHairStylesRecordAdaptor = BFACharacterFacialHairStylesRecordAdaptor;

	using DFCharHairGeosetsRecordAdaptor = BFACharHairGeosetsRecordAdaptor;

	class DFCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureModelDataRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureModelDataRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		GameFileUri getModelUri() const override {
			return this->handle->data.fileDataID;
		}
	};

	class DFCreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureDisplayInfoRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureDisplayInfoRecord>::DB2BackedAdaptor;

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

	using DFItemRecordAdaptor = ModernItemRecordAdaptor<DFDB2ItemRecord, DFDB2ItemSparseRecord, DFDB2ItemAppearanceRecord>;
	using DFItemDisplayInfoRecordAdaptor = ModernItemDisplayInfoRecordAdaptor<DFDB2ItemDisplayInfoRecord, DFDB2ItemDisplayInfoMaterialResRecord>;


	class DFNPCRecordAdaptor : public NPCRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->handle->data.displayId[0];
		}

		constexpr uint32_t getType() const override {
			return this->handle->data.creatureType;
		}

		QString getName() const override {
			return this->db2->getString(this->handle->data.nameLang, this->section_view, this->handle->recordIndex, 0);
		}
	};


}