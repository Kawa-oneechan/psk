#include "model_generic_top.fs"

	float blendVal = albedoVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 0.5;

	//TODO: get a "sockless" texture with zero alpha maybe.
	albedoVal.rgb = mix(PlayerSkin.rgb, albedoVal.rgb, blendVal);
	//No need to worry about the normals.

#include "model_generic_bottom.fs"
