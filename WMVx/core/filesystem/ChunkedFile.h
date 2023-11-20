#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include "CascFileSystem.h"

namespace core {

	struct ChunkHeader {
		uint8_t id[4];
		uint32_t size;
	};

	struct Chunk {
		std::string id;
		uint32_t size;
		uint32_t offset;
	};

	class ChunkedFile {
	public:
		ChunkedFile() = default;
		ChunkedFile(ChunkedFile&&) = default;
		virtual ~ChunkedFile() { }

		static const std::vector<std::string> KNOWN_CHUNKS;

		void open(CascFile* file);

		bool isChunked() const {
			return chunks.size() > 0;
		}

		std::optional<Chunk> get(std::string id) const;

		const std::vector<Chunk>& all() const {
			return chunks;
		}

	protected:
		std::vector<Chunk> chunks;

	};

	/// <summary>
	/// Utility for containing the chunk info and source file together.
	/// Note that the file isnt guarenteed to chunked.
	/// </summary>
	class ChunkedFileInstance {
	public:
		ChunkedFileInstance(CascFile* src) : file(src) {
			chunked.open(src);
		}
		ChunkedFileInstance(ChunkedFileInstance&&) = default;
		virtual ~ChunkedFileInstance() {}

		CascFile* file;
		ChunkedFile chunked;
	};
};