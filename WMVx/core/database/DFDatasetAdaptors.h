#pragma once
#include "GameDatasetAdaptors.h"
#include "DFDefinitions.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"

namespace core {

	using DFAnimationDataRecordAdaptorNext = ModernAnimationDataRecordAdaptorNext<db_df::AnimationDataRecord>;
	using DFCharRacesRecordAdaptorNext = ModernCharRacesRecordAdaptorNext<db_df::ChrRacesRecord>;
	using DFCharacterFacialHairStylesRecordAdaptorNext = ModernCharacterFacialHairStylesDataset<db_df::CharacterFacialHairStylesRecord>;
	using DFCharHairGeosetsRecordAdaptorNext = ModernCharHairGeosetsRecordAdaptor<db_df::CharHairGeosetsRecord>;
	using DFCharacterComponentTextureAdaptorNext = ModernCharacterComponentTextureAdaptor<db_df::CharComponentTextureLayoutsRecord, db_df::CharComponentTextureSectionsRecord>;
	using DFCreatureModelDataRecordAdaptorNext = ModernCreatureModelDataRecordAdaptor<db_df::CreatureModelDataRecord>;
	using DFCreatureDisplayRecordAdaptorNext = ModernCreatureDisplayRecordAdaptorNext<db_df::CreatureDisplayInfoRecord>;
	using DFNPCRecordAdaptorNext = ModernNPCRecordAdaptorNext<db_df::CreatureRecord>;
	using DFItemDisplayInfoRecordAdaptorNext = ModernItemDisplayInfoRecordAdaptorNext<db_df::ItemDisplayInfoRecord, db_df::ItemDisplayInfoMaterialResRecord>;
	using DFItemRecordAdaptorNext = ModernItemRecordAdaptorNext<db_df::ItemRecord, db_df::ItemSparseRecord, db_df::ItemAppearanceRecord>;

}