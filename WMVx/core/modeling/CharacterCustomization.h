#pragma once
#include "ModelSupport.h"
#include "../database/DFDefinitions.h"
#include "../database/FileDataGameDatabase.h"
#include <WDBReader/Database.hpp>
#include <unordered_set>
#include <string>
#include <optional>

namespace core {

	class GameFileSystem;
	class GameDatabase;
	class Model;
	class Scene;

	using CharacterCustomizations = std::unordered_map<std::string, uint32_t>;						// option label -> choice index
	using CharacterCustomizationChoices = std::vector<std::string>;									// labels
	using CharacterCustomizationOptions = std::map<std::string, CharacterCustomizationChoices>;		// option label -> choices(labels)

	class CharacterEyeGlowCustomization {
	public:

		using handler_t = std::function<void(core::Model*)>;

		static void enumBasedHandler(core::Model* model);
		static void geosetBasedHandler(core::Model* model);
	};

	struct CharacterDetails {
		uint32_t raceId = 0;
		core::Gender gender = core::Gender::MALE;
		bool isHd = false;

		static bool detect(const Model* model, const GameDatabase* gameDB, CharacterDetails& out);
	};

	class CharacterCustomizationProvider {
	public:
		CharacterCustomizationProvider() = default;
		CharacterCustomizationProvider(CharacterCustomizationProvider&&) = default;
		virtual ~CharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails&) = 0;
		virtual void reset() = 0;

		/*
			Apply the choices to the model, this is when the provider is also able to update its internal context used for updating model.
			'apply' and 'update' are two seperate steps as there are cases where the model needs to be refreshed, but the choices/context wont have changed.
		*/
		bool apply(Model* model, const CharacterDetails& details, const CharacterCustomizations& choices);

		/*
			use the internal state/context to update the model.
		*/
		virtual bool update(Model* model, CharacterTextureBuilder* builder, Scene* scene) = 0;

		virtual const CharacterCustomizationOptions& getAvailableOptions() = 0;

		virtual CharacterComponentTextureAdaptor* getComponentTextureAdaptor(const CharacterDetails& details) = 0;

	protected:
		virtual bool updateContext(const CharacterDetails& details, const CharacterCustomizations& choices) = 0;
	};


	/// <summary>
	/// Character customization support for pre-shadowlands
	/// </summary>
	class LegacyCharacterCustomizationProvider : public CharacterCustomizationProvider {
	public:

		struct Context {
			CharacterSectionRecordAdaptor* skin;
			CharacterSectionRecordAdaptor* face;
			CharacterSectionRecordAdaptor* hairColour;
			CharacterHairGeosetRecordAdaptor* hairStyle;
			CharacterFacialHairStyleRecordAdaptor* facialStyle;
			CharacterSectionRecordAdaptor* facialColour;
			CharacterSectionRecordAdaptor* underwear;

			Context() {
				skin = nullptr;
				face = nullptr;
				hairColour = nullptr;
				hairStyle = nullptr;
				facialStyle = nullptr;
				facialColour = nullptr;
				underwear = nullptr;
			}

			bool isValid() const {
				return skin != nullptr &&
					face != nullptr &&
					hairColour != nullptr &&
					hairStyle != nullptr &&
					facialStyle != nullptr;
				//note facial colour intentionally missed out, it isnt always needed. same with underwear
			}
		};



		LegacyCharacterCustomizationProvider(GameFileSystem* fs, GameDatabase* db) 
			: CharacterCustomizationProvider(),
			gameFS(fs), gameDB(db) {
			eyeGlowHandler = CharacterEyeGlowCustomization::enumBasedHandler;
		}
		LegacyCharacterCustomizationProvider(LegacyCharacterCustomizationProvider&&) = default;
		virtual ~LegacyCharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails& details); 
		virtual void reset();
		
		virtual bool update(Model* model, CharacterTextureBuilder* builder, Scene* scene);


		virtual const CharacterCustomizationOptions& getAvailableOptions() {
			return known_options;
		}

		virtual CharacterComponentTextureAdaptor* getComponentTextureAdaptor(const CharacterDetails& details);

	protected:
		virtual bool updateContext(const CharacterDetails& details, const CharacterCustomizations& choices);

		GameFileSystem* gameFS;
		GameDatabase* gameDB;

		CharacterEyeGlowCustomization::handler_t eyeGlowHandler;

	private:
		CharacterCustomizationOptions known_options;

		std::unique_ptr<Context> context;

	};

	/// <summary>
	/// Character customization for shadowlands +
	/// </summary>
	class ModernCharacterCustomizationProvider : public CharacterCustomizationProvider {
	public:

		struct Context {

			struct Material {
				uint32_t custMaterialId;
				GameFileUri uri;
				int32_t textureType;	//TODO enum?
				int32_t layer;
				int32_t blendMode;
				int32_t region;	//TODO real type

				bool operator<(const Material& c) const
				{
					return layer < c.layer;
				}
			};

			struct Model {
				uint32_t custSkinnedModelId;
				GameFileUri uri;
				int32_t geosetType;
				int32_t geosetId;
			};

			struct Geoset {
				uint32_t geosetType; //TODO enum
				uint32_t geosetId;
			};



			std::vector<Geoset> geosets;
			std::vector<Model> models;
			std::vector<Material> materials;
		};

		ModernCharacterCustomizationProvider(GameFileSystem* fs, GameDatabase* db, const WDBReader::GameVersion& version);
		ModernCharacterCustomizationProvider(ModernCharacterCustomizationProvider&&) = default;
		virtual ~ModernCharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails& details);
		virtual void reset();

		virtual bool update(Model* model, CharacterTextureBuilder* builder, Scene* scene);

		virtual const CharacterCustomizationOptions& getAvailableOptions() {
			return known_options;
		}

		virtual CharacterComponentTextureAdaptor* getComponentTextureAdaptor(const CharacterDetails& details);

		void setCharacterEyeGlowHandler(CharacterEyeGlowCustomization::handler_t handler) {
			eyeGlowHandler = handler;
		}

	protected:
		virtual bool updateContext(const CharacterDetails& details, const CharacterCustomizations& choices);

		GameFileSystem* gameFS;
		GameDatabase* gameDB;
		IFileDataGameDatabase* fileDataDB;

		CharacterEyeGlowCustomization::handler_t eyeGlowHandler;

	private:

		using db_t = std::pair < WDBReader::Database::RuntimeSchema, std::unique_ptr<WDBReader::Database::DataSource<WDBReader::Database::RuntimeRecord>>>;

		std::unique_ptr<Context> context;

		db_t elementsDB;
		db_t geosetsDB;
		db_t skinnedModelsDB;
		db_t materialsDB;
		db_t textureLayersDB;
		db_t modelsDB;
		db_t raceModelsDB;

		WDBReader::Database::RuntimeSchema _schema_chr_custom;
		WDBReader::Database::RuntimeSchema _schema_chr_option;
		WDBReader::Database::RuntimeSchema _schema_chr_choice;

		uint32_t getModelIdForCharacter(const CharacterDetails& details);

		CharacterCustomizationOptions known_options;

		// name -> id format.
		std::unordered_map<std::string, uint32_t> cacheOptions;

		// option_id -> [choice_id...] format.
		std::unordered_map<uint32_t, std::vector<uint32_t>> cacheChoices;

		
		inline std::optional<WDBReader::Database::RuntimeRecord> findRecordById(db_t& source, uint32_t id) {
			for (auto& rec : *source.second) {
				if (rec.encryptionState == WDBReader::Database::RecordEncryption::ENCRYPTED) {
					continue;
				}

				auto [found_id] = source.first(rec).get<uint32_t>("ID");
				if(found_id == id) {
					return std::move(rec);
				}
			}

			return std::nullopt;
		}

		uint32_t getTextureLayoutId(const CharacterDetails& details);

	};
}