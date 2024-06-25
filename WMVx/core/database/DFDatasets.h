#pragma once
#include "GameDataset.h"
#include "DFDatasetAdaptors.h"
#include "DFDefinitions.h"

#include "GenericDB2Dataset.h"
#include "ReferenceSource.h"
#include "ModernDatasets.h"

#include <algorithm>
#include <execution>

namespace core {


	using DFAnimationDataDataset = ModernAnimationDataDataset<DFAnimationDataRecordAdaptor>;
	using DFCharRacesDataset = GenericDB2Dataset<DatasetCharacterRaces, DFCharRacesRecordAdaptor>;
	using DFCharacterFacialHairStylesDataset = GenericDB2Dataset<DatasetCharacterFacialHairStyles, DFCharacterFacialHairStylesRecordAdaptor>;
	using DFCharHairGeosetsDataset = GenericDB2Dataset<DatasetCharacterHairGeosets, DFCharHairGeosetsRecordAdaptor>;;
	using DFCharacterComponentTextureDataset = ModernCharacterComponentTextureDataset<DFCharacterComponentTextureAdaptor, db_df::CharComponentTextureLayoutsRecord, db_df::CharComponentTextureSectionsRecord>;
	using DFCreatureModelDataDataset = GenericDB2Dataset<DatasetCreatureModelData, DFCreatureModelDataRecordAdaptor>;
	using DFCreatureDisplayDataset = ModernCreatureDisplayDataset<DFCreatureDisplayRecordAdaptor>;
	using DFNPCsDataset = GenericDB2Dataset<DatasetNPCs, DFNPCRecordAdaptor>;
	using DFItemDisplayInfoDataset = ModernItemDisplayInfoDataset<DFItemDisplayInfoRecordAdaptor, db_df::ItemDisplayInfoMaterialResRecord>;
	using DFItemDataset = ModernItemDataset<DFItemRecordAdaptor, db_df::ItemSparseRecord, db_df::ItemAppearanceRecord, db_df::ItemModifiedAppearanceRecord>;

};