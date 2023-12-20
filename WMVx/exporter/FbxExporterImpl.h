#pragma once

#include "FbxExporter.h"
#include "../core/utility/ScopeGuard.h"
#include "../core/utility/Vector3.h"
#include <fbxsdk.h>

namespace exporter {

	template<class T>
	inline constexpr bool IsGeosetAware = requires(const T* t) {
		{ t->isGeosetIndexVisible(size_t()) } -> std::convertible_to<bool>;
	};

	using namespace core;

	template<class T>
	FbxNode* createMesh(const T* model, FbxManager* pManager, FbxScene* pScene, core::Matrix matrix = core::Matrix::identity()) {
		const auto model_name = model->model->getFileInfo().toString();
		FbxNode* meshNode = FbxNode::Create(pScene, qPrintable(model_name));

		// Create new Matrix Data
		Matrix m = Matrix::newScale(matrix.getScale());

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
		const auto& model_indices = model->model->getIndices();
		for (const auto& pass : model->model->getRenderPasses()) {
			const auto& geoset = model->model->getGeosetAdaptors().at(pass.geosetIndex);
			if constexpr (IsGeosetAware<T>) {
				if (!model->isGeosetIndexVisible(pass.geosetIndex)) {
					continue;
				}
			}

			FbxString material_name = QString("material_" + QString::number(material_index)).toStdString().c_str();
			FbxSurfaceMaterial* material = pScene->GetMaterial(material_name.Buffer());
			meshNode->AddMaterial(material);

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


template<class T>
void FbxExporter::FbxModelFile::createMaterials(const T* model, fbxsdk::FbxNode* pMeshNode)
{
	const auto model_name = model->model->getFileInfo().toString();

	auto pass_index = 0;
	for (const auto& pass : model->model->getRenderPasses()) {

		if constexpr (IsGeosetAware<T>) {
			if (!model->isGeosetIndexVisible(pass.geosetIndex)) {
				continue;
			}
		}

		FbxString material_name = (model_name + "_" + QString::number(pass_index++)).toStdString().c_str();
		FbxString shader_name = "Phong";
		FbxSurfacePhong* material = FbxSurfacePhong::Create(mSdkManager, material_name);
		material->Ambient.Set(FbxDouble3(0.7, 0.7, 0.7));

		QString tex_name = QString::number(pass.tex) + ".png";
		QString tex_fullpath_filename = fileName + "." + tex_name;

		textures.insert({
			model->getTextureId(pass.tex),
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
	}
}

};