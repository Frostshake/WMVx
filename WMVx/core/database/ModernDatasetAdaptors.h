#pragma once

#include "DB2File.h"
#include "GameDatasetAdaptors.h"
#include "DB2BackedDataset.h"
#include "FileDataGameDatabase.h"

namespace core {
	template
		<class ItemRecord, class ItemSparseRecord, class ItemAppearanceRecord>
	class ModernItemRecordAdaptor : public ItemRecordAdaptor, public DB2BackedAdaptor<typename ItemRecord> {
	public:
		ModernItemRecordAdaptor(const ItemRecord* handle,
			const DB2File<ItemRecord>* db2,
			const typename DB2File<ItemRecord>::SectionView* section_view,
			const ItemSparseRecord* sparse,
			const ItemAppearanceRecord* appearance)
			: DB2BackedAdaptor<ItemRecord>(handle, db2, section_view),
			sparseHandle(sparse),
			appearanceHandle(appearance) {}
		ModernItemRecordAdaptor(ModernItemRecordAdaptor&&) = default;
		virtual ~ModernItemRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		constexpr uint32_t getItemDisplayInfoId() const override {
			return appearanceHandle->data.itemDisplayInfoId;
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return (ItemInventorySlotId)this->handle->data.inventoryType;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)this->handle->data.sheatheType;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return (ItemQualityId)sparseHandle->data.overallQualityId;
		}

		QString getName() const override {
			return sparseHandle->inlineStrings[4];
		}

	protected:
		const ItemSparseRecord* sparseHandle;
		const ItemAppearanceRecord* appearanceHandle;
	};

	template
		<class ItemDisplayInfoRecord, class ItemDisplayInfoMaterialResRecord>
	class ModernItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor, public DB2BackedAdaptor<typename ItemDisplayInfoRecord> {
	public:

		ModernItemDisplayInfoRecordAdaptor(const ItemDisplayInfoRecord* handle,
			const DB2File<ItemDisplayInfoRecord>* db2,
			const typename DB2File<ItemDisplayInfoRecord>::SectionView* section_view,
			const std::vector<const ItemDisplayInfoMaterialResRecord*>& materials,
			const IFileDataGameDatabase* fdDB
		)
			: DB2BackedAdaptor<ItemDisplayInfoRecord>(handle, db2, section_view),
			materials(materials),
			fileDataDB(fdDB) {}

		ModernItemDisplayInfoRecordAdaptor(ModernItemDisplayInfoRecordAdaptor&&) = default;
		virtual ~ModernItemDisplayInfoRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return this->handle->data.id;
		}

		std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot) const override {

			const bool same_resource = this->handle->data.modelResourcesId[0] == this->handle->data.modelResourcesId[1];

			if (same_resource) {
				const auto& temp = fileDataDB->findByModelResIdFixed<2>(this->handle->data.modelResourcesId[0]);
				return {
					temp[0],
					temp[1] > 0 ? temp[1] : temp[0]
				};
			}

			return {
				fileDataDB->findByModelResId(this->handle->data.modelResourcesId[0]),
				fileDataDB->findByModelResId(this->handle->data.modelResourcesId[1])
			};
		}

		constexpr uint32_t getGeosetGlovesFlags() const override {
			return this->handle->data.geosetGroup[0];
		}

		constexpr uint32_t getGeosetBracerFlags() const override {
			return this->handle->data.geosetGroup[1];
		}

		constexpr uint32_t getGeosetRobeFlags() const override {
			return this->handle->data.geosetGroup[2];
		}

		std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot) const override {

			const bool same_resource = this->handle->data.modelMaterialResourcesId[0] == this->handle->data.modelMaterialResourcesId[1];

			if (same_resource) {
				const auto& temp = fileDataDB->findByMaterialResIdFixed<2>(this->handle->data.modelMaterialResourcesId[0]);
				return {
					temp[0],
					temp[1] > 0 ? temp[1]: temp[0]
				};
			}

			return {
				fileDataDB->findByMaterialResId(this->handle->data.modelMaterialResourcesId[0]),
				fileDataDB->findByMaterialResId(this->handle->data.modelMaterialResourcesId[1])
			};
		}

		GameFileUri getTextureUpperArm() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 0) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerArm() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 1) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureHands() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 2) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperChest() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 3) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerChest() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 4) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureUpperLeg() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 5) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureLowerLeg() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 6) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		GameFileUri getTextureFoot() const override {
			for (auto* mat : materials) {
				if (mat->data.componentSection == 7) {
					return findTextureFileId(mat->data.materialResourcesId);
				}
			}

			return GameFileUri(0u);
		}

		constexpr uint32_t getItemVisualId() const override {
			return this->handle->data.itemVisualId;
		}

	protected:

		std::vector<const ItemDisplayInfoMaterialResRecord*> materials;
		const IFileDataGameDatabase* fileDataDB;

		inline GameFileUri::id_t findModelFileId(uint32_t id) const {
			return fileDataDB->findByModelResId(id);
		}

		inline GameFileUri::id_t findTextureFileId(uint32_t id) const {
			return fileDataDB->findByMaterialResId(id);
		}
	};


};