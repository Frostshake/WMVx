#pragma once

#include "GameDatasetAdaptors.h"
#include "GenericDBCDataset.h"
#include "../game/GameConstants.h"
#include <WDBReader/Database.hpp>

namespace core {

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCAnimationDataRecordAdaptor : public AnimationDataRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->_record.data.id;
		}

		virtual QString getName() const {
			return QString(this->_record.data.name.get());
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCChrRacesRecordAdaptor : public CharacterRaceRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		QString getClientPrefix() const override {
			return QString(this->_record.data.clientPrefix.get());
		}

		QString getClientFileString() const override {
			return QString(this->_record.data.clientFileString.get());
		}

		std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const override {
			return std::nullopt;
		}

		virtual std::optional<CharacterRelationSearchContext> getModelSearchContext(Gender gender) const override {
			return CharacterRelationSearchContext::makeLegacy(gender, this->_record.data.id, this->getClientPrefix());
		}

		virtual std::optional<CharacterRelationSearchContext> getTextureSearchContext(Gender gender) const override {
			return CharacterRelationSearchContext::makeLegacy(gender, this->_record.data.id, this->getClientPrefix());
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCCharacterHairGeosetsRecordAdaptor : public CharacterHairGeosetRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr virtual Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}

		constexpr virtual uint32_t getGeoset() const override {
			return this->_record.data.geosetId;
		}

		constexpr virtual bool isBald() const override {
			return this->_record.data.showscalp;
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCCreatureModelDataRecordAdaptor : public CreatureModelDataRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const {
			return this->_record.data.id;
		}

		virtual GameFileUri getModelUri() const {
			return QString(this->_record.data.modelName.get());
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCCharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}

		constexpr CharacterSectionType getType() const override {
			return static_cast<CharacterSectionType>(this->_record.data.baseSection);
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				QString(this->_record.data.textureName[0].get()),
				QString(this->_record.data.textureName[1].get()),
				QString(this->_record.data.textureName[2].get())
			};
		}

		constexpr uint32_t getSection() const override {
			return this->_record.data.variationIndex;
		}

		constexpr uint32_t getVariationIndex() const override {
			return this->_record.data.colorIndex;
		}

		constexpr bool isHD() const override {
			return false;
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCCreatureDisplayExtraRecordAdaptor : public CreatureDisplayExtraRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.displayRaceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.displaySexId);
		}

		constexpr uint32_t getSkinId() const override {
			return this->_record.data.skinId;
		}

		constexpr uint32_t getFaceId() const override {
			return  this->_record.data.faceId;
		}

		constexpr uint32_t getHairStyleId() const override {
			return this->_record.data.hairStyleId;
		}

		constexpr uint32_t getHairColorId() const override {
			return this->_record.data.hairColorId;
		}

		constexpr uint32_t getFacialHairId() const override {
			return this->_record.data.facialHairId;
		}

		std::map<ItemInventorySlotId, uint32_t> getItemDisplayIds() const override {
			constexpr std::array slot_map = {
				CharacterSlot::HEAD,
				CharacterSlot::SHOULDER,
				CharacterSlot::SHIRT,
				CharacterSlot::CHEST,
				CharacterSlot::BELT,
				CharacterSlot::PANTS,
				CharacterSlot::BOOTS,
				CharacterSlot::BRACERS,
				CharacterSlot::GLOVES,
				CharacterSlot::TABARD,
				CharacterSlot::CAPE
			};

			std::map<ItemInventorySlotId, uint32_t> result;
			constexpr auto size = std::extent<decltype(this->_record.data.NPCItemDisplayId)>::value;

			for (auto i = 0; i < size; i++) {
				const auto& item = this->_record.data.NPCItemDisplayId[i];
				if (item > 0) {
					const auto& options = Mapping::CharacterSlotItemInventory.at(slot_map[i]);
					ItemInventorySlotId inv_slot = options[0];

					//if (options.size() > 1) {
					//	//TODO handle multiple;
					//}

					result.insert({ inv_slot, item });
				}
			}

			return result;
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCCreatureDisplayRecordAdaptor : public CreatureDisplayRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		GenericLegacyDBCCreatureDisplayRecordAdaptor(T&& record,
			std::unique_ptr<CreatureDisplayExtraRecordAdaptor> extra) : 
			GenericDBCRecordAdaptor<T>(std::move(record)),
			_extra_adaptor(std::move(extra))
		{}
		GenericLegacyDBCCreatureDisplayRecordAdaptor(GenericLegacyDBCCreatureDisplayRecordAdaptor<T>&&) = default;
		virtual ~GenericLegacyDBCCreatureDisplayRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getModelId() const override {
			return this->_record.data.modelId;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				QString(this->_record.data.textureVariation[0].get()),
				QString(this->_record.data.textureVariation[1].get()),
				QString(this->_record.data.textureVariation[2].get()),
			};
		}

		const CreatureDisplayExtraRecordAdaptor* getExtra() const override {
			return _extra_adaptor.get();
		}

	protected:
		std::unique_ptr<CreatureDisplayExtraRecordAdaptor> _extra_adaptor;
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCItemDisplayInfoRecordAdaptor : public ItemDisplayRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const override {
			return this->_record.data.id;
		}

		virtual std::array<GameFileUri, 2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			QString prefix = getPathPrefix(char_slot, item_slot);

			std::array<GameFileUri, 2> models = {
				GameFileUri(""),
				GameFileUri("")
			};

			QString left = this->_record.data.modelName[0].get();
			QString right = this->_record.data.modelName[1].get();

			if (!left.isEmpty()) {
				models[0] = prefix + left;
			}

			if (!right.isEmpty()) {
				models[1] = prefix + right;
			}

			if (char_slot == CharacterSlot::HEAD && search.has_value()) {
				//head handling include race / gender data, seems to only be needed for path types.
				for (auto& model : models) {
					if (model.isPath() && model.getPath().length() > 0) {
						auto model_file_name = GameFileUri::removeExtension(model.getPath());
						model_file_name.append("_");
						model_file_name.append(search->raceClientPrefix);
						model_file_name.append(GenderUtil::toChar(static_cast<Gender>(search->gender)));
						model_file_name.append(".m2");
						model = model_file_name;
					}
				}
			}

			return models;
		}

		constexpr virtual uint32_t getGeosetGlovesFlags() const override {
			return this->_record.data.geosetGroup[0];
		}

		constexpr virtual uint32_t getGeosetBracerFlags() const override {
			return this->_record.data.geosetGroup[1];
		}

		constexpr virtual uint32_t getGeosetRobeFlags() const override {
			return this->_record.data.geosetGroup[2];
		}

		virtual std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const override {

			QString prefix = getPathPrefix(char_slot, item_slot);

			std::array<GameFileUri, 2> textures = {
				GameFileUri(""),
				GameFileUri("")
			};

			QString left = this->_record.data.modelTexture[0].get();
			QString right = this->_record.data.modelTexture[1].get();


			if (!left.isEmpty()) {
				textures[0] = prefix + left + ".blp";
			}

			if (!right.isEmpty()) {
				textures[1] = prefix + right + ".blp";
			}

			return textures;
		}

		virtual GameFileUri getTextureUpperArm() const override {
			return QString(this->_record.data.texture[0].get());
		}

		virtual GameFileUri getTextureLowerArm() const override {
			return QString(this->_record.data.texture[1].get());
		}

		virtual GameFileUri getTextureHands() const override {
			return QString(this->_record.data.texture[2].get());
		}

		virtual GameFileUri getTextureUpperChest() const override {
			return QString(this->_record.data.texture[3].get());
		}

		virtual GameFileUri getTextureLowerChest() const override {
			return QString(this->_record.data.texture[4].get());
		}

		virtual GameFileUri getTextureUpperLeg() const override {
			return QString(this->_record.data.texture[5].get());
		}

		virtual GameFileUri getTextureLowerLeg() const override {
			return QString(this->_record.data.texture[6].get());
		}

		virtual GameFileUri getTextureFoot() const override {
			return QString(this->_record.data.texture[7].get());
		}

		constexpr virtual uint32_t getItemVisualId() const override {
			return this->_record.data.itemVisualId;
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

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCItemVisualRecordAdaptor : public ItemVisualRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const override {
			return this->_record.data.id;
		}

		virtual std::array<uint32_t, 5> getItemVisualEffectIds() const override {
			return std::to_array(this->_record.data.slot);
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCItemVisualEffectRecordAdaptor : public ItemVisualEffectRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const override {
			return this->_record.data.id;
		}

		virtual QString getModel() const override {
			return QString(this->_record.data.model.get());
		}
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCSpellItemEnchantmentRecordAdaptor : public SpellItemEnchantmentRecordAdaptor, public GenericDBCRecordAdaptor<T> {
	public:
		using GenericDBCRecordAdaptor<T>::GenericDBCRecordAdaptor;

		constexpr virtual uint32_t getId() const override {
			return this->_record.data.id;
		}

		virtual QString getName() const override {
			return QString(this->_record.data.nameLang.strings[0].get());
		}

		constexpr virtual uint32_t getItemVisualId() const override {
			return this->_record.data.itemVisual;
		}
	};

}