#pragma once

#include <cstdint>
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"

namespace core {
	struct M2Chunk_AFID {
		uint16_t animationId;
		uint16_t variationId;
		uint32_t fileId;
	};

	struct M2Chunk_SKS1 {
		M2Array globalSequences;
		M2Array animations;
		M2Array animationLookup;
	};

	struct M2Chunk_SKA1 {
		M2Array attachments;
		M2Array attachmentLookup;
	};

	struct M2Chunk_SKB1 {
		M2Array bones;
		M2Array keyBoneLookup;
	};

	struct M2Chunk_SKPD {
		uint8_t unknown[8];
		uint32_t parentSkelFileId;
		uint8_t unknown2[4];
	};

	struct M2Chunk_TXID {
		uint32_t fileDataId;
	};
}