#pragma once

#include "VanillaM2Definitions.h"
#include "VanillaModelSupport.h"
#include "GenericModelAdaptors.h"

namespace core {

	class VanillaModelAnimationSequenceAdaptor : public ModelAnimationSequenceAdaptor {
	public:
		VanillaModelAnimationSequenceAdaptor(VanillaAnimationSequenceM2* handle) {
			this->handle = handle;
		}
		VanillaModelAnimationSequenceAdaptor(VanillaModelAnimationSequenceAdaptor&&) = default;

		virtual ~VanillaModelAnimationSequenceAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return handle->id;
		};

		constexpr virtual uint16_t getVariationId() const {
			return handle->variationId;
		};

		constexpr virtual uint32_t getDuration() const {
			return handle->endTimestamp - handle->startTimestamp;
		}

	protected:
		VanillaAnimationSequenceM2* handle;
	};

};