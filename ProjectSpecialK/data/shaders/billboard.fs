#include "model_generic_top.fs"

	float occlusionVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 0.0;

#include "model_generic_bottom.fs"
