#pragma once
#include "VanillaAnimation.h"
#include "ModelAdaptors.h"
#include "../utility/Matrix.h"

namespace core {
	class VanillaBone : public ModelBoneAdaptor {
	public:
		VanillaBone() = default;
		VanillaBone(VanillaBone&&) = default;
		virtual ~VanillaBone() {}

		VanillaAnimated<Vector3> translation;
		VanillaAnimated<Quaternion> rotation; //TODO see old source (two implementations), not sure which is correct for wotlk
		VanillaAnimated<Vector3> scale;

		Vector3 pivot;
		Vector3 translationPivot;

		bool billboard;
		//TODO better names
		Matrix mat;
		Matrix mrot; 

		VanillaModelBoneM2 boneDefinition;

		bool calculated;

		virtual const Matrix& getMat() const {
			return mat;
		}

		virtual const Matrix& getMRot() const {
			return mrot;
		}

		virtual const Vector3& getTranslationPivot() const {
			return translationPivot;
		}

		virtual const Vector3& getPivot() const {
			return pivot;
		}

		virtual int16_t getParentBoneId() const {
			return boneDefinition.parentBoneId;
		}

		virtual void resetCalculated() {
			calculated = false;
		}

		virtual void calculateMatrix(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			if (calculated) {
				return;
			}

			Matrix m;
			Quaternion q;

			if (rotation.uses(animation_index) || scale.uses(animation_index) || translation.uses(animation_index) || billboard) {
				m.translation(pivot);

				if (translation.uses(animation_index)) {
					m *= Matrix::newTranslation(translation.getValue(animation_index, tick));
				}

				if (rotation.uses(animation_index)) {
					q = rotation.getValue(animation_index, tick);
					m *= Matrix::newQuatRotate(q);
				}

				if (scale.uses(animation_index)) {
					m *= Matrix::newScale(scale.getValue(animation_index, tick));
				}

				if (billboard) {
					//TODO
				}

				m *= Matrix::newTranslation(pivot * -1.0f);
			}
			else {
				m.unit();
			}

			if (boneDefinition.parentBoneId > -1) {
				allbones[boneDefinition.parentBoneId]->calculateMatrix(animation_index, tick, allbones);
				mat = allbones[boneDefinition.parentBoneId]->getMat() * m;
			}
			else {
				mat = m;
			}

			if (rotation.uses(animation_index)) {
				if (boneDefinition.parentBoneId >= 0) {
					mrot = allbones[boneDefinition.parentBoneId]->getMRot() * Matrix::newQuatRotate(q);
				}
				else {
					mrot = Matrix::newQuatRotate(q);
				}
			}
			else {
				mrot.unit();
			}

			//TODO
			translationPivot = mat * pivot;
			calculated = true;
		}
	};

	class VanillaModelColor : public ModelColorAdaptor {
	public:
		VanillaModelColor() = default;
		VanillaModelColor(VanillaModelColor&&) = default;
		virtual ~VanillaModelColor() {}

		VanillaAnimated<Vector3> color;
		VanillaAnimated<float, int16_t, ShortToFloat> opacity;

		virtual bool colorUses(size_t animation_index) const {
			return color.uses(animation_index);
		}

		virtual Vector3 colorValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return color.getValue(animation_index, tick);
		}

		virtual bool opacityUses(size_t animation_index) const {
			return opacity.uses(animation_index);
		}

		virtual float opacityValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return opacity.getValue(animation_index, tick);
		}

	};

	class VanillaModelTransparency : public ModelTransparencyAdaptor {
	public:
		VanillaModelTransparency() = default;
		VanillaModelTransparency(VanillaModelTransparency&&) = default;
		virtual ~VanillaModelTransparency() {}

		VanillaAnimated<float, int16_t, ShortToFloat> transparency;

		virtual bool transparencyUses(size_t animation_index) const {
			return transparency.uses(animation_index);
		}

		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return transparency.getValue(animation_index, tick);
		}
	};

};