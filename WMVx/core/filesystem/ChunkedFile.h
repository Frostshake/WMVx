#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include <unordered_map>
#include "CascFileSystem.h"

namespace core {

	struct ChunkHeader {
		uint8_t id[4];
		uint32_t size;
	};

	struct Chunk {
		using id_t = std::string;
		id_t id;
		uint32_t size;
		uint32_t offset;
	};

	class ChunkedFile {
	public:
		static constexpr int32_t MAGIC_SIZE = 4;

		ChunkedFile() = default;
		ChunkedFile(ChunkedFile&&) = default;
		virtual ~ChunkedFile() { }

		static const std::vector<Chunk::id_t> KNOWN_CHUNKS;

		void open(CascFile* file);

		bool isChunked() const {
			return !chunks.empty();
		}

		const Chunk* get(const Chunk::id_t& id) const;

	protected:

		// its is assumed that the chunk id is unique.
		std::unordered_map<Chunk::id_t, Chunk> chunks;

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