#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <utility>
#include "../game/GameConstants.h"

namespace core {

    class Model;
    class M2Model;
    class GeosetModifier;
    class GeosetTransform;

    class GeosetState {
        public:
        void init(const M2Model* _model, bool default_vis = true);

        bool has(CharacterGeosets geoset) const;
        bool visible(CharacterGeosets geoset) const;
        bool indexVisible(uint32_t index) const;
        void setVisibility(CharacterGeosets geoset, uint32_t flags, bool relative = true);
        void clearVisibility(CharacterGeosets geoset);

        void each(auto clb) {
            std::for_each(visibleGeosets.begin(), visibleGeosets.end(), clb);
        }

        private:
        std::vector<std::pair<uint32_t, bool>> visibleGeosets;	// vector index corrisponds to getGeosets index.

        friend class GeosetTransform;
    };

    enum class GeosetModiferPriority : int32_t {
        Base = 0,
        ModelInit = 1000,
        CharCustomsEarly = 2000,
        CharCustomsLate = 3000,
        CharCustomsOverride = 4000,
        CharEquip = 5000,
        ForcedOverride = 10000
    };

    class GeosetModifier : public std::enable_shared_from_this<GeosetModifier> {
    public:
        GeosetModifier(const Model* _model = nullptr) : model(_model) {}
        virtual void operator()(GeosetState& state) = 0;
        virtual int32_t priority() const = 0;
        virtual ~GeosetModifier() = default;

    protected:
        const Model* model;
    };

    class GeosetTransform {
    public:
        void apply(GeosetState& state);

        template<typename T> 
        std::shared_ptr<T> getModifier() const 
        { 
            for (const auto& modifier : modifiers) { 
                auto derived = std::dynamic_pointer_cast<T>(modifier); 
                if (derived) { 
                    return derived; 
                } 
            } 
            return nullptr;
        }

        template<typename T>
        std::shared_ptr<T> getOrMakeModifier(auto make)
        {
            std::shared_ptr<T> mod = getModifier<T>();

            if (mod == nullptr) {
                mod = make();
                addModifier(mod);
            }

            return mod;
        }

        template<typename T>
        void addModifier(std::shared_ptr<T> modifier)
        {
            modifiers.push_back(modifier);

            std::sort(modifiers.begin(), modifiers.end(), [](const auto& l, const auto& r) {
                return l->priority() < r->priority();
            });

            assert(modifiers.size() < 10);
        }

    protected:
        std::vector<std::shared_ptr<GeosetModifier>> modifiers;
    };
    

}