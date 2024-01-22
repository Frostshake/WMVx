#include "../../stdafx.h"
#include "Scene.h"

namespace core {

	Scene::Scene(QObject* _parent) : QObject(_parent)
	{
		showGrid = false;
		active_model = nullptr;
		next_id = 1;
	}

	Scene::~Scene() {
		// models need to be specifically released before the texturemanager is destroyed.
		// otherwise texture deleters will be referencing a non-existing manager.
		models.clear();
	}

	Model* Scene::addModel(std::unique_ptr<Model> m) {
		assert(m->meta._id == 0);
		m->meta._id = next_id++;
		models.push_back(std::move(m));

		auto* last = models.back().get();
		emit modelAdded(last);

		return last;
	}

	void Scene::removeModel(ModelMeta::id_t id) {
		auto model = std::find_if(models.begin(), models.end(), [id](const auto& m) {
			return m->meta == id;
		});

		const bool found = model != models.end();
		assert(found);

		if (found) {
			if (model->get() == active_model) {
				setSelectedModel(nullptr);
			}

			emit(modelRemoved(model->get()));
			models.erase(model);
		}

		if (models.size() == 0) {
			assert(textureManager.textures().size() == 0);
		}
	}

	Model* Scene::selectedModel() const {
		return active_model;
	}

	void Scene::setSelectedModel(Model* model) {
#ifdef _DEBUG
		if (model != nullptr) {
			auto found = std::find_if(models.begin(), models.end(), [&model](const auto& m) {
				return m.get() == model;
			});

			assert(found != models.end());
		}
#endif

		active_model = model;
		modelSelectionChanged(active_model);
	}

};