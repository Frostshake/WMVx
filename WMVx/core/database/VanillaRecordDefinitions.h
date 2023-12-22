#pragma once

#include <cstdint>
#include "../utility/Vector3.h"
#include "DBCDefinitions.h"

namespace core {
	struct VanillaDBCLangStringRef {
		DBCStringRef enUS; //also enGB
		DBCStringRef koKR;
		DBCStringRef frFR;
		DBCStringRef deDE;
		DBCStringRef enCN; //also zhCN
		DBCStringRef enTW; //also zhTW
		DBCStringRef esES;
		DBCStringRef esMX;
		uint32_t flags;
	};

	struct VanillaDBCAnimationDataRecord {
		uint32_t id;
		DBCStringRef name;
		uint32_t weaponFlags;
		uint32_t bodyFlags;
		uint32_t flags;
		uint32_t fallbackId;
		uint32_t previousId;
	};


	struct VanillaDBCCharRacesRecord {
		uint32_t id;
		uint32_t flags;
		uint32_t factionId;
		uint32_t explorationSoundId;
		uint32_t maleModelDisplayId;
		uint32_t femaleModelDisplayId;
		DBCStringRef clientPrefix;
		float mountScale;
		uint32_t baseLanguage;
		uint32_t creatureTypeId;
		uint32_t loginSpellId;
		uint32_t stunSpellId;
		uint32_t resSicknessSpellId;
		uint32_t splashSoundId;
		uint32_t startingTaxiNodes;
		DBCStringRef clientFileString;
		uint32_t cinematicSequenceId;
		VanillaDBCLangStringRef raceNameNeutral;
		DBCStringRef facialHairCustomization[2];
		DBCStringRef hairCustomization;
	};

	struct VanillaDBCCharSectionsRecord {
		uint32_t id;
		uint32_t raceId;
		uint32_t sexId;
		uint32_t type;
		uint32_t section;
		uint32_t variationIndex;	
		DBCStringRef texture1;
		DBCStringRef texture2;
		DBCStringRef texture3;
		uint32_t flags;
	};

	struct VanillaDBCCharacterFacialHairStylesRecord {
		uint32_t raceId;
		uint32_t sexId;
		uint32_t variationId;
		uint32_t geoset1;
		uint32_t geoset2;
		uint32_t geoset3;
		uint32_t geoset4;
		uint32_t geoset5;
		uint32_t geoset6;
	};

	struct VanillaDBCCharHairGeosetsRecord {
		uint32_t id;
		uint32_t raceId;
		uint32_t sexId;
		uint32_t hairType;
		uint32_t geoset;
		uint32_t showScalp;
	};

	struct VanillaDBCCreatureModelDataRecord {
		uint32_t id;
		uint32_t flags;
		DBCStringRef modelName;
		uint32_t sizeClass;
		float modelScale;
		uint32_t bloodLevelId;
		uint32_t footprintTextureId;
		float footprintTextureLength;
		float footprintTextureWidth;
		float footprintTextureScale;
		uint32_t foleyMaterialId;
		uint32_t footstepShakeSize;
		uint32_t deathThudShakeSize;
		float collisionWidth;
		float collisionHeight;
		float mountHeight;
	};

	struct VanillaDBCCreatureDisplayInfoRecord {
		uint32_t id;
		uint32_t modelId;
		uint32_t soundId;
		uint32_t extraDisplayInformationId;
		float scale;
		uint32_t opacity;
		DBCStringRef texture[3];
		uint32_t sizeClass;
		uint32_t bloodId;	
		uint32_t npcSoundsId;
	};

	struct VanillaDBCCreatureDisplayInfoExtraRecord {
		uint32_t id;
		uint32_t raceId;
		uint32_t sexId;
		uint32_t skinId;
		uint32_t faceId;
		uint32_t hairStyleId;
		uint32_t hairColorId;
		uint32_t facialHairId;
		uint32_t npcItemDisplayId[10];
		DBCStringRef bakeName;
	};

	struct VanillaDBCItemDisplayInfoRecord {
		uint32_t id;
		DBCStringRef modelLeft;
		DBCStringRef modelRight;
		DBCStringRef modelLeftTexture;
		DBCStringRef modelRightTexture;
		DBCStringRef icon;
		uint32_t geosetGlovesFlags;	//TODO GEOSET NAMES DONT APPEAR TO ACCURATELY REFLECT WHAT THEY ARE USED FOR!
		uint32_t geosetBracerFlags;
		uint32_t geosetRobeFlags;
		uint32_t geosetBootsFlags;
		uint32_t spellVisualId;
		uint32_t groupSoundIndexId;
		uint32_t helmetGeosetVisMaleId;
		uint32_t helmentGeosetVisFemaleId;
		DBCStringRef textureUpperArm;
		DBCStringRef textureLowerArm;
		DBCStringRef textureHands;
		DBCStringRef textureUpperChest;
		DBCStringRef textureLowerChest;
		DBCStringRef textureUpperLeg;
		DBCStringRef textureLowerLeg;
		DBCStringRef textureFoot;
		uint32_t itemVisualId;		
	};

	struct VanillaDBCItemVisualRecord {
		uint32_t id;
		uint32_t visualEffectId[5];
	};

	struct VanillaDBCItemVisualEffectRecord {
		uint32_t id;
		DBCStringRef model;
	};

	struct VanillaDBCSpellItemEnchantmentRecord {
		uint32_t id;
		uint32_t effects[3];
		uint32_t effectPointsMin[3];
		uint32_t effectPointsMax[3];
		uint32_t effectArgs[3];
		VanillaDBCLangStringRef name;
		uint32_t itemVisualId;
		uint32_t flags;
	};

};