#pragma once

#include "../core/modeling/Model.h"
#include <QString>
#include <vector>

namespace fbxsdk {
	class FbxManager;
	class FbxScene;
	class FbxNode;
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

		FbxExporter& addModel(core::Model* model) {
			models.push_back(model);
			return *this;
		}

		bool execute();

	private:

		void addModelToScene(core::Model* model, fbxsdk::FbxManager* pSdkManager, fbxsdk::FbxScene* pScene);

		fbxsdk::FbxNode* createMesh(core::Model* model, fbxsdk::FbxManager* pManager, fbxsdk::FbxScene* pScene);

		fbxsdk::FbxNode* createSkeleton(core::Model* model, fbxsdk::FbxManager* pManager, fbxsdk::FbxScene* pScene);

		void createMaterials(core::Model* model, fbxsdk::FbxManager* pManager, fbxsdk::FbxScene* pScene, fbxsdk::FbxNode* pMeshNode);


		const QString destinationFileName;

		std::map<GLuint, QString> textures;
		std::map<uint32_t, fbxsdk::FbxNode*> bone_nodes_map;

		std::vector<core::Model*> models;
	};

};