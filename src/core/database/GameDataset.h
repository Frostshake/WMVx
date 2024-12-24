#pragma once
#include <vector>
#include <optional>
#include "GameDatasetAdaptors.h"

namespace core {

	template<typename T>
	class GameDataset {
	public:
		// Adaptor base class, not to be changed.
		using BaseAdaptor = T;
		// Adaptor implementing class, can be changed by inheriting classes.
		using Adaptor = T;

		GameDataset() = default;
		GameDataset(GameDataset&&) = default;
		virtual ~GameDataset() {}

		virtual const std::vector<BaseAdaptor*>& all() const = 0;

		template<typename P>
		inline const BaseAdaptor* find(P pred) const {
			const auto& records = this->all();
			auto result = std::find_if(records.begin(), records.end(), pred);
			return result == records.end() ? nullptr : *result;
		}

		template<typename P>
		inline const BaseAdaptor* findById(P id) const {
			return this->find([&id](const BaseAdaptor* adaptor) -> bool {
				return adaptor->getId() == id;
			});
		}

		template<typename P>
		inline const std::vector<BaseAdaptor*> where(P pred) const {
			const auto& records = this->all();
			std::vector<BaseAdaptor*> out;
			std::copy_if(records.begin(), records.end(), std::back_inserter(out), pred);
			return out;
		}

		template<typename P>
		inline size_t count(P pred) const {
			const auto& records = this->all();
			auto count = std::count_if(records.begin(), records.end(), pred);
			static_assert(sizeof(count) == sizeof(size_t));
			return count;
		}

	};

	class DatasetAnimationData : public GameDataset<AnimationDataRecordAdaptor> {
	};

	class DatasetCharacterRaces : public GameDataset<CharacterRaceRecordAdaptor> {
	};

	class DatasetCharacterFacialHairStyles : public GameDataset<CharacterFacialHairStyleRecordAdaptor> {

	};

	class DatasetCharacterHairGeosets : public GameDataset<CharacterHairGeosetRecordAdaptor> {

	};

	class DatasetCharacterSections : public GameDataset<CharacterSectionRecordAdaptor> {

	};

	class DatasetCharacterComponentTextures : public GameDataset<CharacterComponentTextureAdaptor> {
		// join of CharComponentTextureLayouts and CharComponentTextureSections
	};

	class DatasetCreatureModelData : public GameDataset<CreatureModelDataRecordAdaptor> {

	};

	class DatasetCreatureDisplay : public GameDataset<CreatureDisplayRecordAdaptor> {

	};

	class DatasetItems : public GameDataset<ItemRecordAdaptor> {

	};

	class DatasetItemDisplay : public GameDataset<ItemDisplayRecordAdaptor> {
		// join of items and itemssparse / cache
	};

	class DatasetItemVisual : public GameDataset<ItemVisualRecordAdaptor> {

	};

	class DatasetItemVisualEffect : public GameDataset<ItemVisualEffectRecordAdaptor> {

	};

	class DatasetSpellItemEnchantment : public GameDataset<SpellItemEnchantmentRecordAdaptor> {

	};


	/* --- */

	class DatasetNPCs : public GameDataset<NPCRecordAdaptor> {
	
	};
}