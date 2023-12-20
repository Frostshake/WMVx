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

	fbxsdk::FbxAnimCurveDef::EInterpolationType toFbxInterpolation(core::Interpolation type) {
		return type == core::Interpolation::INTERPOLATION_LINEAR ? FbxAnimCurveDef::eInterpolationLinear : FbxAnimCurveDef::eInterpolationCubic;
	}

	FbxNode* createMesh(core::RawModel* model, FbxManager* pManager, FbxScene* pScene, core::Matrix matrix = core::Matrix::identity()) {
		const auto model_name = model->getFileInfo().toString();
		FbxNode* meshNode = FbxNode::Create(pScene, qPrintable(model_name));

		// Create new Matrix Data
		Matrix m = Matrix::newScale(matrix.getScale());

		const auto num_vertices = model->getRawVertices().size();
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
		for (const auto& raw_vert : model->getRawVertices()) {
			const Vector3& position = m * Vector3::yUpToZUp(raw_vert.position);
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
		const auto& model_indices = model->getIndices();
		for (const auto& pass : model->getRenderPasses()) {
			FbxString material_name = QString("material_" + QString::number(material_index)).toStdString().c_str();
			FbxSurfaceMaterial* material = pScene->GetMaterial(material_name.Buffer());
			meshNode->AddMaterial(material);

			const auto& geoset = model->getGeosetAdaptors().at(pass.geosetIndex);
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

	FbxNode* createSkeleton(core::Model* model, FbxManager* pManager, FbxScene* pScene, std::map<uint32_t, fbxsdk::FbxNode*>& bone_nodes_map) {
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

	void FbxExporter::FbxModelFile::createMaterials(const core::ModelTextureInfo* texInfo, core::RawModel* model,fbxsdk::FbxNode* pMeshNode) {
		const auto model_name = model->getFileInfo().toString();
		auto pass_index = 0;
		for (const auto& pass : model->getRenderPasses()) {

			FbxString material_name = (model_name + "_" + QString::number(pass_index)).toStdString().c_str();
			FbxString shader_name = "Phong";
			FbxSurfacePhong* material = FbxSurfacePhong::Create(mSdkManager, material_name);
			material->Ambient.Set(FbxDouble3(0.7, 0.7, 0.7));

			QString tex_name = QString::number(pass.tex) + ".png";
			QString tex_fullpath_filename = fileName + "." + tex_name;

			textures.insert({
				texInfo->getTextureId(pass.tex),
				tex_fullpath_filename
			});

			FbxFileTexture* texture = FbxFileTexture::Create(mSdkManager, tex_name.toStdString().c_str());
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

	std::vector<FbxCluster*> linkMeshAndSkeleton(core::Model* model, FbxScene* pScene, FbxNode* pMeshNode, FbxNode* pSkelNode, const std::map<uint32_t, FbxNode*>& bone_node_map) {
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


	bool FbxExporter::execute() {

		for (const auto& model : models) {
			{
				FbxModelFile model_file(destinationFileName);
				model_file.build(model.first);
			}

			const QString anim_dir_name = QString(destinationFileName).replace(".fbx", "_animations");

			for (const auto& anim : model.second) {
				
				QDir dir(anim_dir_name);
				if (!dir.exists()) {
					dir.mkdir(".");
				}

				const QString safe_anim_name = QString(anim.first).replace("/", "_");
				const QString animation_name = anim_dir_name + "/" + safe_anim_name + ".fbx";

				FbxAnimFile anim_file(animation_name);
				anim_file.build(model.first, anim.second);
			}
		}	

		return true;
	}

	FbxExporter::FbxModelFile::~FbxModelFile()
	{
		for (const auto& item : textures) {
			if (QFile::exists(item.second)) {
				QFile::remove(item.second);
			}
		}
	}

	void FbxExporter::FbxModelFile::build(core::Model* model)
	{
		init();

		mIOS->SetBoolProp(EXP_FBX_MATERIAL, true);
		mIOS->SetBoolProp(EXP_FBX_TEXTURE, true);
		mIOS->SetBoolProp(EXP_FBX_EMBEDDED, true);
		mIOS->SetBoolProp(EXP_FBX_SHAPE, true);
		mIOS->SetBoolProp(EXP_FBX_GOBO, true);
		mIOS->SetBoolProp(EXP_FBX_ANIMATION, false);
		mIOS->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

		{
			FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(mSdkManager, "SceneInfo");
			sceneInfo->mTitle = "WMVx Model Scene";
			sceneInfo->mAuthor = "WMVx";
			sceneInfo->mRevision = "0.0.0";
			mScene->SetSceneInfo(sceneInfo);
		}

		{
			std::map<uint32_t, fbxsdk::FbxNode*> bone_nodes_map;

			FbxNode* lRootNode = mScene->GetRootNode();
			FbxNode* primary_model_mesh_node = createMesh(model->model.get(), mSdkManager, mScene);
			FbxNode* primary_model_skeleton_node = createSkeleton(model, mSdkManager, mScene, bone_nodes_map);

			lRootNode->AddChild(primary_model_mesh_node);
			lRootNode->AddChild(primary_model_skeleton_node);

			createMaterials(model, model->model.get(), primary_model_mesh_node);

			for (const auto* attachment : model->getAttachments()) {
				std::map<uint32_t, fbxsdk::FbxNode*> attach_bone_nodes_map;
				Matrix m = model->model->getBoneAdaptors()[attachment->bone]->getMat();
				FbxNode* attach_mesh_node = createMesh(attachment->model.get(), mSdkManager, mScene, m);
				FbxNode* attach_skeleton_node = createSkeleton(model, mSdkManager, mScene, attach_bone_nodes_map);

				lRootNode->AddChild(attach_mesh_node);
				lRootNode->AddChild(attach_skeleton_node);

				createMaterials(attachment, attachment->model.get(), attach_mesh_node);
			}

			//TODO handle merged models.	

			std::vector<FbxCluster*> bone_clusters = linkMeshAndSkeleton(
				model, 
				mScene, 
				primary_model_mesh_node, 
				primary_model_skeleton_node, 
				bone_nodes_map
			);

			storeBindPose(mScene, primary_model_mesh_node, bone_clusters);
			storeRestPose(mScene, primary_model_skeleton_node);
		}

		for (const auto& item : textures) {
			exportGLTexture(item.first, item.second);
		}

		output();
	}

	void FbxExporter::FbxAnimFile::build(core::Model* model, const AnimationOption& anim_opt)
	{
		init();

		mIOS->SetBoolProp(EXP_FBX_MATERIAL, false);
		mIOS->SetBoolProp(EXP_FBX_TEXTURE, false);
		mIOS->SetBoolProp(EXP_FBX_EMBEDDED, false);
		mIOS->SetBoolProp(EXP_FBX_SHAPE, true);
		mIOS->SetBoolProp(EXP_FBX_GOBO, true);
		mIOS->SetBoolProp(EXP_FBX_ANIMATION, true);
		mIOS->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

		{
			FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(mSdkManager, "SceneInfo");
			sceneInfo->mTitle = "WMVx Animation Scene";
			sceneInfo->mAuthor = "WMVx";
			sceneInfo->mRevision = "0.0.0";
			mScene->SetSceneInfo(sceneInfo);
		}

		{
			std::map<uint32_t, fbxsdk::FbxNode*> bone_nodes_map;

			FbxNode* lRootNode = mScene->GetRootNode();
			FbxNode* primary_model_skeleton_node = createSkeleton(model, mSdkManager, mScene, bone_nodes_map);


			//TODO add attachments, relations

			lRootNode->AddChild(primary_model_skeleton_node);

			createAnimation(model, anim_opt, bone_nodes_map);
		}

		output();
	}

	FbxExporter::FbxFileBase::FbxFileBase(const QString name) 
		: fileName(name)
	{
		mSdkManager = nullptr;
		mScene = nullptr;
		mIOS = nullptr;
	}

	FbxExporter::FbxFileBase::~FbxFileBase()
	{
		if (mSdkManager) {
			mSdkManager->Destroy();
		}
	}

	void FbxExporter::FbxFileBase::init()
	{
		assert(mSdkManager == nullptr);


		mSdkManager = FbxManager::Create();
		if (!mSdkManager) {
			throw std::runtime_error("Unable to create FBX sdk manager.");
		}

		mIOS = FbxIOSettings::Create(mSdkManager, IOSROOT);
		mSdkManager->SetIOSettings(mIOS);

		mScene = FbxScene::Create(mSdkManager, "Scene");
		if (!mScene) {
			throw std::runtime_error("Unable to create FBX scene.");
		}
	}

	void FbxExporter::FbxFileBase::output()
	{
		fbxsdk::FbxExporter* lExporter = fbxsdk::FbxExporter::Create(mSdkManager, "");
		auto exporter_guard = sg::make_scope_guard([&]() {
			if (lExporter) {
				lExporter->Destroy();
			}
		});

		int lFileFormat = -1;

		if (!lExporter->Initialize(fileName.toStdString().c_str(), lFileFormat, mSdkManager->GetIOSettings())) {
			throw std::runtime_error("Unable to initialise exporter.");
		}

		lExporter->SetFileExportVersion(FBX_2014_00_COMPATIBLE);

		bool result = lExporter->Export(mScene);

		if (!result) {
			auto status = lExporter->GetStatus();
			throw std::runtime_error("Export unsuccessful.");
		}
	}

	void FbxExporter::FbxAnimFile::createAnimation(core::Model* model, const AnimationOption& anim_opt, std::map<uint32_t, fbxsdk::FbxNode*>& bone_nodes_map)
	{
		if (bone_nodes_map.empty()) {
			return;
		}

		const auto* animation = model->model->getModelAnimationSequenceAdaptors().at(anim_opt.index);
		
		FbxAnimStack* anim_stack = FbxAnimStack::Create(mScene, "Anim Layer");
		FbxAnimLayer* anim_layer = FbxAnimLayer::Create(mScene, "Anim layer");
		anim_stack->AddMember(anim_layer);

		FbxTime::SetGlobalTimeMode(FbxTime::eFrames60);

		const uint32_t duration = animation->getDuration();
		float interval = duration / 60;
		if (interval < 1.0f) {
			interval = duration;
		}

		Animator animator;
		animator.setSpeed(1.0f);
		animator.setPaused(false);
		animator.setFrame(0);


		for (uint32_t t = 0; t < duration; t += interval)
		{
			FbxTime time;
			time.SetSecondDouble((float)t / 1000.0);

			const auto& tick = animator.tick(t);

			for (auto& it : bone_nodes_map) {
				const auto* bone = model->model->getBoneAdaptors()[it.first];

				const auto* translation = bone->getTranslation();
				const auto* rotation = bone->getRotation();
				const auto* scale = bone->getScale();

				const bool has_translation = translation->uses(anim_opt.index);
				const bool has_rotation = rotation->uses(anim_opt.index);
				const bool has_scale = scale->uses(anim_opt.index);

				if (!has_translation && !has_rotation && !has_scale) {
					continue;
				}

				if (has_translation) {
					FbxAnimCurve* t_curve_x = it.second->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X, true);
					FbxAnimCurve* t_curve_y = it.second->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y, true);
					FbxAnimCurve* t_curve_z = it.second->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z, true);

					Vector3 value = translation->getValue(anim_opt.index, tick);

					if (bone->getParentBoneId() != -1)
					{
						const auto* parent_bone = model->model->getBoneAdaptors()[bone->getParentBoneId()];
						value += (bone->getPivot() - parent_bone->getPivot());
					}

					const auto type = toFbxInterpolation(translation->getType());

					t_curve_x->KeyModifyBegin();
					int key_index = t_curve_x->KeyAdd(time);
					t_curve_x->KeySetValue(key_index, value.x * SCALE_FACTOR);
					t_curve_x->KeySetInterpolation(key_index, type);
					t_curve_x->KeyModifyEnd();

					t_curve_y->KeyModifyBegin();
					key_index = t_curve_y->KeyAdd(time);
					t_curve_y->KeySetValue(key_index, value.y * SCALE_FACTOR);
					t_curve_y->KeySetInterpolation(key_index, type);
					t_curve_y->KeyModifyEnd();

					t_curve_z->KeyModifyBegin();
					key_index = t_curve_z->KeyAdd(time);
					t_curve_z->KeySetValue(key_index, value.z * SCALE_FACTOR);
					t_curve_z->KeySetInterpolation(key_index, type);
					t_curve_z->KeyModifyEnd();
				}

				if (has_rotation) {
					FbxAnimCurve* r_curve_x = it.second->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X, true);
					FbxAnimCurve* r_curve_y = it.second->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y, true);
					FbxAnimCurve* r_curve_z = it.second->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z, true);

					Quaternion q = rotation->getValue(anim_opt.index, t);
					Quaternion tq;
					tq.x = q.w; tq.y = q.x; tq.z = q.y; tq.w = q.z;

					Vector3 angles = tq.toEulerXYZ();

					const auto x = angles.x * -(180.0f / PI);
					const auto y = angles.y * -(180.0f / PI);
					const auto z = angles.z * -(180.0f / PI);

					const auto type = toFbxInterpolation(rotation->getType());

					r_curve_x->KeyModifyBegin();
					int key_index = r_curve_x->KeyAdd(time);
					r_curve_x->KeySetValue(key_index, x);
					r_curve_x->KeySetInterpolation(key_index, type);
					r_curve_x->KeyModifyEnd();

					r_curve_y->KeyModifyBegin();
					key_index = r_curve_y->KeyAdd(time);
					r_curve_y->KeySetValue(key_index, y);
					r_curve_y->KeySetInterpolation(key_index, type);
					r_curve_y->KeyModifyEnd();

					r_curve_z->KeyModifyBegin();
					key_index = r_curve_z->KeyAdd(time);
					r_curve_z->KeySetValue(key_index, z);
					r_curve_z->KeySetInterpolation(key_index, type);
					r_curve_z->KeyModifyEnd();
				}

				if (has_scale) {
					FbxAnimCurve* s_curve_x = it.second->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X, true);
					FbxAnimCurve* s_curve_y = it.second->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y, true);
					FbxAnimCurve* s_curve_z = it.second->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z, true);

					Vector3 value = scale->getValue(anim_opt.index, t);

					const auto type = toFbxInterpolation(scale->getType());

					s_curve_x->KeyModifyBegin();
					int key_index = s_curve_x->KeyAdd(time);
					s_curve_x->KeySetValue(key_index, value.x);
					s_curve_x->KeySetInterpolation(key_index, type);
					s_curve_x->KeyModifyEnd();

					s_curve_y->KeyModifyBegin();
					key_index = s_curve_y->KeyAdd(time);
					s_curve_y->KeySetValue(key_index, value.y);
					s_curve_y->KeySetInterpolation(key_index, type);
					s_curve_y->KeyModifyEnd();

					s_curve_z->KeyModifyBegin();
					key_index = s_curve_z->KeyAdd(time);
					s_curve_z->KeySetValue(key_index, value.z);
					s_curve_z->KeySetInterpolation(key_index, type);
					s_curve_z->KeyModifyEnd();
				}
			}			
		}
	}
}