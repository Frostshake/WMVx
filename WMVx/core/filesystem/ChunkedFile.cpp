#include "../../stdafx.h"
#include "ChunkedFile.h"

namespace core {

	const std::vector<std::string> ChunkedFile::KNOWN_CHUNKS =
	{
	  "PFID",
	  "SFID",
	  "AFID",
	  "BFID",
	  "MD21",
	  "TXAC",
	  "EXPT",
	  "EXP2",
	  "PABC",
	  "PADC",
	  "PSBC",
	  "PEDC",
	  "SKID",
	  "TXID",
	  "LDV1",
	  "AFM2",
	  "AFSA",
	  "AFSB",
	  "SKL1",
	  "SKA1",
	  "SKB1",
	  "SKS1",
	  "SKPD"
	  /*
	  "MOHD",
	  "MOTX",
	  "MOMT",
	  "MOUV",
	  "MOGN",
	  "MOGI",
	  "MOSB",
	  "MOPV",
	  "MOPT",
	  "MOPR",
	  "MOVV",
	  "MOVB",
	  "MOLT",
	  "MODS",
	  "MODN",
	  "MODD",
	  "MFOG",
	  "MCVP",
	  "GFID",
	  "MOGP",
	  "MOPY",
	  "MOVI",
	  "MOVT",
	  "MONR",
	  "MOTV",
	  "MOBA",
	  "MOLR",
	  "MODR",
	  "MOBN",
	  "MOBR",
	  "MOCV",
	  "MLIQ",
	  "MORI",
	  "MORB",
	  "MOTA",
	  "MOBS",
	  "MDAL",
	  "MOPL",
	  "MOPB",
	  "MOLS",
	  "MOLP"
	  */
	};

	void ChunkedFile::open(CascFile* file) {
		auto filesize = file->getFileSize();

		if (filesize >= sizeof(ChunkHeader)) {
			auto buffer = std::vector<uint8_t>(filesize);
			file->read(buffer.data(), filesize);
			//TODO reading the whole file isnt needed, can we just read chunk headers and seek between them?

			ChunkHeader header;
			memcpy(&header, buffer.data(), sizeof(ChunkHeader));
			std::string magic = std::string((char*)header.id, 4);
			bool is_known_chunk = std::find(KNOWN_CHUNKS.begin(), KNOWN_CHUNKS.end(), magic) != KNOWN_CHUNKS.end();

			if (is_known_chunk && header.size <= filesize) {
				uint32_t offset = 0;

				while (offset < filesize)
				{
					ChunkHeader chunkHead;
					memcpy(&chunkHead, buffer.data() + offset, sizeof(ChunkHeader));

					offset += sizeof(ChunkHeader);

					Chunk chunk = Chunk();
					chunk.id = std::string((char*)chunkHead.id, 4);
					chunk.offset = offset;
					chunk.size = chunkHead.size;
					chunks.push_back(std::move(chunk));

					offset += chunkHead.size;
				}
			}
		}
	}

	std::optional<Chunk> ChunkedFile::get(std::string id) const {
		auto result = std::find_if(chunks.begin(), chunks.end(), [id](Chunk chunk) -> bool {
			return chunk.id == id;
		});

		return result == chunks.end() ? std::nullopt : std::optional<Chunk>(*result);
	}
};