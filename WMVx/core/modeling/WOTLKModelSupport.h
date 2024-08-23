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

		virtual const Vector4& getTColor() const {
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

		StandardAnimated<Vector3> color;
		StandardAnimated<float, int16_t, ShortToFloat> opacity;
		StandardAnimated<float> above;
		StandardAnimated<float> below;

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
		StandardAnimated<float> speed;
		StandardAnimated<float> variation;
		StandardAnimated<float> spread;
		StandardAnimated<float> lat;
		StandardAnimated<float> gravity;
		StandardAnimated<float> lifespan;
		StandardAnimated<float> rate;
		StandardAnimated<float> areal;
		StandardAnimated<float> areaw;
		StandardAnimated<float> deacceleration;
		StandardAnimated<float> enabled;

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
							Particle p = generator(
								this, 
								animation_index, 
								tick, 
								allbones, 
								{ w, l, spd, var, spr, spr2 }
							);
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