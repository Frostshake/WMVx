#pragma once
#include "GameDataset.h"
#include "DFDatasetAdaptors.h"
#include "DB2File.h"
#include "DFRecordDefinitions.h"
#include "DB2BackedDataset.h"

#define BOOST_METAPARSE_LIMIT_STRING_SIZE 64

#include "boost/mpl/string.hpp"
#include "boost/metaparse/string.hpp"

#include "GenericDB2Dataset.h"
#include "ReferenceSource.h"

#include <algorithm>
#include <execution>

#include "BFADatasets.h"

namespace core {


	using DFAnimationDataDataset = BFAAnimationDataDataset;

	using DFCharRacesDataset = GenericDB2Dataset<DatasetCharacterRaces, DFCharRacesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/chrraces.db2")>::value>;

	using DFCharacterComponentTextureDataset = ModernCharacterComponentTextureDataset<DFCharacterComponentTextureAdaptor, DFDB2CharComponentTextureSectionsRecord, DFDB2CharComponentTextureLayoutsRecord>;

	using DFCharacterFacialHairStylesDataset = BFACharacterFacialHairStylesDataset;

	using DFCharHairGeosetsDataset = BFACharHairGeosetsDataset;

	using DFCreatureModelDataDataset = GenericDB2Dataset<DatasetCreatureModelData, DFCreatureModelDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creaturemodeldata.db2")>::value >;

	using DFCreatureDisplayDataset = GenericDB2Dataset<DatasetCreatureDisplay, DFCreatureDisplayRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creaturedisplayinfo.db2")>::value >;

	using DFItemDisplayInfoDataset = ModernItemDisplayInfoDataset< DFItemDisplayInfoRecordAdaptor, DFDB2ItemDisplayInfoMaterialResRecord>;

	using DFItemDataset = ModernItemDataset<DFItemRecordAdaptor, DFDB2ItemSparseRecord, DFDB2ItemAppearanceRecord, DFDB2ItemModifiedAppearanceRecord>;

	using DFItemDataset = ModernItemDataset<DFItemRecordAdaptor, DFDB2ItemSparseRecord, DFDB2ItemAppearanceRecord, DFDB2ItemModifiedAppearanceRecord>;

	using DFNPCsDataset = GenericDB2Dataset<DatasetNPCs, DFNPCRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creature.db2")>::value >;

};