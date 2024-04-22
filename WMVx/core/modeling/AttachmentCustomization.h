#pragma once
#include "Attachment.h"
#include <vector>

namespace core {

	class GameFileSystem;
	class GameDatabase;
	class Scene;
	class CharacterItemWrapper;

	class AttachmentCustomizationProvider {
	public:
		AttachmentCustomizationProvider() = default;
		AttachmentCustomizationProvider(AttachmentCustomizationProvider&&) = default;
		virtual ~AttachmentCustomizationProvider() {}

		virtual std::vector<AttachmentPosition> getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item, bool sheatheWeapons) const = 0;

		virtual std::unique_ptr<Attachment> makeAttachment(
			CharacterSlot slot, 
			AttachmentPosition position, 
			const CharacterItemWrapper& wrapper,
			const GameFileUri& model_file,
			const GameFileUri& texture_file,
			Model* parent,
			Scene* scene
		) const = 0;
	};

	class StandardAttachmentCustomizationProvider : public AttachmentCustomizationProvider {
	public:

		StandardAttachmentCustomizationProvider(GameFileSystem* fs, GameDatabase* db, RawModel::Factory factory)
			: AttachmentCustomizationProvider(),
			gameFS(fs), gameDB(db), modelFactory(factory) {}
		StandardAttachmentCustomizationProvider(StandardAttachmentCustomizationProvider&&) = default;
		virtual ~StandardAttachmentCustomizationProvider() {}

		virtual std::vector<AttachmentPosition> getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item, bool sheatheWeapons) const;

		virtual std::unique_ptr<Attachment> makeAttachment(
			CharacterSlot slot, 
			AttachmentPosition position, 
			const CharacterItemWrapper& wrapper,
			const GameFileUri& model_file,
			const GameFileUri& texture_file,
			Model* parent,
			Scene* scene
		) const override;

	protected:
		GameFileSystem* gameFS;
		GameDatabase* gameDB;
		RawModel::Factory modelFactory;
	};

	class MergedAwareAttachmentCustomizationProvider : public StandardAttachmentCustomizationProvider {
	public:

		MergedAwareAttachmentCustomizationProvider(GameFileSystem* fs, GameDatabase* db, RawModel::Factory factory)
			: StandardAttachmentCustomizationProvider(fs, db, factory) {}
		MergedAwareAttachmentCustomizationProvider(MergedAwareAttachmentCustomizationProvider&&) = default;
		virtual ~MergedAwareAttachmentCustomizationProvider() {}

		virtual std::vector<AttachmentPosition> getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item, bool sheatheWeapons) const;

		virtual std::unique_ptr<Attachment> makeAttachment(
			CharacterSlot slot,
			AttachmentPosition position,
			const CharacterItemWrapper& wrapper,
			const GameFileUri& model_file,
			const GameFileUri& texture_file,
			Model* parent,
			Scene* scene
		) const override;

	protected:

		bool isMergedType(CharacterSlot slot) const;

	};
}