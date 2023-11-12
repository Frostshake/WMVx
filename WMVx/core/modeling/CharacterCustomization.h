#pragma once
#include "ModelSupport.h"
#include "../database/BFARecordDefinitions.h"
#include "../database/DB2File.h"
#include <unordered_set>
#include <string>


namespace core {

	class GameFileSystem;
	class GameDatabase;
	class Model;

	//TODO tidy name
	using ChrCustomization = std::unordered_map<std::string, uint32_t>;

	struct CharacterDetails {
		uint32_t raceId = 0;
		core::Gender gender = core::Gender::MALE;
		bool isHd = false;
	};

	class CharacterCustomizationProvider {
	public:
		CharacterCustomizationProvider() = default;
		CharacterCustomizationProvider(CharacterCustomizationProvider&&) = default;
		virtual ~CharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails&) = 0;
		virtual void reset() = 0;

		virtual bool apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices) = 0;

		virtual const ChrCustomization& getAvailableOptions() = 0;
	};


	/// <summary>
	/// Character customization support for pre-shadowlands
	/// </summary>
	class LegacyCharacterCustomizationProvider : public CharacterCustomizationProvider {
		//TODO simplify by making seperate providers for (1.12 & 3.3.5) and 8.3
	public:
		LegacyCharacterCustomizationProvider(GameFileSystem* fs, GameDatabase* db) 
			: CharacterCustomizationProvider(),
			gameFS(fs), gameDB(db) {
			// ...
		}
		LegacyCharacterCustomizationProvider(LegacyCharacterCustomizationProvider&&) = default;
		virtual ~LegacyCharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails& details); 
		virtual void reset();

		virtual bool apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices);

		virtual const ChrCustomization& getAvailableOptions() {
			return known_options;
		}

	protected:
		GameFileSystem* gameFS;
		GameDatabase* gameDB;

	private:
		ChrCustomization known_options;	//value is option total

	};

	/// <summary>
	/// Character customization for shadowlands +
	/// </summary>
	class ModernCharacterCustomizationProvider : public CharacterCustomizationProvider {
	public:
		ModernCharacterCustomizationProvider(GameFileSystem* fs) 
			: CharacterCustomizationProvider(),
			gameFS(fs) {
			// ...
		}
		ModernCharacterCustomizationProvider(ModernCharacterCustomizationProvider&&) = default;
		virtual ~ModernCharacterCustomizationProvider() {}

		virtual void initialise(const CharacterDetails& details);
		virtual void reset();

		virtual bool apply(Model* model, const CharacterDetails& details, const ChrCustomization& choices);

		virtual const ChrCustomization& getAvailableOptions() {
			return known_options;
		}

	protected:
		GameFileSystem* gameFS;

	private:

		uint32_t getModelIdForCharacter(const CharacterDetails& details);

		ChrCustomization known_options;	//value is option total

	};
}