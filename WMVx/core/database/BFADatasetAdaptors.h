#pragma once
#include "GameDatasetAdaptors.h"
#include "BFADefinitions.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"

namespace core {

	using BFAAnimationDataRecordAdaptorNext = ModernAnimationDataRecordAdaptorNext<db_bfa::AnimationDataRecord>;
	using BFACharRacesRecordAdaptorNext = ModernCharRacesRecordAdaptorNext<db_bfa::ChrRacesRecord>;
	using BFACharacterFacialHairStylesRecordAdaptorNext = ModernCharacterFacialHairStylesDataset<db_bfa::CharacterFacialHairStylesRecord>;
	using BFACharHairGeosetsRecordAdaptorNext = ModernCharHairGeosetsRecordAdaptor<db_bfa::CharHairGeosetsRecord>;

	class BFACharSectionsRecordAdaptorNext : public CharacterSectionRecordAdaptor {
	public:
		using Record = db_bfa::CharSectionsRecord;
		BFACharSectionsRecordAdaptorNext(db_bfa::CharSectionsRecord&& section,
			const db_bfa::CharBaseSectionRecord* base,
			const IFileDataGameDatabase* fdDB) :
			_sectionsRecord(std::move(section)), _baseRecord(base), _fileDataDB(fdDB) 
		{}
		BFACharSectionsRecordAdaptorNext(BFACharSectionsRecordAdaptorNext&&) = default;
		virtual ~BFACharSectionsRecordAdaptorNext() = default;
		
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

	using BFACharacterComponentTextureAdaptorNext = ModernCharacterComponentTextureAdaptor<db_bfa::CharComponentTextureLayoutsRecord, db_bfa::CharComponentTextureSectionsRecord>;
	using BFACreatureModelDataRecordAdaptorNext = ModernCreatureModelDataRecordAdaptor<db_bfa::CreatureModelDataRecord>;
	using BFACreatureDisplayExtraRecordAdaptorNext = ModernCreatureDisplayInfoExtraRecordAdaptorNext<db_bfa::CreatureDisplayInfoExtraRecord>;
	using BFACreatureDisplayRecordAdaptorNext = ModernCreatureDisplayRecordAdaptorNext<db_bfa::CreatureDisplayInfoRecord>;
	using BFANPCRecordAdaptorNext = ModernNPCRecordAdaptorNext<db_bfa::CreatureRecord>;
	using BFAItemDisplayInfoRecordAdaptorNext = ModernItemDisplayInfoRecordAdaptorNext<db_bfa::ItemDisplayInfoRecord, db_bfa::ItemDisplayInfoMaterialResRecord>;
	using BFAItemRecordAdaptorNext = ModernItemRecordAdaptorNext<db_bfa::ItemRecord, db_bfa::ItemSparseRecord, db_bfa::ItemAppearanceRecord>;

}