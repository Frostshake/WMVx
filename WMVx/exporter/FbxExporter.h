#pragma once

#include "../core/modeling/Model.h"
#include "../Formatting.h"
#include <QString>
#include <vector>

namespace fbxsdk {
	class FbxManager;
	class FbxScene;
	class FbxNode;
	class FbxIOSettings;
};

namespace exporter {

	class FbxExporter {
	public:

		constexpr static float SCALE_FACTOR = 50.0f;

		FbxExporter(const QString& outname) : destinationFileName(outname) {

		}
		FbxExporter(FbxExporter&&) = default;
		FbxExporter(const FbxExporter&) = delete;
		virtual ~FbxExporter() { }

		FbxExporter& addModel(core::Model* model, AnimationOptions animations) {
			models.push_back({ model, animations});
			return *this;
		}

		bool execute();

	protected:

		class FbxFileBase {
		public:
			FbxFileBase(const QString name);
			FbxFileBase(FbxFileBase&&) = default;
			FbxFileBase(const FbxFileBase&) = delete;
			virtual ~FbxFileBase();

		protected:

			void init();
			void output();

			fbxsdk::FbxManager* mSdkManager;
			fbxsdk::FbxScene* mScene;
			fbxsdk::FbxIOSettings* mIOS;

			const QString fileName;
		};

		class FbxModelFile: public FbxFileBase {
		public:
			FbxModelFile(const QString name) : FbxFileBase(name) {
			}
			FbxModelFile(FbxModelFile&&) = default;
			FbxModelFile(const FbxModelFile&) = delete;
			virtual ~FbxModelFile();

			void build(core::Model* model);

		protected:
			void createMaterials(const core::ModelTextureInfo* texInfo, core::RawModel* model, fbxsdk::FbxNode* pMeshNode);

			std::map<GLuint, QString> textures;
		};

		class FbxAnimFile : public FbxFileBase {
		public:
			FbxAnimFile(const QString name) : FbxFileBase(name) {
			}
			FbxAnimFile(FbxAnimFile&&) = default;
			FbxAnimFile(const FbxAnimFile&) = delete;
			virtual ~FbxAnimFile() {}

			void build(core::Model* model, const AnimationOption& anim_opt);

		protected:
			void createAnimation(core::Model* model, const AnimationOption& anim_opt, std::map<uint32_t, fbxsdk::FbxNode*>& bone_nodes_map);
		};

	private:
		
		const QString destinationFileName;
		std::vector<std::pair<core::Model*, AnimationOptions>> models;
	};

};