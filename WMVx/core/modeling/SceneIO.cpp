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
		
		obj["meta"] = QJsonObject{
			{"name", model->meta.getName()}
		};

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
					{"character_slot", (int32_t)equip.first},
					{"item_id", (int32_t)equip.second.item()->getId()},
					{"item_dispay_id", (int32_t)equip.second.display()->getId()}
				});
			}
			char_obj["equipment"] = char_equip;

			if (model->tabardCustomizationChoices.has_value()) {

				char_obj["custom_tabard"] = QJsonObject{
					{"icon", (int32_t)model->tabardCustomizationChoices->icon},
					{"icon_color", (int32_t)model->tabardCustomizationChoices->iconColor},
					{"border", (int32_t)model->tabardCustomizationChoices->border},
					{"boorder_color", (int32_t)model->tabardCustomizationChoices->borderColor},
					{"background", (int32_t)model->tabardCustomizationChoices->background},
				};
			}

			QJsonObject custom_obj;
			for (const auto& choice : model->characterCustomizationChoices) {
				custom_obj.insert(QString::fromStdString(choice.first), (int32_t)choice.second);
			}
			char_obj["customizations"] = custom_obj;

			QJsonArray attachments;
			for (const auto& attach : model->getAttachments()) {
				QJsonObject att{
					{"file_name", attach->model->getFileInfo().path},
					{"attachment_position", (int32_t)attach->attachmentPosition},
					{"character_slot", (int32_t)attach->characterSlot},
				};

				//TODO attachment effects

				attachments.append(att);
			}
			char_obj["attachments"] = attachments;


			QJsonArray merged;
			for (const auto& merge : model->getMerged()) {
				merged.append(QJsonObject{
					{"file_name", merge->model->getFileInfo().path},
					{"type", (int32_t)merge->getType()},
					{"id", (int32_t)merge->getId()}
				});
			}
			char_obj["merged"] = merged;

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

		std::optional<CharacterRelationSearchContext> textureSearchContext = std::nullopt;

		if (m->getCharacterDetails().has_value()) {
			const auto* race_adaptor = gameDB->characterRacesDB->findById(m->getCharacterDetails()->raceId);
			if (race_adaptor != nullptr) {
				textureSearchContext = race_adaptor->getTextureSearchContext(m->getCharacterDetails()->gender);
			}
		}

		const QJsonObject meta = model["meta"].toObject();
		m->meta.setName(meta["name"].toString());

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
				CharacterSlot slot = static_cast<CharacterSlot>(equip_obj["character_slot"].toInt());
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

			if (char_obj.contains("attachments")) {

				std::map<CharacterSlot, int32_t> slot_indexes;

				const QJsonArray attachments = char_obj["attachments"].toArray();
				for (const auto& attach : attachments) {
					const QJsonObject attach_obj = attach.toObject();
					const auto att_file_name = attach_obj["file_name"].toString();
					
					auto att = std::make_unique<Attachment>(modelFactory);
					att->attachmentPosition = static_cast<AttachmentPosition>(attach_obj["attachment_position"].toInt());
					att->characterSlot = static_cast<CharacterSlot>(attach_obj["character_slot"].toInt());

					auto attachment_index = 0;
					if (slot_indexes.contains(att->characterSlot)) {
						attachment_index = slot_indexes[att->characterSlot] + 1;
					}

					auto loadTexture = std::bind(&ModelTextureInfo::loadTexture,
						att.get(),
						std::placeholders::_1,
						std::placeholders::_2,
						std::placeholders::_3,
						std::placeholders::_4,
						std::ref(scene->textureManager),
						gameFS
					);
					att->model->load(gameFS, att_file_name, loadTexture);
					att->initAnimationData(att->model.get());

					const auto& equip = m->characterEquipment.at(att->characterSlot);
					//load attachment texture
					GameFileUri texture_file_name = equip.display()->getModelTexture(att->characterSlot, equip.item()->getInventorySlotId(), textureSearchContext)[attachment_index];
					Log::message("Loaded attachment texture: " + texture_file_name.toString());
					auto tex = scene->textureManager.add(texture_file_name, gameFS);
					if (tex != nullptr) {
						att->replacableTextures[TextureType::CAPE] = tex;
					}

					slot_indexes[att->characterSlot] = attachment_index;

					m->setAttachmentPosition(att.get(), att->attachmentPosition);

					m->addAttachment(std::move(att));
				}
			}

			if (char_obj.contains("merged")) {
				const QJsonArray merged = char_obj["merged"].toArray();
				for (const auto& merge : merged) {
					const QJsonObject merge_obj = merge.toObject();
					const auto merge_file_name = merge_obj["file_name"].toString();

					auto merge_item = std::make_unique<MergedModel>(
						modelFactory,
						m.get(),
						static_cast<MergedModel::Type>(merge_obj["type"].toInt()),
						merge_obj["id"].toInt()
					);

					merge_item->initialise(merge_file_name, gameFS, gameDB, scene->textureManager);
					merge_item->merge();

					m->addRelation(std::move(merge_item));
				}
			}

			if (char_obj.contains("custom_tabard")) {
				TabardCustomizationOptions tabard;
				QJsonObject char_tab = char_obj["custom_tabard"].toObject();
				tabard.icon = char_tab["icon"].toInt();
				tabard.iconColor = char_tab["icon_color"].toInt();
				tabard.border = char_tab["border"].toInt();
				tabard.borderColor = char_tab["border_color"].toInt();
				tabard.background = char_tab["background"].toInt();

				m->tabardCustomizationChoices.emplace(tabard);				
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