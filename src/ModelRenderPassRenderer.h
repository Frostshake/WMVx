#pragma once
#include <optional>
#include "core/modeling/Model.h"
#include "core/modeling/Animation.h"


class ModelRenderPassRenderer
{
public:
	static bool start(const core::RenderOptions& renderOptions,
		const core::ModelTextureInfo* textureInfo, 
		const core::M2Model* model,
		std::optional<size_t> animation_index,
		const core::ModelRenderPass& pass,
		const core::AnimationTickArgs& tick);
	static void finish(const core::ModelRenderPass& pass);
};

