#pragma once
#include <cstdint>
#include <QString>
#include "../utility/Vector3.h"
#include "../utility/Quaternion.h"
#include <set>
#include "../database/GameDatabase.h"
#include "../database/BFAGameDatabase.h"
#include "../database/GameDatasetAdaptors.h"
#include "BFAAnimation.h"
#include "../utility/Matrix.h"
#include "ModelAdaptors.h"
#include "Particles.h"
#include "GenericModelAdaptors.h"
#include "Texture.h"


namespace core {
	class BFABone : public  ModelBoneAdaptor {
	public:
		BFABone() = default;
		BFABone(BFABone&&) = default;
		virtual ~BFABone() {}

		BFAAnimated<Vector3> translation;
		BFAAnimated<Quaternion, PACK_QUATERNION, Quat16ToQuat32> rotation; //TODO see old source (two implementations), not sure which is correct for wotlk
		BFAAnimated<Vector3> scale;

		Vector3 pivot;
		Vector3 translationPivot;

		bool billboard;
		//TODO better names
		Matrix mat; 
		Matrix mrot; 

		BFAModelBoneM2 boneDefinition;

		bool calculated;

		virtual const AnimatedValue<Vector3>* getTranslation() const override {
			return &translation;
		}

		virtual const AnimatedValue<Quaternion>* getRotation() const override {
			return &rotation;
		}

		virtual const AnimatedValue<Vector3>* getScale() const override {
			return &scale;
		}

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
					m *= Matrix::newTranslation(translation.getValue(animation_index,tick));
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

	using BFAModelColor = WOTLKModelColor; 
	using BFAModelTransparency = WOTLKModelTransparency;
	using BFAModelRibbonEmitter = WOTLKModelRibbonEmitter;

	class BFAModelParticleEmitter : public GenericModelParticleEmitterAdaptor<BFAModelParticleEmitterM2> {
	public:
		BFAModelParticleEmitter() = default;
		BFAModelParticleEmitter(BFAModelParticleEmitter&&) = default;
		virtual ~BFAModelParticleEmitter() {}

		//TODO better names
		BFAAnimated<float> speed;
		BFAAnimated<float> variation;
		BFAAnimated<float> spread;
		BFAAnimated<float> lat;
		BFAAnimated<float> gravity;
		BFAAnimated<float> lifespan;
		BFAAnimated<float> rate;
		BFAAnimated<float> areal;
		BFAAnimated<float> areaw;
		BFAAnimated<float> deacceleration;
		BFAAnimated<float> enabled;

		Vector4 colors[3];	//TODO why fixed size?
		float sizes[3];

		float rem;


		//TODO CHECK AGAINST WMV BFA CODE!
		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			const float deltat = tick.deltaTime / 1000.0f;

			size_t l_manim = animation_index;
			/*if (bZeroParticle)	//TODO what is zero particle?
			* l_manim = 0;
				;*/

			l_manim = 0;	//TODO DEBUG REMOVE - TESTING ONLY - SHOULD USE CONDITION ABOVE

			float grav = gravity.getValue(animation_index, tick);
			float deaccel = deacceleration.getValue(animation_index, tick);

			//TODO not sure why gravity is sometimes NaN...
			if (isnan(grav)) {
				grav = 0;
			}

			//TODO is condition needed/valid?
			if (generator) {
				float frate = rate.getValue(l_manim, tick);
				float flife = lifespan.getValue(l_manim, tick);
				float ftospawn;
				if (flife)
					ftospawn = (deltat * frate / flife) + rem;
				else
					ftospawn = rem;

				if (ftospawn < 1.0f) {
					rem = ftospawn;
					if (rem < 0)
						rem = 0;
				}
				else {
					unsigned int tospawn = (int)ftospawn;

					if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
						tospawn = (unsigned int)(MAX_PARTICLES - particles.size());

					rem = ftospawn - (float)tospawn;

					float w = areal.getValue(l_manim, tick) * 0.5f;
					float l = areaw.getValue(l_manim, tick) * 0.5f;
					float spd = speed.getValue(l_manim, tick);
					float var = variation.getValue(l_manim, tick);
					float spr = spread.getValue(l_manim, tick);
					float spr2 = lat.getValue(l_manim, tick);
					bool en = true;
					if (enabled.uses(animation_index))
						en = enabled.getValue(animation_index, tick) != 0;

					//rem = 0;
					if (en) {
						for (size_t i = 0; i < tospawn; i++) {
							Particle p = generator(this, animation_index, tick, allbones, w, l, spd, var, spr, spr2);
							// sanity check:
							if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
								particles.push_back(std::move(p));
						}
					}
				}

			}


			float mspeed = 1.0f;

			float mid = 0.5f; //TODO WHERE SHOULD THIS LIVE?

			//TODO tidy loop
			for (auto it = particles.begin(); it != particles.end(); ) {
				auto& particle = *it;
				particle.speed += particle.down * grav * deltat - particle.dir * deaccel * deltat;

				const auto& slowdown = definition.params.drag;
				if (slowdown > 0) {
					mspeed = expf(-1.0f * slowdown * particle.life);
				}

				particle.position += particle.speed * mspeed * deltat;

				assert(!isnan(particle.position.x));

				particle.life += deltat;
				float rlife = particle.life / particle.maxlife;

				//TODO tidy 
				particle.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
				particle.color = lifeRamp<Vector4>(rlife, mid, colors[0], colors[1], colors[2]);

				if (rlife >= 1.0f) {
					particles.erase(it++);
				}
				else {
					++it;
				}
			}
		}

		virtual const std::vector<uint16_t> getTexture() const {
			if (definition.flags & ModelParticleFlags::MULTITEXTURE) {
				return {
					definition.texture.texture_0,
					definition.texture.texture_1,
					definition.texture.texture_2,
				};
			}

			return { definition.texture.texture_0 };
		}

		constexpr virtual uint8_t getParticleType() const {
			return 0;
		}

		virtual float lifespanValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return lifespan.getValue(animation_index, tick);
		}
	};
}