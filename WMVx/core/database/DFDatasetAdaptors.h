#pragma once
#include "GameDatasetAdaptors.h"
#include "DFRecordDefinitions.h"
#include "DB2BackedDataset.h"

#include "BFADatasetAdaptors.h"

namespace core {

	using DFAnimationDataRecordAdaptor = BFAAnimationDataRecordAdaptor;

	class DFCharRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public DB2BackedAdaptor<DFDB2ChrRacesRecord> {
	public:
		using DB2BackedAdaptor<DFDB2ChrRacesRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		QString getClientPrefix() const override {
			return this->db2->getString(this->handle->data.clientPrefix, this->section_view, this->handle->recordIndex, 0);
		}

		QString getClientFileString() const override {
			return this->db2->getString(this->handle->data.clientFileString, this->section_view, this->handle->recordIndex, 1);
		}

		std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const override {
			//TODODF
			return std::nullopt;
		}
	};

	class DFCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureModelDataRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureModelDataRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		GameFileUri getModelUri() const override {
			return this->handle->data.fileDataID;
		}
	};

	class DFCreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureDisplayInfoRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureDisplayInfoRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->handle->data.modelId;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				this->handle->data.textureVariationFileDataID[0],
				this->handle->data.textureVariationFileDataID[1],
				this->handle->data.textureVariationFileDataID[2]
			};
		}
	};


	class DFNPCRecordAdaptor : public NPCRecordAdaptor, public DB2BackedAdaptor<DFDB2CreatureRecord> {
	public:
		using DB2BackedAdaptor<DFDB2CreatureRecord>::DB2BackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->handle->data.displayId[0];
		}

		constexpr uint32_t getType() const override {
			return this->handle->data.creatureType;
		}

		QString getName() const override {
			return this->db2->getString(this->handle->data.nameLang, this->section_view, this->handle->recordIndex, 0);
		}
	};


	//TODO DF

}