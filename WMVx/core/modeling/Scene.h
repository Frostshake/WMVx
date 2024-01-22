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
		Scene(QObject* _parent);
		Scene(const Scene& instance) = delete;
		virtual ~Scene();

		Model* addModel(std::unique_ptr<Model> m);
		void removeModel(ModelMeta::id_t id);

		std::vector<std::unique_ptr<Model>> models;
		TextureManager textureManager;

		bool showGrid;

		Model* selectedModel() const;
		void setSelectedModel(Model* model);

	signals:
		void modelAdded(Model* model);
		void modelRemoved(Model* model);
		void modelSelectionChanged(Model* model);

	private:
		ModelMeta::id_t next_id;
		Model* active_model;

	};
};