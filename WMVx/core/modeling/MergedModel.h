#pragma once

#include <memory>
#include <vector>
#include "../game/GameConstants.h"
#include "RawModel.h"
#include "ModelSupport.h"
#include "Animator.h"
#include "../utility/Vector3.h"
#include "../game/GameClientAdaptor.h"


namespace core {

	class Model;

	/*
		for DF+ character handling, we sometimes need to 'merge' multiple models into one.
	*/
	class MergedModel : public ModelTextureInfo,  public ModelAnimationInfo, public ModelGeosetInfo
	{
	public:
		enum class Type {
			CHAR_MODEL_ADDITION //DF+ character 'SkinnedModel' customizations.
		};

		using id_t = uint32_t;

		MergedModel(ModelFactory& factory, Model* _owner, Type _type, id_t _id);
		MergedModel(MergedModel&&) = default;
		virtual ~MergedModel() {}

		void initialise(const GameFileUri& uri, GameFileSystem* fs, GameDatabase* db, TextureManager& manager);
		void merge();

		void update(const Animator& animator, const AnimationTickArgs& tick);


		Type getType() const {
			return type;
		}

		id_t getId() const {
			return id;
		}


		std::unique_ptr<RawModel> model;
		Model* owner;

		//this model -> parent model, format
		std::unordered_map<uint16_t, uint16_t> boneMap;


	protected:

		void updateAnimationWithOwner();

	private:

		// type and id are purely for wmvx usage, has no relation to any game file data.
		const Type type;
		const id_t id;
	};

}