#include "../../stdafx.h"
#include "Geoset.h"
#include "M2.h"

static bool geoset_match(core::CharacterGeosets geoset, uint32_t id) {
	const auto id_range_start = geoset * 100;
	const auto id_range_end = (geoset + 1) * 100;
		
	return id_range_start < id && id < id_range_end;
}

void core::GeosetState::init(const M2Model* _model, bool default_vis)
{
	visibleGeosets.reserve(_model->getGeosetAdaptors().size());
	for (const auto& geo : _model->getGeosetAdaptors()) {
		visibleGeosets.emplace_back(geo->getId(), default_vis);
	}
}

bool core::GeosetState::has(CharacterGeosets geoset) const
{
	auto found = std::find_if(visibleGeosets.begin(), visibleGeosets.end(), [geoset](const auto& el) {
		return geoset_match(geoset, el.first);
	});

	return found != visibleGeosets.end();
}

bool core::GeosetState::visible(CharacterGeosets geoset) const
{
	auto found = std::find_if(visibleGeosets.begin(), visibleGeosets.end(), [geoset](const auto& el) {
			return geoset_match(geoset, el.first) && el.second;
		});

	return found != visibleGeosets.end();
}

bool core::GeosetState::indexVisible(uint32_t index) const
{
	return visibleGeosets[index].second;
}

void core::GeosetState::setVisibility(CharacterGeosets geoset, uint32_t flags, bool relative)
{
	//formula for converting a geoset flag into an id
	//depending on the context and usage, xx0 can be the default (hidden/empty) , or xx1 can be default (non-empty?)
	//xx1 id's look to be the default, so +1 gets added to the flags
	const auto geoset_id = (geoset * 100) + flags + (relative ? 1 : 0);

	for (auto& el : visibleGeosets) {
		if (geoset_match(geoset, el.first)) {
			el.second = el.first == geoset_id;
		}
	}
}

void core::GeosetState::clearVisibility(CharacterGeosets geoset)
{
	for (auto& el : visibleGeosets) {
		if (geoset_match(geoset, el.first)) {
			el.second = false;
		}
	}
}

void core::GeosetTransform::apply(GeosetState& state)
{
	for (auto& el : state.visibleGeosets) {
		el.second = false;
	}

	for (auto& mod : modifiers) {
		(*mod)(state);
	}
}
