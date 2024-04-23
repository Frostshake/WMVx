#pragma once

#include <memory>
#include <vector>
#include "../game/GameConstants.h"
#include "RawModel.h"
#include "Animator.h"
#include "ModelSupport.h"
#include "../utility/Vector3.h"


namespace core {

	class Model;
	class GameFileSystem;
	class GameDatabase;

	/*
		for DF+ character handling, we sometimes need to 'merge' multiple models into one.
	*/
	class MergedModel : public ModelTextureInfo,  public ModelAnimationInfo, public ModelGeosetInfo
	{
	public:
		enum class Type {
			CHAR_MODEL_ADDITION, //DF+ character 'SkinnedModel' customizations.
			CHAR_ATTACHMENT_ADDITION //DF+ character attachment models.
		};

		constexpr static float RESOLUTION_FINE		= 0.0001f;
		constexpr static float RESOLUTION_ROUGH		= 0.001f;

		using id_t = uint64_t;

		MergedModel(std::unique_ptr<RawModel> raw_model, Model* _owner, Type _type, id_t _id);
		MergedModel(MergedModel&&) = default;
		virtual ~MergedModel() {}

		void initialise(const GameFileUri& uri, GameFileSystem* fs, GameDatabase* db, TextureManager& manager);
		void merge(float resolution);

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

	
	class HasMergedModels {
	public:
		const std::vector<MergedModel*>& getMerged() const {
			return reinterpret_cast<const std::vector<MergedModel*>&>(merged);
		}

		void addRelation(std::unique_ptr<MergedModel> relation) {
			merged.push_back(std::move(relation));
		}

		void removeRelation(MergedModel::Type type, MergedModel::id_t id) {
			std::erase_if(merged, [type, id](const std::unique_ptr<MergedModel>& rel) -> bool {
				return rel->getType() == type && rel->getId() == id;
			});
		}

		bool relationExists(MergedModel::Type type, MergedModel::id_t id) {
			return std::find_if(merged.begin(), merged.end(), [type, id](const std::unique_ptr<MergedModel>& rel) -> bool {
				return rel->getType() == type && rel->getId() == id;
			}) != merged.end();
		}


	protected:
		std::vector<std::unique_ptr<MergedModel>> merged;
	};
}