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

	using BFAAnimationDataDataset = ModernAnimationDataDataset<BFAAnimationDataRecordAdaptor>;
	using BFACharRacesDataset = GenericDB2Dataset<DatasetCharacterRaces, BFACharRacesRecordAdaptor>;
	using BFACharacterFacialHairStylesDataset = GenericDB2Dataset< DatasetCharacterFacialHairStyles, BFACharacterFacialHairStylesRecordAdaptor>;
	using BFACharHairGeosetsDataset = GenericDB2Dataset< DatasetCharacterHairGeosets, BFACharHairGeosetsRecordAdaptor>;

	class BFACharSectionsDataset : public DatasetCharacterSections {
	public:
		using Adaptor = BFACharSectionsRecordAdaptor;
		BFACharSectionsDataset(CascFileSystem* fs, const IFileDataGameDatabase* fdDB) :
			DatasetCharacterSections(),
			fileDataDB(fdDB)
		{
			{
				auto base_section_file = fs->openFile("dbfilesclient/charbasesection.db2");
				auto db2 = WDBR::Database::makeDB2File<db_bfa::CharBaseSectionRecord>(
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
				auto db2 = WDBR::Database::makeDB2File<db_bfa::CharSectionsRecord>(
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
		BFACharSectionsDataset(BFACharSectionsDataset&&) = default;
		virtual ~BFACharSectionsDataset() = default;

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

	using BFACharacterComponentTextureDataset = ModernCharacterComponentTextureDataset<BFACharacterComponentTextureAdaptor, db_bfa::CharComponentTextureLayoutsRecord, db_bfa::CharComponentTextureSectionsRecord>;
	using BFACreatureModelDataDataset = GenericDB2Dataset<DatasetCreatureModelData, BFACreatureModelDataRecordAdaptor>;
	using BFACreatureDisplayDataset = ModernCreatureDisplayDataset<BFACreatureDisplayRecordAdaptor, BFACreatureDisplayExtraRecordAdaptor>;
	using BFANPCsDataset = GenericDB2Dataset<DatasetNPCs, BFANPCRecordAdaptor>;
	using BFAItemDisplayInfoDataset = ModernItemDisplayInfoDataset< BFAItemDisplayInfoRecordAdaptor, db_bfa::ItemDisplayInfoMaterialResRecord>;
	using BFAItemDataset = ModernItemDataset<BFAItemRecordAdaptor, db_bfa::ItemSparseRecord, db_bfa::ItemAppearanceRecord, db_bfa::ItemModifiedAppearanceRecord>;

};