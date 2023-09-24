#pragma once
#include <cstdint>
#include <QString>
#include "../utility/Vector3.h"
#include "../utility/Quaternion.h"
#include <set>
#include "../database/GameDatabase.h"
#include "../database/WOTLKGameDatabase.h"
#include "../database/GameDatasetAdaptors.h"
#include "WOTLKAnimation.h"
#include "../utility/Matrix.h"
#include "ModelAdaptors.h"
#include "Particles.h"
#include "GenericModelAdaptors.h"
#include "Texture.h"

namespace core {

	class WOTLKBone : public  ModelBoneAdaptor {
	public:
		WOTLKBone() = default;
		WOTLKBone(WOTLKBone&&) = default;
		virtual ~WOTLKBone() {}

		WOTLKAnimated<Vector3> translation;
		WOTLKAnimated<Quaternion, PACK_QUATERNION, Quat16ToQuat32> rotation; //TODO see old source (two implementations), not sure which is correct for wotlk
		WOTLKAnimated<Vector3> scale;

		Vector3 pivot;
		Vector3 translationPivot;

		bool billboard;
		//TODO better names.
		Matrix mat;
		Matrix mrot;

		WOTLKModelBoneM2 boneDefinition;

		bool calculated;

		virtual Matrix getMat() const {
			return mat;
		}

		virtual Matrix getMRot() const {
			return mrot;
		}

		virtual Vector3 getTranslationPivot() const {
			return translationPivot;
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

	class WOTLKModelColor : public ModelColorAdaptor {
	public:
		WOTLKModelColor() = default;
		WOTLKModelColor(WOTLKModelColor&&) = default;
		virtual ~WOTLKModelColor() {}

		WOTLKAnimated<Vector3> color;
		WOTLKAnimated<float, int16_t, ShortToFloat> opacity;

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

	class WOTLKModelTransparency : public ModelTransparencyAdaptor {
	public:
		WOTLKModelTransparency() = default;
		WOTLKModelTransparency(WOTLKModelTransparency&&) = default;
		virtual ~WOTLKModelTransparency() {}

		WOTLKAnimated<float, int16_t, ShortToFloat> transparency;

		virtual bool transparencyUses(size_t animation_index) const {
			return transparency.uses(animation_index);
		}

		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return transparency.getValue(animation_index, tick);
		}
	};

	class WOTLKModelRibbonEmitter : public ModelRibbonEmitterAdaptor {
	public:

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			const auto* parent_bone = allbones[definition.boneIndex];

			//TODO tidy code, better names, better logic

			Vector3 ntpos = parent_bone->getMat() * pos;
			Vector3 ntup = parent_bone->getMat() * (pos + Vector3(0, 0, 1));
			ntup -= ntpos;
			ntup.normalize();
			float dlen = (ntpos - tpos).length();

			// move first segment
			RibbonSegment& first = *segments.begin();
			if (first.len > definition.edgeLifetime) {
				// add new segment
				first.back = (tpos - ntpos).normalize();
				first.len0 = first.len;
				RibbonSegment newseg;
				newseg.position = ntpos;
				newseg.up = ntup;
				newseg.len = dlen;
				segments.push_front(newseg);
			}
			else {
				first.up = ntup;
				first.position = ntpos;
				first.len += dlen;
			}

			// kill stuff from the end
			float l = 0;
			bool erasemode = false;
			for (std::list<RibbonSegment>::iterator it = segments.begin(); it != segments.end(); ) {
				if (!erasemode) {
					l += it->len;
					if (l > length) {
						it->len = l - length;
						erasemode = true;
					}
					++it;
				}
				else {
					segments.erase(it++);
				}
			}

			tpos = ntpos;
			tcolor = Vector4(color.getValue(animation_index, tick), opacity.getValue(animation_index, tick));
			tabove = above.getValue(animation_index, tick);
			tbelow = below.getValue(animation_index, tick);
		}

		virtual const std::vector<uint16_t> getTexture() const {
			return textures;
		}

		virtual Vector4 getTColor() const {
			return tcolor;
		}

		virtual float getLength() const {
			return length;
		}

		virtual float getTAbove() const {
			return tabove;
		}

		virtual float getTBelow() const {
			return tbelow;
		}

		virtual const std::list<ModelRibbonEmitterAdaptor::RibbonSegment>& getSegments() const {
			return segments;
		}

		WOTLKModelRibbonEmitterM2 definition;

		WOTLKAnimated<Vector3> color;
		WOTLKAnimated<float, int16_t, ShortToFloat> opacity;
		WOTLKAnimated<float> above;
		WOTLKAnimated<float> below;

		Vector3 pos;
		Vector3 tpos; //TODO better name
		Vector4 tcolor;
		int32_t numberOfSegments;
		float length;

		float tabove, tbelow;

		std::vector<uint16_t> textures;


		std::list<ModelRibbonEmitterAdaptor::RibbonSegment> segments;
	};


	class WOTLKModelParticleEmitter : public GenericModelParticleEmitterAdaptor<WOTLKModelParticleEmitterM2> {
	public:

		WOTLKModelParticleEmitter() = default;
		WOTLKModelParticleEmitter(WOTLKModelParticleEmitter&&) = default;
		virtual ~WOTLKModelParticleEmitter() {}

		//TODO better names
		WOTLKAnimated<float> speed;
		WOTLKAnimated<float> variation;
		WOTLKAnimated<float> spread;
		WOTLKAnimated<float> lat;
		WOTLKAnimated<float> gravity;
		WOTLKAnimated<float> lifespan;
		WOTLKAnimated<float> rate;
		WOTLKAnimated<float> areal;
		WOTLKAnimated<float> areaw;
		WOTLKAnimated<float> deacceleration;
		WOTLKAnimated<float> enabled;

		Vector4 colors[3];	//TODO why fixed size?
		float sizes[3];

		float rem;

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			const float deltat = tick.deltaTime / 1000.0f;

			size_t l_manim = animation_index;
			/*if (bZeroParticle)	//TODO what is zero particle?
			* l_manim = 0;
				;*/

			l_manim = 0;	//TODO DEBUG REMOVE - TESTING ONLY - SHOULD USE CONDITION ABOVE

			float grav = gravity.getValue(animation_index, tick);
			float deaccel = deacceleration.getValue(animation_index, tick);

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
						en = enabled.getValue(animation_index,tick) != 0;

					//rem = 0;
					if (en) {
						for (size_t i = 0; i < tospawn; i++) {
							Particle p = generator(this, animation_index, tick, allbones, w, l, spd, var, spr, spr2);
							// sanity check:
							if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
								particles.push_back(p);
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
			return { definition.texture };
		}

		constexpr virtual uint8_t getParticleType() const {
			return definition.particleType;
		}

		virtual float lifespanValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return lifespan.getValue(animation_index, tick);
		}

	};




}