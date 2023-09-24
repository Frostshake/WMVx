#pragma once
#include <cstdint>

namespace core {
	// Referenced from https://wowdev.wiki/BLP 

	struct BLPHeader {
		uint8_t signature[4];	//BLP2
		uint32_t version;
		uint8_t colorEncoding;
		uint8_t alphaSize;
		uint8_t preferredFormat;
		uint8_t hasMips;
		uint32_t width;
		uint32_t height;
		uint32_t mipOffsets[16];
		uint32_t mipSizes[16];
		//TODO wow dev wiki has a 'extended' / pallette field here!?
	};

	enum BLPColorEncoding : uint8_t {
		COLOR_JPEG = 0, // not supported
		COLOR_PALETTE = 1,
		COLOR_DXT = 2,
		COLOR_ARGB8888 = 3,
		COLOR_ARGB8888_dup = 4,    // same decompression, likely other PIXEL_FORMAT
	};

	enum BLPPixelFormat : uint8_t {
		PIXEL_DXT1 = 0,
		PIXEL_DXT3 = 1,
		PIXEL_ARGB8888 = 2,
		PIXEL_ARGB1555 = 3,
		PIXEL_ARGB4444 = 4,
		PIXEL_RGB565 = 5,
		PIXEL_A8 = 6,
		PIXEL_DXT5 = 7,
		PIXEL_UNSPECIFIED = 8,
		PIXEL_ARGB2565 = 9,
		PIXEL_BC5 = 11, // DXGI_FORMAT_BC5_UNORM 
		NUM_PIXEL_FORMATS = 12, // (no idea if format=10 exists)
	};
};