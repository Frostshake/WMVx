#pragma once
#include <QtCore>
#include <memory>
#include "Model.h"
#include "Texture.h"

namespace core {

	class Scene : public QObject
	{
		Q_OBJECT

	public:
		Scene(QObject* _parent) : QObject(_parent)
		{
			showGrid = false;
		}

		Scene(const Scene& instance) = delete;
		virtual ~Scene() {
			// models need to be specifically released before the texturemanager is destroyed.
			// otherwise texture deleters will be referencing a non-existing manager.
			models.clear();
		}

		void addModel(std::unique_ptr<Model> m) {
			models.push_back(std::move(m));
			emit modelAdded();
		}

		void removeModel(size_t model_index) {
			{
				auto model = std::move(models.at(model_index));
				models.erase(std::next(models.begin(), model_index));
			}

			if (models.size() == 0) {
				assert(textureManager.textures().size() == 0);
			}

			emit modelRemoved();
		}

		std::vector<std::unique_ptr<Model>> models;
		TextureManager textureManager;

		bool showGrid;

	signals:
		void modelAdded();
		void modelRemoved();

	private:

	};
};