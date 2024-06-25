#pragma once

#include "GameDataset.h"
#include "VanillaDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	using VanillaAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, VanillaAnimationDataRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaChrRacesDataset = GenericDBCDataset<DatasetCharacterRaces, VanillaChrRacesRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, VanillaCharacterFacialHairStylesRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, VanillaCharHairGeosetsRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, VanillaCharSectionsRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, VanillaCreatureModelDataRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaCreatureDisplayDataset = GenericDBCCreatureDisplayInfoDataset<VanillaCreatureDisplayInfoRecordAdaptor, VanillaCreatureModelDisplayInfoExtraRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;

	class VanillaItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = VanillaItemRecordAdaptor;

		VanillaItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			for (auto it = itemCacheRecords.begin(); it != itemCacheRecords.end(); ++it) {
				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it))
				);
			}

		}
		VanillaItemDataset(VanillaItemDataset&&) = default;
		virtual ~VanillaItemDataset() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using VanillaItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, VanillaItemDisplayInfoRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaItemVisualDataset = GenericDBCDataset<DatasetItemVisual, VanillaItemVisualRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, VanillaItemVisualEffectRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;
	using VanillaSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, VanillaSpellItemEnchantmentRecordAdaptor, WDBReader::Database::DBCVersion::VANILLA>;

};