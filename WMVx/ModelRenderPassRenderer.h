#pragma once
#include <optional>
#include "core/modeling/Model.h"
#include "core/modeling/AnimationCommon.h"


class ModelRenderPassRenderer
{
public:


	static bool start(const core::RenderOptions& renderOptions,
		const core::ModelTextureInfo* textureInfo, 
		const core::RawModel* model, 
		std::optional<size_t> animation_index,
		const core::ModelRenderPass& pass,
		const core::AnimationTickArgs& tick);
	static void finish(const core::ModelRenderPass& pass);
};

