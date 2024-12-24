#include "../../stdafx.h"
#include "Particles.h"

namespace core {

	void CalcSpreadMatrix(float Spread1, float Spread2, float w, float l)
	{
		int i, j;
		float a[2], c[2], s[2];
		Matrix	Temp;

		SpreadMat.unit();

		a[0] = Random::between(-Spread1, Spread1) / 2.0f;
		a[1] = Random::between(-Spread2, Spread2) / 2.0f;

		/*SpreadMat.m[0][0]*=l;
		SpreadMat.m[1][1]*=l;
		SpreadMat.m[2][2]*=w;*/

		for (i = 0; i < 2; i++)
		{
			c[i] = cos(a[i]);
			s[i] = sin(a[i]);
		}
		Temp.unit();
		Temp.m[1][1] = c[0];
		Temp.m[2][1] = s[0];
		Temp.m[2][2] = c[0];
		Temp.m[1][2] = -s[0];

		SpreadMat = SpreadMat * Temp;

		Temp.unit();
		Temp.m[0][0] = c[1];
		Temp.m[1][0] = s[1];
		Temp.m[1][1] = c[1];
		Temp.m[0][1] = -s[1];

		SpreadMat = SpreadMat * Temp;

		float Size = abs(c[0]) * l + abs(s[0]) * w;
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
				SpreadMat.m[i][j] *= Size;
	}

	ModelParticleEmitterAdaptor::Particle ParticleFactory::plane(ModelParticleEmitterAdaptor* emitter,
		size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones,
		Args args) 
	{
		ModelParticleEmitterAdaptor::Particle p;

		const uint32_t emitter_flags = emitter->getFlags();

		//Spread Calculation
		Matrix mrot;

		const auto* bone = allbones[emitter->getBone()];
		const auto parentBoneId = bone->getParentBoneId();

		CalcSpreadMatrix(args.spr, args.spr, 1.0f, 1.0f);
		mrot = bone->getMRot() * SpreadMat;


		if (emitter_flags == 1041) { // Trans Halo
			p.position = bone->getMat() * (emitter->getPosition() + Vector3(Random::between(-args.l, args.l), 0, Random::between(-args.w, args.w)));

			const float t = Random::between(0.0f, float(2 * PI));

			p.position = Vector3(0.0f, emitter->getPosition().y + 0.15f, emitter->getPosition().z) + Vector3(cos(t) / 8, 0.0f, sin(t) / 8); // Need to manually correct for the halo - why?
			assert(!isnan(p.position.x));

			// var isn't being used, which is set to 1.0f,  whats the importance of this?
			// why does this set of values differ from other particles

			Vector3 dir(0.0f, 1.0f, 0.0f);
			p.dir = dir;
			p.speed = dir.normalize() * args.spd * Random::between(0.0f, args.var);
			assert(!isnan(p.speed.x));
		}
		else if (emitter_flags == 25 && parentBoneId < 1) { // Weapon Flame
			assert(false);
			//	p.pos = sys->parent->pivot * (sys->pos + Vector3(randfloat(-l, l), randfloat(-l, l), randfloat(-w, w)));
			//	Vector3 dir = mrot * Vector3(0.0f, 1.0f, 0.0f);
			//	p.dir = dir.normalize();
			//	//Vec3D dir = sys->model->bones[sys->parent->parent].mrot * sys->parent->mrot * Vec3D(0.0f, 1.0f, 0.0f);
			//	//p.speed = dir.normalize() * spd;

		}
		else if (emitter_flags == 25 && parentBoneId > 0) { // Weapon with built-in Flame (Avenger lightsaber!)
			assert(false);
			//	p.pos = sys->parent->mat * (sys->pos + Vector3(randfloat(-l, l), randfloat(-l, l), randfloat(-w, w)));
			//	Vector3 dir = Vector3(sys->parent->mat.m[1][0], sys->parent->mat.m[1][1], sys->parent->mat.m[1][2]) * Vec3D(0.0f, 1.0f, 0.0f);
			//	p.speed = dir.normalize() * spd * randfloat(0, var * 2);

		}
		else if (emitter_flags == 17 && parentBoneId < 1) { // Weapon Glow
			assert(false);
			//	p.pos = sys->parent->pivot * (sys->pos + Vector3(randfloat(-l, l), randfloat(-l, l), randfloat(-w, w)));
			//	Vector3 dir = mrot * Vector3(0, 1, 0);
			//	p.dir = dir.normalize();

		}
		else {
			p.position = emitter->getPosition() + Vector3(Random::between(-args.l, args.l), 0, Random::between(-args.w, args.w));
			
			assert(!isnan(p.position.x));

			//Vec3D dir = mrot * Vec3D(0,1,0);
			Vector3 dir = bone->getMRot() * Vector3(0, 1, 0);
			p.dir = dir;//.normalize();
			p.down = Vector3(0, -1.0f, 0); // dir * -1.0f;
			const auto randf_result = Random::between(-args.var, args.var);
			p.speed = dir.normalize() * args.spd * (1.0f + randf_result);
			assert(!isnan(p.speed.x));
		}


		if (!emitter->isBillboard()) {
			p.corners[0] = mrot * Vector3(-1, 0, +1);
			p.corners[1] = mrot * Vector3(+1, 0, +1);
			p.corners[2] = mrot * Vector3(+1, 0, -1);
			p.corners[3] = mrot * Vector3(-1, 0, -1);
		}

		p.life = 0;
		size_t l_anim = animation_index;
		//if (bZeroParticle)	//TODO
			//l_anim = 0;	//TODO check zero particle logic elsewhere too
		l_anim = 0; //TODO DEBUG REMOVE!

		p.maxlife = emitter->lifespanValue(l_anim, tick);
		if (p.maxlife == 0)
			p.maxlife = 1;

		p.origin = p.position;

		const auto tex_dimension = emitter->getTextureDimension();
		p.tile = Random::between(0, std::max(1, (int)tex_dimension[0]) * std::max(1, (int)tex_dimension[1]) - 1);

		return p;
	}

	ModelParticleEmitterAdaptor::Particle ParticleFactory::sphere(ModelParticleEmitterAdaptor* emitter,
		size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones,
		Args args) 
	{
		ModelParticleEmitterAdaptor::Particle p;

		const uint32_t emitter_flags = emitter->getFlags();

		const auto* bone = allbones[emitter->getBone()];

		Vector3 dir;
		float radius;

		radius = Random::between(0.0f, 1.0f);

		// Old method
		//float t = randfloat(0,2*PI);

		// New
		// Spread should never be zero for sphere particles ?
		float t = 0;
		if (args.spr == 0) {
			t = Random::between((float)-PI, (float)PI);
		}
		else {
			t = Random::between(-args.spr, args.spr);
		}

		//Spread Calculation
		Matrix mrot;

		CalcSpreadMatrix(args.spr * 2, args.spr2 * 2, args.w, args.l);
		mrot = bone->getMRot() * SpreadMat;


		if (emitter_flags == 57 || emitter_flags == 313) { // Faith Halo
			Vector3 bdir(args.w * cosf(t) * 1.6, 0.0f, args.l * sinf(t) * 1.6);

			p.position = emitter->getPosition() + bdir;
			p.tpos = bone->getMat() * p.position;
			assert(!isnan(p.position.x));

			if (bdir.lengthSquared() == 0)
				p.speed = Vector3(0, 0, 0);
			else {
				dir = bone->getMRot() * (bdir.normalize());//mrot * Vec3D(0, 1.0f,0);
				p.speed = dir.normalize() * args.spd * (1.0f + Random::between(-args.var, args.var));   // ?
				assert(!isnan(p.speed.x));
			}

		}
		else {
			Vector3 bdir;
			float temp;

			bdir = mrot * Vector3(0, 1, 0) * radius;
			temp = bdir.z;
			bdir.z = bdir.y;
			bdir.y = temp;

			p.position = emitter->getPosition() + bdir;
			p.tpos = bone->getMat() * emitter->getPosition() + bdir;
			assert(!isnan(p.position.x));


			//p.pos = sys->pos + bdir;
			//p.pos = sys->parent->mat * p.pos;


			if ((bdir.lengthSquared() == 0) && ((emitter_flags & 0x100) != 0x100))
			{
				p.speed = Vector3(0, 0, 0);
				dir = bone->getMRot() * Vector3(0, 1, 0);
			}
			else {
				if (emitter_flags & 0x100)
					dir = bone->getMRot() * Vector3(0, 1, 0);
				else
					dir = bdir.normalize();

				p.speed = dir.normalize() * args.spd * (1.0f + Random::between(-args.var, args.var));   // ?
				assert(!isnan(p.speed.x));
			}
		}

		p.dir = dir.normalize();//mrot * Vec3D(0, 1.0f,0);
		p.down = Vector3(0, -1.0f, 0);

		p.life = 0;
		size_t l_anim = animation_index;
		//if (bZeroParticle)	//TODO
			//l_anim = 0;

		l_anim = 0; //TODO DEBUG REMOVE

		p.maxlife = emitter->lifespanValue(l_anim, tick);
		if (p.maxlife == 0)
			p.maxlife = 1;

		p.origin = p.position;

		const auto tex_dimension = emitter->getTextureDimension();
		p.tile = Random::between(0, std::max(1, (int)tex_dimension[0]) * std::max(1, (int)tex_dimension[1]) - 1);

		return p;
	}

}