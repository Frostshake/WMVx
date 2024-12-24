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

		struct Selection {
			Model* root;
			ComponentMeta* component;
		};

		Scene(QObject* _parent);
		Scene(const Scene& instance) = delete;
		virtual ~Scene();

		Model* addModel(std::unique_ptr<Model> m);
		ComponentMeta* addComponent(ComponentMeta* meta);
		void removeComponent(ComponentMeta* meta);

		std::vector<std::unique_ptr<Model>> models;
		TextureManager textureManager;

		bool showGrid;

		const Selection& selected() const;
		void setSelectedModel(Model* model, ComponentMeta* component);

		void componentUpdated(ComponentMeta* component);

		Model* findComponentRoot(ComponentMeta* meta) const;

	signals:
		void componentAdded(ComponentMeta* meta);
		void componentRemoved(ComponentMeta* meta);
		void modelSelectionChanged(const Selection& selection);
		void sceneChanged();


	private:
		Selection active_selection;

	};
};