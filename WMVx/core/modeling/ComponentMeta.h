#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <memory>
#include "../filesystem/GameFileUri.h"

namespace core {

	class Scene;

	struct RenderOptions {
		bool showWireFrame;
		bool showBounds;
		bool showBones;
		bool showTexture;
		bool showRender;
		bool showParticles;
		float opacity;


		RenderOptions() {
			showWireFrame = false;
			showBounds = false;
			showBones = false;
			showTexture = true;
			showRender = true;
			showParticles = true;
			opacity = 1.f;
		}
	};

	class ComponentMeta {
	public:
		friend class Scene;

		enum class Type {
			ROOT,
			ATTACHMENT,
			EFFECT,
			MERGED
		};


		ComponentMeta(Type type) {
			_type = type;
		}
		virtual ~ComponentMeta() {}

		Type getMetaType() const {
			return _type;
		};
		
		virtual GameFileInfo getMetaGameFileInfo() const = 0;

		QString getMetaLabel() const {
			if (_name.length() > 0) {
				return _name;
			}

			return getMetaGameFileInfo().toString();
		}

		QString getMetaName() const {
			return _name;
		}

		void setMetaName(QString str) {
			_name = str;
		}

		virtual std::vector<ComponentMeta*> getMetaChildren() const = 0;

		RenderOptions renderOptions;

	private: 
		Type _type;
		QString _name;
	};

}