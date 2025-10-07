#include "model_generic_top.fs"

	float occlusionVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	float fresnelVal = albedoVal.a;

#include "model_generic_bottom.fs"
