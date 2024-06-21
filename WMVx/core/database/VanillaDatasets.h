#pragma once

#include "GameDataset.h"
#include "VanillaDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	using VanillaAnimationDataDatasetNext = GenericLegacyDBCDataset<DatasetAnimationData, VanillaAnimationDataRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaChrRacesDatasetNext = GenericLegacyDBCDataset<DatasetCharacterRaces, VanillaChrRacesRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharacterFacialHairStylesDatasetNext = GenericLegacyDBCDataset<DatasetCharacterFacialHairStyles, VanillaCharacterFacialHairStylesRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharHairGeosetsDatasetNext = GenericLegacyDBCDataset<DatasetCharacterHairGeosets, VanillaCharHairGeosetsRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharSectionsDatasetNext = GenericLegacyDBCDataset<DatasetCharacterSections, VanillaCharSectionsRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCreatureModelDataDatasetNext = GenericLegacyDBCDataset<DatasetCreatureModelData, VanillaCreatureModelDataRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCreatureDisplayDatasetNext = GenericLegacyDBCCreatureDisplayInfoDataset<DatasetCreatureDisplay, VanillaCreatureDisplayInfoRecordAdaptorNext, VanillaCreatureModelDisplayInfoExtraRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;

	class VanillaItemDatasetNext : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = VanillaItemRecordAdaptor;

		VanillaItemDatasetNext(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			for (auto it = itemCacheRecords.begin(); it != itemCacheRecords.end(); ++it) {
				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it))
				);
			}

		}
		VanillaItemDatasetNext(VanillaItemDatasetNext&&) = default;
		virtual ~VanillaItemDatasetNext() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using VanillaItemDisplayInfoDatasetNext = GenericLegacyDBCDataset<DatasetItemDisplay, VanillaItemDisplayInfoRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaItemVisualDatasetNext = GenericLegacyDBCDataset<DatasetItemVisual, VanillaItemVisualRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaItemVisualEffectDatasetNext = GenericLegacyDBCDataset<DatasetItemVisualEffect, VanillaItemVisualEffectRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaSpellItemEnchantmentDatasetNext = GenericLegacyDBCDataset<DatasetSpellItemEnchantment, VanillaSpellItemEnchantmentRecordAdaptorNext, WDBReader::Database::DBCVersion::VANILLA>;

};