#pragma once

#include <memory>
#include "../modeling/RawModel.h"
#include "../modeling/TabardCustomization.h"

namespace core {

	class GameClientInfo;
	class GameFileSystem;
	class GameDatabase;

	using ModelFactory = std::function<std::unique_ptr<RawModel>()>;
	using TabardCustomizationProviderFactory = std::function<std::unique_ptr<TabardCustomizationProvider>(GameFileSystem*)>;

	struct ModelSupport {
		ModelSupport(ModelFactory&& model_factory,
			TabardCustomizationProviderFactory&& tabard_factory)
			: modelFactory(model_factory), tabardCustomizationProviderFactory(tabard_factory)
		{}

		//TODO should these be const?
		ModelFactory modelFactory;
		TabardCustomizationProviderFactory tabardCustomizationProviderFactory;
	};

	static const ModelSupport NullModelSupport = ModelSupport(
		[]() {
			return nullptr;
		},[](GameFileSystem* fs) {
			return nullptr;
		}
	);

	class GameClientAdaptor {
	public:
		GameClientAdaptor() = default;
		GameClientAdaptor(GameClientAdaptor&&) = default;
		virtual ~GameClientAdaptor() {}

		virtual std::unique_ptr<GameFileSystem> filesystem(const QString& root) = 0;
		virtual std::unique_ptr<GameDatabase> database() = 0;
		virtual const ModelSupport modelSupport() = 0;
	};

	class VanillaGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const QString& root) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;
	};

	class WOTLKGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const QString& root) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;
	};

	class BFAGameClientAdaptor : public GameClientAdaptor {
	public:
		std::unique_ptr<GameFileSystem> filesystem(const QString& root) override;
		std::unique_ptr<GameDatabase> database() override;
		const ModelSupport modelSupport() override;
	};
};