#pragma once

#include "WOTLKM2Definitions.h"
#include "WOTLKModelSupport.h"
#include "GenericModelAdaptors.h"

namespace core {

	class WOTLKModelAnimationSequenceAdaptor : public ModelAnimationSequenceAdaptor {
	public:
		WOTLKModelAnimationSequenceAdaptor(WOTLKAnimationSequenceM2* handle) {
			this->handle = handle;
		}
		WOTLKModelAnimationSequenceAdaptor(WOTLKModelAnimationSequenceAdaptor&&) = default;

		virtual ~WOTLKModelAnimationSequenceAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return handle->id;
		};

		constexpr virtual uint16_t getVariationId() const {
			return handle->variationId;
		};

		constexpr virtual uint32_t getDuration() const {
			return handle->duration;
		}

	protected:
		WOTLKAnimationSequenceM2* handle;
	};

}