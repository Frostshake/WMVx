#include "../../stdafx.h"
#include "SceneIO.h"
#include "../../Build.h"
#include "../utility/Logger.h"

namespace core {

	void SceneIO::load(QString path)
	{
		QFile file(path);

		if (!file.open(QFile::ReadOnly | QFile::Text)) {
			throw FileIOException(path.toStdString(), "Could not read file.");
		}

		QByteArray bytes = file.readAll();

		QJsonParseError jsonError;
		QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonError);

		if (jsonError.error != QJsonParseError::NoError) {
			throw FileIOException(path.toStdString(), "JSON parse error.");
		}

		QJsonObject root = doc.object();
		QJsonObject meta = root.value("meta").toObject();

		if (meta["format"].toString() != SceneIO::FORMAT_VERSION ||
			meta["profile_version"].toString() != profileVersionString()) {
			Log::message("Incompatible scene file version.");
			return;
		}

		QJsonArray models = root.value("models").toArray();

		for (auto model : models) {
			importModel(model.toObject());
		}
	}


	void SceneIO::save(QString path)
	{
		QFile file(path);

		if (!file.open(QFile::WriteOnly | QFile::Text)) {
			throw FileIOException(path.toStdString(), "Could not write to file.");
		}

		QTextStream stream(&file);

		QJsonArray models;
		for (const auto& model : scene->models) {
			models.append(modelToJson(model.get()));
		}

		QJsonObject root;
		root["meta"] = QJsonObject{
			{"wmvx_version", WMVX_BUILD},
			{"format", SceneIO::FORMAT_VERSION},
			{"profile_version", profileVersionString()}
		};
		root["models"] = models;

		QJsonDocument doc(root);

		stream << doc.toJson();
	}

	inline QString SceneIO::profileVersionString() const 
	{
		return clientInfo.profile.targetVersion;
	}

	QJsonObject SceneIO::modelToJson(const Model* model)
	{
		QJsonObject render_opts;
		render_opts["wire_frame"] = model->renderOptions.showWireFrame;
		render_opts["bounds"] = model->renderOptions.showBounds;
		render_opts["bones"] = model->renderOptions.showBones;
		render_opts["texture"] = model->renderOptions.showTexture;
		render_opts["render"] = model->renderOptions.showRender;
		render_opts["particles"] = model->renderOptions.showParticles;

		render_opts["opacity"] = model->renderOptions.opacity;

		render_opts["position"] = vector3ToJson(model->renderOptions.position);
		render_opts["rotation"] = vector3ToJson(model->renderOptions.rotation);
		render_opts["scale"] = vector3ToJson(model->renderOptions.scale);

		QJsonObject obj;
		obj["file_name"] = model->model->getFileInfo().path;
		obj["render_options"] = render_opts;

		if (model->animate) {
			obj["animation"] = QJsonObject{
				{"index", QString::number(model->animator.getAnimationIndex().value())},
				{"paused", model->animator.isPaused()},
				{"speed", model->animator.getSpeed()},
				{"frame", QString::number(model->animator.getCurrentFrame())}
			};
		}
		else {
			obj["animation"] = QJsonValue(QJsonValue::Null);
		}

		return obj;
	}

	QJsonObject SceneIO::vector3ToJson(const Vector3& vec)
	{
		return QJsonObject{
			{"x", vec.x},
			{"y", vec.y},
			{"z", vec.z},
		};
	}

	Vector3 SceneIO::vector3FromJson(const QJsonObject& obj)
	{
		return Vector3(
			obj["x"].toDouble(),
			obj["y"].toDouble(),
			obj["z"].toDouble()
		);
	}

	void SceneIO::importModel(QJsonObject model)
	{
		auto fileName = model["file_name"].toString();
		if (fileName.isNull()) {
			return;
		}

		auto file = gameFS->openFile(fileName);
		if (file == nullptr) {
			return;
		}
		gameFS->closeFile(file);

		auto m = std::make_unique<Model>(Model(modelFactory));
		m->initialise(fileName, gameFS, gameDB, scene->textureManager);

		QJsonObject render_opts = model["render_options"].toObject();

		m->renderOptions.showWireFrame = render_opts["wire_frame"].toBool();
		m->renderOptions.showBounds = render_opts["bounds"].toBool();
		m->renderOptions.showBones = render_opts["bones"].toBool();
		m->renderOptions.showTexture = render_opts["texture"].toBool();
		m->renderOptions.showRender = render_opts["render"].toBool();
		m->renderOptions.showParticles = render_opts["particles"].toBool();

		m->renderOptions.opacity = render_opts["opacity"].toDouble();

		m->renderOptions.position = vector3FromJson(render_opts["position"].toObject());
		m->renderOptions.rotation = vector3FromJson(render_opts["rotation"].toObject());
		m->renderOptions.scale = vector3FromJson(render_opts["scale"].toObject());

		scene->addModel(std::move(m));
	}

};