#include "../../stdafx.h"
#include "Scene.h"

namespace core {

	Scene::Scene(QObject* _parent) : QObject(_parent)
	{
		showGrid = false;
		active_selection = { nullptr, nullptr };
	}

	Scene::~Scene() {
		// models nedestroyed.
		// otherwise texture deleters will be referencing a non-existing managered to be specifically released before the texturemanager is .
		models.clear();
	}

	Model* Scene::addModel(std::unique_ptr<Model> m) {
		models.push_back(std::move(m));

		auto* last = models.back().get();
		emit componentAdded(last);

		return last;
	}

	ComponentMeta* Scene::addComponent(ComponentMeta* meta)
	{
		emit componentAdded(meta);
		return meta;
	}

	void Scene::removeComponent(ComponentMeta* meta) {
		if (meta->getMetaType() == ComponentMeta::Type::ROOT) {
			auto model = std::find_if(models.begin(), models.end(), [meta](const auto& m) {
				return dynamic_cast<ComponentMeta*>(m.get()) == meta;
			});

			const bool found = model != models.end();
			assert(found);

			emit(componentRemoved(meta));

			if (found) {
				if (model->get() == active_selection.root) {
					setSelectedModel(nullptr, nullptr);
				}

				models.erase(model);
			}

			if (models.size() == 0) {
				assert(textureManager.textures().size() == 0);
			}
		}
		else {
			componentRemoved(meta);
		}
	}

	const Scene::Selection& Scene::selected() const {
		return active_selection;
	}

	void Scene::setSelectedModel(Model* model, ComponentMeta* component) {
#ifdef _DEBUG
		if (model != nullptr) {
			auto found = std::find_if(models.begin(), models.end(), [&model](const auto& m) {
				return m.get() == model;
			});

			assert(found != models.end());
		}
#endif

		active_selection.root = model;
		active_selection.component = component;
		modelSelectionChanged(active_selection);
	}

	void Scene::componentUpdated(ComponentMeta* meta) {
		emit sceneChanged();
		//TODO sceneChanged needs to be emitted from elsewhere too.
	}

	bool _contains_meta_child(
		ComponentMeta* search,
		const std::vector<ComponentMeta*>& children) {
		for (ComponentMeta* child : children) {
			if (search == child) {
				return true;
			}
			else if (_contains_meta_child(search, child->getMetaChildren())) {
				return true;
			}
		}

		return false;
	};

	Model* Scene::findComponentRoot(ComponentMeta* meta) const {

		for (const auto& model : models) {
			if (meta == dynamic_cast<ComponentMeta*>(model.get())) {
				return model.get();
			}
			else if (_contains_meta_child(meta, model->getMetaChildren())) {
				return model.get();
			}
		}

		return nullptr;
	}

};