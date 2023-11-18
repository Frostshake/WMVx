#pragma once
#include <array>
#include <set>
#include "../filesystem/GameFileSystem.h"
#include "../database/GameDataset.h"
#include "../database/GameDatabase.h"
#include "../game/GameConstants.h"


namespace core {
	struct TextureGroup {
		static const size_t maxTextureCount = 3;
		std::array<GameFileUri, 3> texture;
		int32_t textureCount;
		int32_t base;

		const bool operator<(const TextureGroup& other) const
		{
			for (size_t i = 0; i < maxTextureCount; i++) {
				if (texture[i] < other.texture[i]) return true;
				if (texture[i] > other.texture[i]) return false;
			}
			return false;
		}
	};

	struct TextureSet {
		std::set<TextureGroup> groups;

		void load(GameFileUri modelFileUri, GameDatabase* gameDB) {

			std::function<bool(const CreatureModelDataRecordAdaptor*)> model_compare = [&modelFileUri](const CreatureModelDataRecordAdaptor* adaptor) -> bool {
				return adaptor->getModelUri().getId() == modelFileUri.getId();
			};

			if (modelFileUri.isPath()) {
				const GameFileUri::path_t matchName = GameFileUri::removeExtension(modelFileUri.getPath());
				model_compare = [=](const CreatureModelDataRecordAdaptor* adaptor) -> bool {
					assert(adaptor->getModelUri().isPath());
					GameFileUri::path_t testName = GameFileUri::removeExtension(adaptor->getModelUri().getPath());
					return matchName.compare(testName, Qt::CaseInsensitive) == 0;
				};
			}

			auto modelData = gameDB->creatureModelDataDB->find(model_compare);

			if (modelData != nullptr) {
				//found matching model 
				//now look for skins

				auto display_infos = gameDB->creatureDisplayDB->where([&modelData](const CreatureDisplayRecordAdaptor* adaptor) -> bool {
					return adaptor->getModelId() == modelData->getId();
					});

				for (auto& displayInfo : display_infos) {
					TextureGroup texture_group;
					texture_group.textureCount = 0;
					texture_group.base = (int32_t)TextureType::GAMEOBJECT1;
					//TODO check base is correct

					auto textureUris = displayInfo->getTextures();

					for (auto i = 0; i < textureUris.size(); i++) {
						auto& uri = textureUris[i];
						if (!uri.isEmpty()) {
							texture_group.texture[i] = uri;
							texture_group.textureCount++;
						}
					}


					if (texture_group.textureCount > 0) {
						groups.insert(std::move(texture_group));
					}
				}
			}
		}
	};


}

