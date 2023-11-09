#pragma once

#include "GameDatabase.h"
#include "DFDatasets.h"

namespace core {

	class DFGameDatabase : public GameDatabase
	{
	public:
		DFGameDatabase() : GameDatabase() {}
		DFGameDatabase(DFGameDatabase&&) = default;
		virtual ~DFGameDatabase() {}

		void load(const GameFileSystem* const fs) override {

			auto* const cascFS = (CascFileSystem*)(fs);

			//TODO DF

		}

	protected:


	};
};