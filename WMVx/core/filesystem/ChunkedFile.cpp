#include "../../stdafx.h"
#include "ChunkedFile.h"


namespace core {

	const std::vector<Chunk::id_t> ChunkedFile::KNOWN_CHUNKS =
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
		const auto filesize = file->getFileSize();

		if (filesize >= sizeof(ChunkHeader)) {
			ChunkHeader header;
			file->read(&header, sizeof(header));
			const std::string_view magic((char*)header.id, MAGIC_SIZE);
			const bool is_known_chunk = std::find(KNOWN_CHUNKS.begin(), KNOWN_CHUNKS.end(), magic) != KNOWN_CHUNKS.end();

			if (is_known_chunk && header.size <= filesize) {
				uint32_t offset = 0;

				while (offset < filesize)
				{
					ChunkHeader chunkHead;
					file->read(&chunkHead, sizeof(chunkHead), offset);

					offset += sizeof(ChunkHeader);
					Chunk chunk = {
						std::string((char*)chunkHead.id, MAGIC_SIZE),
						chunkHead.size,
						offset
					};

					assert(!chunks.contains(chunk.id));
					chunks.insert({ chunk.id, std::move(chunk) });

					offset += chunkHead.size;
				}
			}
		}
	}

	const Chunk* ChunkedFile::get(const Chunk::id_t& id) const {
		const auto& res = chunks.find(id); 
		if (res == chunks.end()) {
			return nullptr;
		}

		return &(res->second);
	}
};