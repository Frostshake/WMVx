#pragma once
#include <QString>
#include "../game/GameClientInfo.h"
#include "Scene.h"
#include "../database/GameDatabase.h"
#include "../filesystem/GameFileSystem.h"


namespace core {
	class SceneIO
	{
	public:
		SceneIO(GameClientInfo client_info) :
			clientInfo(client_info), gameDB(nullptr), gameFS(nullptr), scene(nullptr) {
		}
		virtual ~SceneIO() {};

		inline SceneIO& setDatabase(GameDatabase* db) {
			gameDB = db;
			return *this;
		}

		inline SceneIO& setFilesystem(GameFileSystem* fs) {
			gameFS = fs;
			return *this;
		}

		inline SceneIO& setModelFactory(M2Model::Factory& mf) {
			modelFactory = mf;
			return *this;
		}

		inline SceneIO& setScene(Scene* s) {
			scene = s;
			return *this;
		}

		inline SceneIO& setAttachmentProviderFactory(AttachmentCustomizationProviderFactory factory) {
			attachmentFactory = factory;
			return *this;
		}

		void load(QString path);

		void save(QString path);

		const QString FORMAT_VERSION = "1.4";

	protected:

		inline QString profileVersionString() const;

		QJsonObject modelToJson(const Model* model);

		QJsonObject toJson(const GameFileUri& uri) const;
		QJsonObject toJson(const Vector3& vec) const;
		GameFileUri toFileUri(const QJsonObject&) const;
		Vector3 toVector3(const QJsonObject&) const;

		void importModel(QJsonObject model);

		GameClientInfo clientInfo;
		GameDatabase* gameDB;
		GameFileSystem* gameFS;
		M2Model::Factory modelFactory;
		AttachmentCustomizationProviderFactory attachmentFactory;
		Scene* scene;
	};

};