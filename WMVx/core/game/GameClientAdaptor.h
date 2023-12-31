#pragma once

#include <memory>
#include "../modeling/RawModel.h"
#include "../modeling/TabardCustomization.h"
#include "../modeling/CharacterCustomization.h"
#include "GameClientInfo.h"

namespace core {

	class GameFileSystem;
	class GameDatabase;

	using ModelFactory = std::function<std::unique_ptr<RawModel>()>;
	using TabardCustomizationProviderFactory = std::function<std::unique_ptr<TabardCustomizationProvider>(GameFileSystem*)>;
	using CharacterCustomizationProviderFactory = std::function<std::unique_ptr<CharacterCustomizationProvider>(GameFileSystem*, GameDatabase*)>;

	struct ModelSupport {
		ModelSupport(ModelFactory&& model_factory,
			TabardCustomizationProviderFactory&& tabard_factory,
			CharacterCustomizationProviderFactory&& char_factory)
			: modelFactory(model_factory), 
			tabardCustomizationProviderFactory(tabard_factory),
			characterCustomizationProviderFactory(char_factory)
		{}

		//TODO should these be const?
		ModelFactory modelFactory;
		TabardCustomizationProviderFactory tabardCustomizationProviderFactory;
		CharacterCustomizationProviderFactory characterCustomizationProviderFactory;
	};

	static const ModelSupport NullModelSupport = ModelSupport(
		[]() {
			return nullptr;
		},[](GameFileSystem* fs) {
			return nullptr;
		}, [](GameFileSystem* fs, GameDatabase* db) {
			return nullptr;
		}
	);

	class GameClientAdaptor {
	public:
		GameClientAdaptor() = default;
		GameClientAdaptor(GameClientAdaptor&&) = default;
		virtual ~GameClientAdaptor() {}

		virtual std::unique_ptr<GameFileSystem> filesystem(const GameClientInfo::Environment& environment) = 0;
		virtual std::unique_ptr<GameDatabase> database() = 0;
		virtual const ModelSupport modelSupport() = 0;
	};

	std::unique_ptr<GameClientAdaptor> makeGameClientAdaptor(const GameClientInfo::Profile& profile);

	class VanillaGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const GameClientInfo::Environment& environment) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;

		static const GameClientInfo::Profile PROFILE;
	};

	class WOTLKGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const GameClientInfo::Environment& environment) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;

		static const GameClientInfo::Profile PROFILE;
	};

	class BFAGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const GameClientInfo::Environment& environment) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;

		static const GameClientInfo::Profile PROFILE;
	};

	class DFGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const GameClientInfo::Environment& environment) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;

		static const GameClientInfo::Profile PROFILE;
	};
};