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

		inline SceneIO& setModelFactory(ModelFactory& mf) {
			modelFactory = mf;
			return *this;
		}

		inline SceneIO& setScene(Scene* s) {
			scene = s;
			return *this;
		}

		void load(QString path);

		void save(QString path);

		const QString FORMAT_VERSION = "1.1";

	protected:

		inline QString profileVersionString() const;

		QJsonObject modelToJson(const Model* model);
		QJsonObject vector3ToJson(const Vector3& vec);
		Vector3 vector3FromJson(const QJsonObject&);

		void importModel(QJsonObject model);

		GameClientInfo clientInfo;
		GameDatabase* gameDB;
		GameFileSystem* gameFS;
		ModelFactory modelFactory;
		Scene* scene;
	};

};