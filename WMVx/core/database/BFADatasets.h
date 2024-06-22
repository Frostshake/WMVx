#pragma once
#include "GameDataset.h"
#include "BFADatasetAdaptors.h"
#include "BFADefinitions.h"

#include "GenericDB2Dataset.h"
#include "ReferenceSource.h"
#include "ModernDatasets.h"

#include <algorithm>
#include <execution>

namespace core {

	using BFAAnimationDataDatasetNext = ModernAnimationDataDatasetNext<BFAAnimationDataRecordAdaptorNext>;
	using BFACharRacesDatasetNext = GenericDB2DatasetNext<DatasetCharacterRaces, BFACharRacesRecordAdaptorNext>;
	using BFACharacterFacialHairStylesDatasetNext = GenericDB2DatasetNext< DatasetCharacterFacialHairStyles, BFACharacterFacialHairStylesRecordAdaptorNext>;
	using BFACharHairGeosetsDatasetNext = GenericDB2DatasetNext< DatasetCharacterHairGeosets, BFACharHairGeosetsRecordAdaptorNext>;

	class BFACharSectionsDatasetNext : public DatasetCharacterSections {
	public:
		using Adaptor = BFACharSectionsRecordAdaptorNext;
		BFACharSectionsDatasetNext(CascFileSystem* fs, const IFileDataGameDatabase* fdDB) :
			DatasetCharacterSections(),
			fileDataDB(fdDB)
		{
			{
				auto base_section_file = fs->openFile("dbfilesclient/charbasesection.db2");
				auto db2 = WDBR::Database::makeDB2File<db_bfa::CharBaseSectionRecord, WDBR::Filesystem::CASCFileSource>(
					static_cast<CascFile*>(base_section_file.get())->release()
				);
				baseRecords.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						baseRecords.push_back(std::move(rec));
					}
				}
			}

			{
				auto sections_file = fs->openFile("dbfilesclient/charsections.db2");
				auto db2 = WDBR::Database::makeDB2File<db_bfa::CharSectionsRecord, WDBR::Filesystem::CASCFileSource>(
					static_cast<CascFile*>(sections_file.get())->release()
				);
				adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						const db_bfa::CharBaseSectionRecord* base_ptr = findBase(rec.data.baseSection);
						adaptors.push_back(
							std::make_unique<Adaptor>(std::move(rec), base_ptr, fileDataDB)
						);
					}
				}
			}
		}
		BFACharSectionsDatasetNext(BFACharSectionsDatasetNext&&) = default;
		virtual ~BFACharSectionsDatasetNext() = default;

		const std::vector<CharacterSectionRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterSectionRecordAdaptor*>&>(this->adaptors);
		}

	protected:

		const db_bfa::CharBaseSectionRecord* findBase(uint32_t baseSectionId) {
			for (auto it = baseRecords.cbegin(); it != baseRecords.cend(); ++it) {
				if (it->data.id == baseSectionId) {
					return &(*it);
				}
			}

			return nullptr;
		}

		std::vector<std::unique_ptr<Adaptor>> adaptors;
		std::vector<db_bfa::CharBaseSectionRecord> baseRecords;
		const IFileDataGameDatabase* fileDataDB;
	};

	using BFACharacterComponentTextureDatasetNext = ModernCharacterComponentTextureDatasetNext<BFACharacterComponentTextureAdaptorNext, db_bfa::CharComponentTextureLayoutsRecord, db_bfa::CharComponentTextureSectionsRecord>;
	using BFACreatureModelDataDatasetNext = GenericDB2DatasetNext<DatasetCreatureModelData, BFACreatureModelDataRecordAdaptorNext>;
	using BFACreatureDisplayDatasetNext = ModernCreatureDisplayDatasetNext<BFACreatureDisplayRecordAdaptorNext, BFACreatureDisplayExtraRecordAdaptorNext>;
	using BFANPCsDatasetNext = GenericDB2DatasetNext<DatasetNPCs, BFANPCRecordAdaptorNext>;
	using BFAItemDisplayInfoDatasetNext = ModernItemDisplayInfoDatasetNext< BFAItemDisplayInfoRecordAdaptorNext, db_bfa::ItemDisplayInfoMaterialResRecord>;
	using BFAItemDatasetNext = ModernItemDatasetNext<BFAItemRecordAdaptorNext, db_bfa::ItemSparseRecord, db_bfa::ItemAppearanceRecord, db_bfa::ItemModifiedAppearanceRecord>;

};