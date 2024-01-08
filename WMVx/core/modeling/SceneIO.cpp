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
		QJsonObject obj;
		obj["file_name"] = model->model->getFileInfo().path;
		obj["render_options"] = QJsonObject{
			{"wire_frame", model->renderOptions.showWireFrame},
			{"bounds", model->renderOptions.showBounds},
			{"bones", model->renderOptions.showBones},
			{"texture", model->renderOptions.showTexture},
			{"render", model->renderOptions.showRender},
			{"particles", model->renderOptions.showParticles},
			{"opacity", model->renderOptions.opacity},
			{"position", toJson(model->renderOptions.position)},
			{"rotation", toJson(model->renderOptions.rotation)},
			{"scale", toJson(model->renderOptions.scale)},
		};

		if (model->model->isCharacter()) {
			QJsonObject char_obj;

			char_obj["render_options"] = QJsonObject{
				{"sheathe_weapons", model->characterOptions.sheatheWeapons},
				{"show_underwear",  model->characterOptions.showUnderWear},
				{"show_ears", model->characterOptions.showEars},
				{"show_feet", model->characterOptions.showFeet},
				{"show_hair", model->characterOptions.showHair},
				{"show_facial_hair", model->characterOptions.showFacialHair},
				{"eye_glow", model->characterOptions.eyeGlow}
			};

			QJsonArray char_equip;
			for (const auto& equip : model->characterEquipment) {
				char_equip.push_back(QJsonObject{
					{"slot", (int32_t)equip.first},
					{"item_id", (int32_t)equip.second.item->getId()},
					{"item_dispay_id", (int32_t)equip.second.display->getId()}
				});
			}
			char_obj["equipment"] = char_equip;

			if (model->tabardCustomization.has_value()) {

				QJsonArray upper_list;
				for (const auto& upper : model->tabardCustomization->texturesUpperChest) {
					upper_list.push_back(toJson(upper));
				}

				QJsonArray lower_list;
				for (const auto& lower : model->tabardCustomization->texturesLowerChest) {
					upper_list.push_back(toJson(lower));
				}

				char_obj["custom_tabard"] = QJsonObject{
					{"textures_upper_chest", std::move(upper_list)},
					{"textures_lower_chest", std::move(lower_list)}
				};
			}

			QJsonObject custom_obj;
			for (const auto& choice : model->characterCustomizationChoices) {
				custom_obj.insert(QString::fromStdString(choice.first), (int32_t)choice.second);
			}
			char_obj["customizations"] = custom_obj;

			//TODO attachments

			//TODO merged models

			obj["character"] = char_obj;
		}
		

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

	QJsonObject SceneIO::toJson(const GameFileUri& uri) const {
		if (uri.isId()) {
			return QJsonObject{ {"id", (int32_t)uri.getId()} };
		}

		return QJsonObject{ {"path", uri.getPath()}};
	}

	QJsonObject SceneIO::toJson(const Vector3& vec) const
	{
		return QJsonObject{
			{"x", vec.x},
			{"y", vec.y},
			{"z", vec.z},
		};
	}

	GameFileUri SceneIO::toFileUri(const QJsonObject& obj) const
	{
		if (obj.contains("id")) {
			return (GameFileUri::id_t)obj["id"].toInt();
		}

		return obj["path"].toString();
	}

	Vector3 SceneIO::toVector3(const QJsonObject& obj) const
	{
		return Vector3(
			obj["x"].toDouble(),
			obj["y"].toDouble(),
			obj["z"].toDouble()
		);
	}

	void SceneIO::importModel(QJsonObject model)
	{
		const auto fileName = model["file_name"].toString();
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

		const QJsonObject render_opts = model["render_options"].toObject();

		m->renderOptions.showWireFrame = render_opts["wire_frame"].toBool();
		m->renderOptions.showBounds = render_opts["bounds"].toBool();
		m->renderOptions.showBones = render_opts["bones"].toBool();
		m->renderOptions.showTexture = render_opts["texture"].toBool();
		m->renderOptions.showRender = render_opts["render"].toBool();
		m->renderOptions.showParticles = render_opts["particles"].toBool();

		m->renderOptions.opacity = render_opts["opacity"].toDouble();

		m->renderOptions.position = toVector3(render_opts["position"].toObject());
		m->renderOptions.rotation = toVector3(render_opts["rotation"].toObject());
		m->renderOptions.scale = toVector3(render_opts["scale"].toObject());

		if (model.contains("character")) {
			const QJsonObject char_obj= model["character"].toObject();
			const QJsonObject char_opts = char_obj["render_options"].toObject();

			m->characterOptions.sheatheWeapons = char_opts["sheathe_weapons"].toBool();
			m->characterOptions.showUnderWear = char_opts["show_underwear"].toBool();
			m->characterOptions.showEars = char_opts["show_ears"].toBool();
			m->characterOptions.showFeet = char_opts["show_feet"].toBool();
			m->characterOptions.showHair = char_opts["show_hair"].toBool();
			m->characterOptions.showFacialHair = char_opts["show_facial_hair"].toBool();
			m->characterOptions.eyeGlow = static_cast<CharacterRenderOptions::EyeGlow>(char_opts["eye_glow"].toInt());

			const QJsonArray char_equip = char_obj["equipment"].toArray();
			for (const auto& equip : char_equip) {
				const auto equip_obj = equip.toObject();
				CharacterSlot slot = static_cast<CharacterSlot>(equip_obj["slot"].toInt());
				uint32_t item_id = equip_obj["item_id"].toInt();
				uint32_t item_display_id = equip_obj["item_display_id"].toInt();

				if (item_id > 0) {
					const auto* item_record = gameDB->itemsDB->findById(item_id);
					if (item_record != nullptr) {
						m->characterEquipment.insert_or_assign(slot, CharacterItemWrapper::make(item_record, gameDB));
					}
				}
				else {
					const auto* display = gameDB->itemDisplayDB->findById(item_display_id);
					if (display != nullptr) {
						m->characterEquipment.insert_or_assign(
							slot, 
							CharacterItemWrapper::make(Mapping::CharacterSlotItemInventory.at(slot)[0], display)
						);
					}
				}
			}

			if (char_obj.contains("custom_tabard")) {
				TabardCustomization tabard;
				QJsonObject char_tab = char_obj["custom_tabard"].toObject();
				QJsonArray upper = char_tab["textures_upper_chest"].toArray();
				QJsonArray lower = char_tab["textures_lower_chest"].toArray();

				const auto insert = [this](auto& src, auto& dest) {
					const auto min = std::min((size_t)src.size(), dest.size());
					for (auto i = 0; i < min; i++) {
						dest[i] = toFileUri(src.at(i).toObject());
					}
				};

				insert(upper, tabard.texturesUpperChest);
				insert(lower, tabard.texturesLowerChest);

				m->tabardCustomization.emplace(tabard);
				
			}

			const QJsonObject char_custom = char_obj["customizations"].toObject();
			for (const auto& key : char_custom.keys()) {
				const auto choice = char_custom.value(key);
				m->characterCustomizationChoices.insert({ key.toStdString(), (uint32_t)choice.toInt()});
			}
		}

		scene->addModel(std::move(m));
	}

};