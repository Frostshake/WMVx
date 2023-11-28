#include "../stdafx.h"
#include "FbxExporter.h"
#include "../core/utility/ScopeGuard.h"
#include "../core/utility/Vector3.h"

#include <stdexcept>
#include <fbxsdk.h>

namespace exporter {

	using namespace core;

	void exportGLTexture(GLuint tex_id, const QString& filename) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		GLint width, height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		std::vector<uint8_t> pixels;
		pixels.resize(width * height * 4);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels.data());

		QImage img((uchar*)pixels.data(), width, height, QImage::Format::Format_ARGB32);
		img.save(filename);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	FbxNode* FbxExporter::createMesh(core::Model* model, FbxManager* pManager, FbxScene* pScene) {
		const auto model_name = model->model->getFileInfo().toString();
		FbxNode* meshNode = FbxNode::Create(pScene, qPrintable(model_name));

		

		const auto num_vertices = model->model->getRawVertices().size();
		FbxMesh* mesh = FbxMesh::Create(pScene, model_name.toStdString().c_str());
		mesh->InitControlPoints(num_vertices);
		FbxVector4* vertices = mesh->GetControlPoints();

		// Set the normals on Layer 0.
		auto layer = mesh->GetLayer(0);
		if (layer == nullptr)
		{
			mesh->CreateLayer();
			layer = mesh->GetLayer(0);
		}


		// We want to have one normal for each vertex (or control point),
		// so we set the mapping mode to eBY_CONTROL_POINT.
		FbxLayerElementNormal* layer_normal = FbxLayerElementNormal::Create(mesh, "");
		layer_normal->SetMappingMode(FbxLayerElement::eByControlPoint);
		layer_normal->SetReferenceMode(FbxLayerElement::eDirect);

		// Create UV for Diffuse channel.
		FbxLayerElementUV* layer_texcoord = FbxLayerElementUV::Create(mesh, "DiffuseUV");
		layer_texcoord->SetMappingMode(FbxLayerElement::eByControlPoint);
		layer_texcoord->SetReferenceMode(FbxLayerElement::eDirect);
		layer->SetUVs(layer_texcoord, FbxLayerElement::eTextureDiffuse);

		// Fill data.

		size_t vert_index = 0;
		for (const auto& raw_vert : model->model->getRawVertices()) {
			const Vector3& position = Vector3::yUpToZUp(raw_vert.position);
			vertices[vert_index].Set(
				position.x * FbxExporter::SCALE_FACTOR,
				position.y * FbxExporter::SCALE_FACTOR,
				position.z * FbxExporter::SCALE_FACTOR
			);
			const auto normal = Vector3(raw_vert.normal).normalize();
			layer_normal->GetDirectArray().Add(FbxVector4(normal.x, normal.y, normal.z));
			layer_texcoord->GetDirectArray().Add(FbxVector2(raw_vert.textureCoords.x, 1.0 - raw_vert.textureCoords.y));
			vert_index++;
		}

		// Create polygons.
		FbxLayerElementMaterial* layer_material = FbxLayerElementMaterial::Create(mesh, "");
		layer_material->SetMappingMode(FbxLayerElement::eByPolygon);
		layer_material->SetReferenceMode(FbxLayerElement::eIndexToDirect);
		layer->SetMaterials(layer_material);

		auto material_index = 0;
		const auto& model_indices = model->model->getIndices();
		for (const auto& pass : model->model->getRenderPasses()) {
			FbxString material_name = QString("material_" + QString::number(material_index)).toStdString().c_str();
			FbxSurfaceMaterial* material = pScene->GetMaterial(material_name.Buffer());
			meshNode->AddMaterial(material);

			const auto& geoset = model->model->getGeosetAdaptors().at(pass.geosetIndex);
			const auto num_faces = geoset->getTriangleCount() / 3;

			for (auto i = 0; i < num_faces; i++) {
				const auto pos = geoset->getTriangleStart() + i * 3;
				mesh->BeginPolygon(material_index);
				mesh->AddPolygon(model_indices[pos]);
				mesh->AddPolygon(model_indices[pos + 1]);
				mesh->AddPolygon(model_indices[pos + 2]);
				mesh->EndPolygon();
			}

			material_index++;
		}

		layer->SetNormals(layer_normal);

		FbxGeometryConverter lGeometryConverter(pManager);
		lGeometryConverter.ComputeEdgeSmoothingFromNormals(mesh);
		//convert soft/hard edge info to smoothing group info
		lGeometryConverter.ComputePolygonSmoothingFromEdgeSmoothing(mesh);

		// Set the mesh as the node attribute of the node.
		meshNode->SetNodeAttribute(mesh);

		// Set the shading mode to view texture.
		meshNode->SetShadingMode(FbxNode::eTextureShading);

		return meshNode;
	}

	FbxNode* FbxExporter::createSkeleton(core::Model* model, FbxManager* pManager, FbxScene* pScene) {
		const auto model_name = model->model->getFileInfo().toString();
		FbxNode* skeletonNode = FbxNode::Create(pManager, qPrintable(model_name + "_rig"));
		FbxSkeleton* bone_group_skeleton_attribute = FbxSkeleton::Create(pScene, "");
		bone_group_skeleton_attribute->SetSkeletonType(FbxSkeleton::eRoot);
		bone_group_skeleton_attribute->Size.Set(10.0 * FbxExporter::SCALE_FACTOR);
		skeletonNode->SetNodeAttribute(bone_group_skeleton_attribute);


		const auto num_bones = model->model->getBoneAdaptors().size();
		std::vector<bool> has_children;
		has_children.resize(num_bones);

		const auto& bones = model->model->getBoneAdaptors();

		for (const auto& bone : bones) {
			const auto parent_id = bone->getParentBoneId();
			if (parent_id != -1) {
				has_children[parent_id] = true;
			}
		}

		size_t bone_index = 0;
		for (const auto& bone : bones) {

			FbxSkeleton::EType bone_type = FbxSkeleton::EType::eLimbNode;

			if (bone->getParentBoneId() == -1) {
				bone_type = FbxSkeleton::eRoot;
			}
			else if (has_children[bone_index]) {
				bone_type = FbxSkeleton::eLimb;
			}


			auto trans = bone->getPivot();
			auto pid = bone->getParentBoneId();
			if (pid > -1) {
				trans -= model->model->getBoneAdaptors().at(pid)->getPivot();
			}

			FbxString bone_name(qPrintable(model_name + "_bone_"+QString::number(bone_index)));
			FbxNode* bone_node = FbxNode::Create(pScene, bone_name);
			bone_nodes_map[bone_index] = bone_node;
			bone_node->LclTranslation.Set(FbxVector4(
				trans.x * FbxExporter::SCALE_FACTOR,
				trans.y * FbxExporter::SCALE_FACTOR,
				trans.z * FbxExporter::SCALE_FACTOR
			));

			FbxSkeleton* skeleton_attribute = FbxSkeleton::Create(pScene, bone_name);
			skeleton_attribute->SetSkeletonType(bone_type);

			switch (bone_type) {
			case FbxSkeleton::EType::eRoot:
				skeleton_attribute->Size.Set(10.0 * FbxExporter::SCALE_FACTOR);
				skeletonNode->AddChild(bone_node);
				break;
			case FbxSkeleton::EType::eLimb:
				skeleton_attribute->LimbLength.Set(5.0 * FbxExporter::SCALE_FACTOR * (sqrtf(trans.x * trans.x + trans.y * trans.y + trans.z * trans.z)));
				bone_nodes_map[pid]->AddChild(bone_node);
				break;
			default:
				skeleton_attribute->Size.Set(1.0 * FbxExporter::SCALE_FACTOR);
				bone_nodes_map[pid]->AddChild(bone_node);
				break;
			}


			bone_node->SetNodeAttribute(skeleton_attribute);

			bone_index++;
		}

		return skeletonNode;
	}

	void FbxExporter::createMaterials(core::Model* model, fbxsdk::FbxManager* pManager, fbxsdk::FbxScene* pScene, fbxsdk::FbxNode* pMeshNode) {
		const auto model_name = model->model->getFileInfo().toString();
		auto pass_index = 0;
		for (const auto& pass : model->model->getRenderPasses()) {

			FbxString material_name = (model_name + "_" + QString::number(pass_index)).toStdString().c_str();
			FbxString shader_name = "Phong";
			FbxSurfacePhong* material = FbxSurfacePhong::Create(pManager, material_name);
			material->Ambient.Set(FbxDouble3(0.7, 0.7, 0.7));

			QString tex_name = QString::number(pass.tex) + ".png";
			QString tex_fullpath_filename = destinationFileName + "." + tex_name;

			textures.insert({
				model->getTextureId(pass.tex),
				tex_fullpath_filename
			});

			FbxFileTexture* texture = FbxFileTexture::Create(pManager, tex_name.toStdString().c_str());
			texture->SetFileName(tex_fullpath_filename.toStdString().c_str());
			texture->SetTextureUse(FbxTexture::eStandard);
			texture->SetMappingType(FbxTexture::eUV);
			texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
			texture->SetSwapUV(false);
			texture->SetTranslation(0.0, 0.0);
			texture->SetScale(1.0, 1.0);
			texture->SetRotation(0.0, 0.0);
			texture->UVSet.Set(FbxString("DiffuseUV"));
			material->Diffuse.ConnectSrcObject(texture);

			pMeshNode->AddMaterial(material);

			pass_index++;
		}
	}

	std::vector<FbxCluster*> linkMeshAndSkeleton(core::Model* model, FbxScene* pScene, FbxNode* pMeshNode, FbxNode* pSkelNode, std::map<uint32_t, FbxNode*>& bone_node_map) {
		std::vector<FbxCluster*> bone_clusters;

		for (const auto& it : bone_node_map) {
			FbxCluster* cluster = FbxCluster::Create(pScene, "");
			bone_clusters.push_back(cluster);
			cluster->SetLink(it.second);
			cluster->SetLinkMode(FbxCluster::ELinkMode::eNormalize);
		}

		{
			auto vert_index = 0;
			const auto& raw_verts = model->model->getRawVertices();
			for (const auto& vert : raw_verts) {
				for (size_t i = 0; i < 4; i++) {
					if (vert.boneWeights[i] > 0) {
						bone_clusters[vert.bones[i]]->AddControlPointIndex(vert_index, static_cast<double>(vert.boneWeights[i]) / 255.0);
					}
				}
				vert_index++;
			}
		}

		{
			FbxAMatrix matrix = pMeshNode->EvaluateGlobalTransform();
			for (auto& it : bone_clusters) {
				it->SetTransformMatrix(matrix);
			}
		}

		{
			auto cluster_it = bone_clusters.begin();
			for (auto& it : bone_node_map) {
				FbxAMatrix matrix = it.second->EvaluateGlobalTransform();
				(*cluster_it)->SetTransformLinkMatrix(matrix);
				++cluster_it;
			}
		}

		{
			FbxGeometry* mesh_attribute = (FbxGeometry*)pMeshNode->GetNodeAttribute();
			FbxSkin* skin = FbxSkin::Create(pScene, "");

			for (auto& it : bone_clusters) {
				skin->AddCluster(it);
			}

			mesh_attribute->AddDeformer(skin);
		}

		return bone_clusters;
	}

	void storeBindPose(FbxScene* pScene, FbxNode* pMeshNode, const std::vector<FbxCluster*>& bone_clusters) {
		FbxPose* pose = FbxPose::Create(pScene, "Bind Pose");
		pose->SetIsBindPose(true);

		for (auto it : bone_clusters)
		{
			FbxNode* node = it->GetLink();
			FbxMatrix matrix = node->EvaluateGlobalTransform();
			pose->Add(node, matrix);
		}

		pose->Add(pMeshNode, pMeshNode->EvaluateGlobalTransform());

		pScene->AddPose(pose);
	}

	void storeRestPose(FbxScene* pScene, FbxNode* pSkelNode) {
		//TODO ?
	}

	void FbxExporter::addModelToScene(core::Model* model, fbxsdk::FbxManager* pSdkManager, fbxsdk::FbxScene* pScene) {
		FbxNode* lRootNode = pScene->GetRootNode();

		FbxNode* primary_model_mesh_node = createMesh(model, pSdkManager, pScene);
		FbxNode* primary_model_skeleton_node = createSkeleton(model, pSdkManager, pScene);

		//TODO add attachments, relations

		lRootNode->AddChild(primary_model_mesh_node);
		lRootNode->AddChild(primary_model_skeleton_node);

		createMaterials(model, pSdkManager, pScene, primary_model_mesh_node);

		std::vector<FbxCluster*> bone_clusters = linkMeshAndSkeleton(model, pScene, primary_model_mesh_node, primary_model_skeleton_node, bone_nodes_map);

		storeBindPose(pScene, primary_model_mesh_node, bone_clusters);
		storeRestPose(pScene, primary_model_skeleton_node);
	}

	bool FbxExporter::execute() {

		auto reset_guard = sg::make_scope_guard([this]() {
			for (const auto& item : textures) {
				if (QFile::exists(item.second)) {
					QFile::remove(item.second);
				}
			}
			textures.clear();

			bone_nodes_map.clear();
		});


		FbxManager* lSdkManager = nullptr;
		FbxScene* lScene = nullptr;

		auto guard = sg::make_scope_guard([&]() {
			if (lSdkManager) {
				lSdkManager->Destroy();
			}
		});

		lSdkManager = FbxManager::Create();
		if (!lSdkManager) {
			throw std::runtime_error("Unable to create FBX sdk manager.");
		}

		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		ios->SetBoolProp(EXP_FBX_MATERIAL, true);
		ios->SetBoolProp(EXP_FBX_TEXTURE, true);
		ios->SetBoolProp(EXP_FBX_EMBEDDED, true);
		ios->SetBoolProp(EXP_FBX_SHAPE, true);
		ios->SetBoolProp(EXP_FBX_GOBO, true);
		ios->SetBoolProp(EXP_FBX_ANIMATION, false);
		ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

		lScene = FbxScene::Create(lSdkManager,"Scene");
		if (!lScene) {
			throw std::runtime_error("Unable to create FBX scene.");
		}

		{
			FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(lSdkManager, "SceneInfo");
			sceneInfo->mTitle = "WMVx Scene";
			sceneInfo->mAuthor = "WMVx";
			sceneInfo->mRevision = "0.0.0";
			lScene->SetSceneInfo(sceneInfo);
		}

		for (const auto& model : models) {
			addModelToScene(model, lSdkManager, lScene);
		}

		for (const auto& item : textures) {
			exportGLTexture(item.first, item.second);
		}

		fbxsdk::FbxExporter* lExporter = fbxsdk::FbxExporter::Create(lSdkManager, "");
		auto exporter_guard = sg::make_scope_guard([&]() {
			if (lExporter) {
				lExporter->Destroy();
			}
		});

		int lFileFormat = -1;

		if (!lExporter->Initialize(destinationFileName.toStdString().c_str(), lFileFormat, lSdkManager->GetIOSettings())) {
			throw std::runtime_error("Unable to initialise exporter.");
		}

		lExporter->SetFileExportVersion(FBX_2014_00_COMPATIBLE);

		bool result = lExporter->Export(lScene);

		if (!result) {
			throw std::runtime_error("Export unsuccessful.");
		}
	}

}