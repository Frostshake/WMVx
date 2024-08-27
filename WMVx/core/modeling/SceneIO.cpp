#include "../../stdafx.h"
#include "SceneIO.h"
#include "../../Build.h"
#include "../utility/Logger.h"
#include "../utility/Exceptions.h"

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
		return QString::fromStdString(clientInfo.profile.targetVersion);
	}

	QJsonObject SceneIO::modelToJson(const Model* model)
	{
		QJsonObject obj;
		obj["file_name"] = model->model->getFileInfo().path;
		
		obj["meta"] = QJsonObject{
			{"name", model->getMetaName()}
		};

		obj["render_options"] = QJsonObject{
			{"wire_frame", model->renderOptions.showWireFrame},
			{"bounds", model->renderOptions.showBounds},
			{"bones", model->renderOptions.showBones},
			{"texture", model->renderOptions.showTexture},
			{"render", model->renderOptions.showRender},
			{"particles", model->renderOptions.showParticles},
			{"opacity", model->renderOptions.opacity}
		};

		obj["model_options"] = QJsonObject{
			{"position", toJson(model->modelOptions.position)},
			{"rotation", toJson(model->modelOptions.rotation)},
			{"scale", toJson(model->modelOptions.scale)}
		};

		if (model->model->getModelPathInfo().isCharacter()) {
			QJsonObject char_obj;

			char_obj["render_options"] = QJsonObject{
				{"sheathe_weapons", model->characterOptions.sheatheWeapons},
				{"show_underwear",  model->characterOptions.showUnderWear},
				{"show_feet", model->characterOptions.showFeet},
				{"show_hair", model->characterOptions.showHair},
				{"show_facial_hair", model->characterOptions.showFacialHair},
				{"ear_visibility", (int)model->characterOptions.earVisibilty},
				{"eye_glow", (int)model->characterOptions.eyeGlow}
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
				{"file_name", attach->getModel()->getFileInfo().path},
				{"attachment_position", (int32_t)attach->attachmentPosition},
				{"character_slot", (int32_t)attach->getSlot()},
			};

				//TODO attachment effects

				attachments.append(att);
			}
			char_obj["attachments"] = attachments;


			QJsonArray merged;
			for (const auto& merge : model->getMerged()) {
				if (merge->getType() != MergedModel::Type::CHAR_ATTACHMENT_ADDITION) {
					merged.append(QJsonObject{
						{"file_name", merge->model->getFileInfo().path},
						{"type", (int32_t)merge->getType()},
						{"id", (int32_t)merge->getId()}
						});
				}
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

		auto m = std::make_unique<Model>();
		m->initialise(fileName, modelFactory, gameFS, gameDB, scene->textureManager);

		std::optional<CharacterRelationSearchContext> textureSearchContext = std::nullopt;

		if (m->getCharacterDetails().has_value()) {
			const auto* race_adaptor = gameDB->characterRacesDB->findById(m->getCharacterDetails()->raceId);
			if (race_adaptor != nullptr) {
				textureSearchContext = race_adaptor->getTextureSearchContext(m->getCharacterDetails()->gender);
			}
		}

		const QJsonObject meta = model["meta"].toObject();
		m->setMetaName(meta["name"].toString());

		const QJsonObject render_opts = model["render_options"].toObject();

		m->renderOptions.showWireFrame = render_opts["wire_frame"].toBool();
		m->renderOptions.showBounds = render_opts["bounds"].toBool();
		m->renderOptions.showBones = render_opts["bones"].toBool();
		m->renderOptions.showTexture = render_opts["texture"].toBool();
		m->renderOptions.showRender = render_opts["render"].toBool();
		m->renderOptions.showParticles = render_opts["particles"].toBool();

		m->renderOptions.opacity = render_opts["opacity"].toDouble();

		const QJsonObject model_opts = model["model_options"].toObject();

		m->modelOptions.position = toVector3(model_opts["position"].toObject());
		m->modelOptions.rotation = toVector3(model_opts["rotation"].toObject());
		m->modelOptions.scale = toVector3(model_opts["scale"].toObject());

		if (model.contains("character")) {
			const QJsonObject char_obj= model["character"].toObject();
			const QJsonObject char_opts = char_obj["render_options"].toObject();

			m->characterOptions.sheatheWeapons = char_opts["sheathe_weapons"].toBool();
			m->characterOptions.showUnderWear = char_opts["show_underwear"].toBool();
			m->characterOptions.showFeet = char_opts["show_feet"].toBool();
			m->characterOptions.showHair = char_opts["show_hair"].toBool();
			m->characterOptions.showFacialHair = char_opts["show_facial_hair"].toBool();
			m->characterOptions.earVisibilty = static_cast<CharacterRenderOptions::EarVisibility>(char_opts["ear_visibility"].toInt());
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

				auto attachmentProvider = attachmentFactory(gameFS, gameDB);
				std::map<CharacterSlot, int32_t> slot_indexes;

				const QJsonArray attachments = char_obj["attachments"].toArray();
				for (const auto& attach : attachments) {
					const QJsonObject attach_obj = attach.toObject();
					const auto att_file_name = attach_obj["file_name"].toString();
					const auto att_slot = static_cast<CharacterSlot>(attach_obj["character_slot"].toInt());
					const auto att_position = static_cast<AttachmentPosition>(attach_obj["attachment_position"].toInt());
					const auto& equipment = m->characterEquipment.at(att_slot);

					auto attachment_index = 0;
					if (slot_indexes.contains(att_slot)) {
						attachment_index = slot_indexes[att_slot] + 1;
					}

					GameFileUri att_texture_name = equipment.display()->getModelTexture(att_slot, equipment.item()->getInventorySlotId(), textureSearchContext)[attachment_index];

					std::unique_ptr<Attachment> att = attachmentProvider->makeAttachment(
						att_slot, 
						att_position, 
						equipment, 
						att_file_name, 
						att_texture_name, 
						m.get(), 
						scene
					);

					{
						auto* tmp = att.get();
						m->addAttachment(std::move(att));
						scene->addComponent(tmp);
					}
				}
			}

			if (char_obj.contains("merged")) {
				const QJsonArray merged = char_obj["merged"].toArray();
				for (const auto& merge : merged) {
					const QJsonObject merge_obj = merge.toObject();
					const auto merge_file_name = merge_obj["file_name"].toString();

					auto merge_item = std::make_unique<MergedModel>(
						m.get(),
						static_cast<MergedModel::Type>(merge_obj["type"].toInt()),
						merge_obj["id"].toInt()
					);

					merge_item->initialise(merge_file_name, modelFactory, gameFS, gameDB, scene->textureManager);
					merge_item->merge(MergedModel::RESOLUTION_FINE);

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

			m->characterInitialised = true;
		}

		scene->addModel(std::move(m));
	}

};