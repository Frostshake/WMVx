#pragma once

#include "GameDatasetAdaptors.h"
#include "DBCFile.h"
#include "DBCBackedDataset.h"

namespace core {

	template<typename T>
	class GenericDBCAnimationDataRecordAdaptor : public AnimationDataRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual QString getName() const {
			return this->dbc->getString(this->handle->name);
		}
	};

	template<typename T>
	class GenericDBCCharacterRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr uint32_t getId() const override {
			return this->handle->id;
		}

		QString getClientPrefix() const override {
			return this->dbc->getString(this->handle->clientPrefix);
		}

		QString getClientFileString() const override {
			return this->dbc->getString(this->handle->clientFileString);
		}

		std::optional<uint32_t> getComponentTextureLayoutId() const override {
			return std::nullopt;
		}
	};

	template<typename T>
	class GenericDBCCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getRaceId() const {
			return this->handle->raceId;
		}

		constexpr virtual Gender getSexId() const {
			return static_cast<Gender>(this->handle->sexId);
		}

		constexpr virtual uint32_t getGeoset100() const {
			return this->handle->geoset1;
		}

		constexpr virtual uint32_t getGeoset200() const {
			return this->handle->geoset2;
		}

		constexpr virtual uint32_t getGeoset300() const {
			return this->handle->geoset3;
		}
	};

	template<typename T>
	class GenericDBCCharacterHairGeosetsRecordAdaptor : public CharacterHairGeosetRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getRaceId() const {
			return this->handle->raceId;
		}

		constexpr virtual Gender getSexId() const {
			return static_cast<Gender>(this->handle->sexId);
		}

		constexpr virtual uint32_t getGeoset() const {
			return this->handle->geoset;
		}

		constexpr virtual bool isBald() const {
			return this->handle->showScalp;
		}
	};

	template<typename T>
	class GenericDBCCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual GameFileUri getModelUri() const {
			return this->dbc->getString(this->handle->modelName);
		}
	};

	template<typename T>
	class GenericDBCItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot) const {

			QString prefix = getPathPrefix(char_slot, item_slot);

			std::array<GameFileUri, 2> models = { 
				GameFileUri(""),
				GameFileUri("") 
			};

			QString left = this->dbc->getString(this->handle->modelLeft);
			QString right = this->dbc->getString(this->handle->modelRight);

			if (!left.isEmpty()) {
				models[0] = prefix + left;
			}

			if (!right.isEmpty()) {
				models[1] = prefix + right;
			}

			return models;
		}

		constexpr virtual uint32_t getGeosetGlovesFlags() const {
			return this->handle->geosetGlovesFlags;
		}

		constexpr virtual uint32_t getGeosetBracerFlags() const {
			return this->handle->geosetBracerFlags;
		}

		constexpr virtual uint32_t getGeosetRobeFlags() const {
			return this->handle->geosetRobeFlags;
		}

		virtual std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot) const {

			QString prefix = getPathPrefix(char_slot, item_slot);

			std::array<GameFileUri, 2> textures = {
				GameFileUri(""),
				GameFileUri("")
			};

			QString left = this->dbc->getString(this->handle->modelLeftTexture);
			QString right = this->dbc->getString(this->handle->modelRightTexture);


			if (!left.isEmpty()) {
				textures[0] = prefix + left + ".blp";
			}

			if (!right.isEmpty()) {
				textures[1] = prefix + right + ".blp";
			}

			return textures;
		}

		virtual GameFileUri getTextureUpperArm() const {
			return this->dbc->getString(this->handle->textureUpperArm);
		}

		virtual GameFileUri getTextureLowerArm() const {
			return this->dbc->getString(this->handle->textureLowerArm);
		}

		virtual GameFileUri getTextureHands() const {
			return this->dbc->getString(this->handle->textureHands);
		}

		virtual GameFileUri getTextureUpperChest() const {
			return this->dbc->getString(this->handle->textureUpperChest);
		}

		virtual GameFileUri getTextureLowerChest() const {
			return this->dbc->getString(this->handle->textureLowerChest);
		}

		virtual GameFileUri getTextureUpperLeg() const {
			return this->dbc->getString(this->handle->textureUpperLeg);
		}

		virtual GameFileUri getTextureLowerLeg() const {
			return this->dbc->getString(this->handle->textureLowerLeg);
		}

		virtual GameFileUri getTextureFoot() const {
			return this->dbc->getString(this->handle->textureFoot);
		}

		constexpr virtual uint32_t getItemVisualId() const {
			return this->handle->itemVisualId;
		}

	protected:

		QString getPathPrefix(CharacterSlot char_slot, ItemInventorySlotId item_slot) const {
			QString prefix = "";

			switch (char_slot) {
			case CharacterSlot::HEAD:
				prefix = "Item\\ObjectComponents\\Head\\";
				break;
			case CharacterSlot::SHOULDER:
				prefix = "Item\\ObjectComponents\\Shoulder\\";
				break;
			case CharacterSlot::HAND_LEFT:
				prefix = "Item\\ObjectComponents\\Weapon\\";
				break;
			case CharacterSlot::HAND_RIGHT:
				prefix = "Item\\ObjectComponents\\Weapon\\";
				break;
			case CharacterSlot::QUIVER:
				prefix = "Item\\ObjectComponents\\Quiver\\";
				break;
			}

			if (char_slot == CharacterSlot::HAND_LEFT || char_slot == CharacterSlot::HAND_RIGHT) {
				if (item_slot == ItemInventorySlotId::SHIELD) {
					prefix = "Item\\ObjectComponents\\Shield\\";
				}
			}

			return prefix;
		}
	};


	template<typename T>
	class GenericDBCItemVisualRecordAdaptor : public ItemVisualRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual std::array<uint32_t, 5> getItemVisualEffectIds() const {
			return std::to_array(this->handle->visualEffectId);
		}
	};

	template<typename T>
	class GenericDBCItemVisualEffectRecordAdaptor : public ItemVisualEffectRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual QString getModel() const {
			return this->dbc->getString(this->handle->model);
		}
	};

	template<typename T>
	class GenericDBCSpellItemEnchantmentRecordAdaptor : public SpellItemEnchantmentRecordAdaptor, public DBCBackedAdaptor<T> {
	public:
		using DBCBackedAdaptor<T>::DBCBackedAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->handle->id;
		}

		virtual QString getName() const {
			return this->dbc->getString(this->handle->name.enUS);
		}

		constexpr virtual uint32_t getItemVisualId() const {
			return this->handle->itemVisualId;
		}
	};
}