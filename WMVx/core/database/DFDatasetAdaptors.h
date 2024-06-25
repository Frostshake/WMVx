#pragma once
#include "GameDatasetAdaptors.h"
#include "DFDefinitions.h"
#include "FileDataGameDatabase.h"
#include "ModernDatasetAdaptors.h"

namespace core {

	using DFAnimationDataRecordAdaptor = ModernAnimationDataRecordAdaptor<db_df::AnimationDataRecord>;
	using DFCharRacesRecordAdaptor = ModernCharRacesRecordAdaptor<db_df::ChrRacesRecord>;
	using DFCharacterFacialHairStylesRecordAdaptor = ModernCharacterFacialHairStylesDataset<db_df::CharacterFacialHairStylesRecord>;
	using DFCharHairGeosetsRecordAdaptor = ModernCharHairGeosetsRecordAdaptor<db_df::CharHairGeosetsRecord>;
	using DFCharacterComponentTextureAdaptor = ModernCharacterComponentTextureAdaptor<db_df::CharComponentTextureLayoutsRecord, db_df::CharComponentTextureSectionsRecord>;
	using DFCreatureModelDataRecordAdaptor = ModernCreatureModelDataRecordAdaptor<db_df::CreatureModelDataRecord>;
	using DFCreatureDisplayRecordAdaptor = ModernCreatureDisplayRecordAdaptor<db_df::CreatureDisplayInfoRecord>;
	using DFNPCRecordAdaptor = ModernNPCRecordAdaptor<db_df::CreatureRecord>;
	using DFItemDisplayInfoRecordAdaptor = ModernItemDisplayInfoRecordAdaptor<db_df::ItemDisplayInfoRecord, db_df::ItemDisplayInfoMaterialResRecord>;
	using DFItemRecordAdaptor = ModernItemRecordAdaptor<db_df::ItemRecord, db_df::ItemSparseRecord, db_df::ItemAppearanceRecord>;

}