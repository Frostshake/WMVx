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


	using DFAnimationDataDatasetNext = ModernAnimationDataDatasetNext<DFAnimationDataRecordAdaptorNext>;
	using DFCharRacesDatasetNext = GenericDB2DatasetNext<DatasetCharacterRaces, DFCharRacesRecordAdaptorNext>;
	using DFCharacterFacialHairStylesDatasetNext = GenericDB2DatasetNext<DatasetCharacterFacialHairStyles, DFCharacterFacialHairStylesRecordAdaptorNext>;
	using DFCharHairGeosetsDatasetNext = GenericDB2DatasetNext<DatasetCharacterHairGeosets, DFCharHairGeosetsRecordAdaptorNext>;;
	using DFCharacterComponentTextureDatasetNext = ModernCharacterComponentTextureDatasetNext<DFCharacterComponentTextureAdaptorNext, db_df::CharComponentTextureLayoutsRecord, db_df::CharComponentTextureSectionsRecord>;
	using DFCreatureModelDataDatasetNext = GenericDB2DatasetNext<DatasetCreatureModelData, DFCreatureModelDataRecordAdaptorNext>;
	using DFCreatureDisplayDatasetNext = ModernCreatureDisplayDatasetNext<DFCreatureDisplayRecordAdaptorNext>;
	using DFNPCsDatasetNext = GenericDB2DatasetNext<DatasetNPCs, DFNPCRecordAdaptorNext>;
	using DFItemDisplayInfoDatasetNext = ModernItemDisplayInfoDatasetNext<DFItemDisplayInfoRecordAdaptorNext, db_df::ItemDisplayInfoMaterialResRecord>;
	using DFItemDatasetNext = ModernItemDatasetNext<DFItemRecordAdaptorNext, db_df::ItemSparseRecord, db_df::ItemAppearanceRecord, db_df::ItemModifiedAppearanceRecord>;

};