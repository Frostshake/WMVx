#pragma once
#include "GameDatasetAdaptors.h"
#include "BFADefinitions.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"

namespace core {

	using BFAAnimationDataRecordAdaptor = ModernAnimationDataRecordAdaptor<db_bfa::AnimationDataRecord>;
	using BFACharRacesRecordAdaptor = ModernCharRacesRecordAdaptor<db_bfa::ChrRacesRecord>;
	using BFACharacterFacialHairStylesRecordAdaptor = ModernCharacterFacialHairStylesDataset<db_bfa::CharacterFacialHairStylesRecord>;
	using BFACharHairGeosetsRecordAdaptor = ModernCharHairGeosetsRecordAdaptor<db_bfa::CharHairGeosetsRecord>;

	class BFACharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor {
	public:
		using Record = db_bfa::CharSectionsRecord;
		BFACharSectionsRecordAdaptor(db_bfa::CharSectionsRecord&& section,
			const db_bfa::CharBaseSectionRecord* base,
			const IFileDataGameDatabase* fdDB) :
			_sectionsRecord(std::move(section)), _baseRecord(base), _fileDataDB(fdDB) 
		{}
		BFACharSectionsRecordAdaptor(BFACharSectionsRecordAdaptor&&) = default;
		virtual ~BFACharSectionsRecordAdaptor() = default;
		
		constexpr uint32_t getId() const override {
			return _sectionsRecord.data.id;
		}

		constexpr uint32_t getRaceId() const override {
			return _sectionsRecord.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(_sectionsRecord.data.sexId);
		}

		constexpr CharacterSectionType getType() const override {
			return static_cast<CharacterSectionType>(_baseRecord ? _baseRecord->data.variationEnum : 0);
		}

		std::array<GameFileUri, 3> getTextures() const override {

			const auto search = CharacterRelationSearchContext::makeModern(
				_sectionsRecord.data.sexId,
				_sectionsRecord.data.raceId,
				CharacterRelationSearchContext::MODERN_GENDER_IGNORE,
				0
			);

			std::array<uint32_t, 3> input = {
				_sectionsRecord.data.materialResourcesId[0],
				_sectionsRecord.data.materialResourcesId[1],
				_sectionsRecord.data.materialResourcesId[2]
			};

			return GameFileUri::arrayConvert(
				_fileDataDB->findByMaterialResIdFixed(input, search)
			);
		}

		constexpr uint32_t getSection() const override {
			return _sectionsRecord.data.variationIndex;
		}

		constexpr uint32_t getVariationIndex() const override {
			return _sectionsRecord.data.colorIndex;
		}

		constexpr bool isHD() const override {
			if (_baseRecord) {
				return _baseRecord->data.layoutResType == 1;
			}

			return false;
		}
		
	protected:
		db_bfa::CharSectionsRecord _sectionsRecord;
		const db_bfa::CharBaseSectionRecord* _baseRecord;
		const IFileDataGameDatabase* _fileDataDB;
	};

	using BFACharacterComponentTextureAdaptor = ModernCharacterComponentTextureAdaptor<db_bfa::CharComponentTextureLayoutsRecord, db_bfa::CharComponentTextureSectionsRecord>;
	using BFACreatureModelDataRecordAdaptor = ModernCreatureModelDataRecordAdaptor<db_bfa::CreatureModelDataRecord>;
	using BFACreatureDisplayExtraRecordAdaptor = ModernCreatureDisplayInfoExtraRecordAdaptor<db_bfa::CreatureDisplayInfoExtraRecord>;
	using BFACreatureDisplayRecordAdaptor = ModernCreatureDisplayRecordAdaptor<db_bfa::CreatureDisplayInfoRecord>;
	using BFANPCRecordAdaptor = ModernNPCRecordAdaptor<db_bfa::CreatureRecord>;
	using BFAItemDisplayInfoRecordAdaptor = ModernItemDisplayInfoRecordAdaptor<db_bfa::ItemDisplayInfoRecord, db_bfa::ItemDisplayInfoMaterialResRecord>;
	using BFAItemRecordAdaptor = ModernItemRecordAdaptor<db_bfa::ItemRecord, db_bfa::ItemSparseRecord, db_bfa::ItemAppearanceRecord>;

}