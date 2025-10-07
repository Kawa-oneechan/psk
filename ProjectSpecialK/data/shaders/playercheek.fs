#include "model_generic_top.fs"

	float blendVal = albedoVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 1.0;

	albedoVal.rgb = mix(PlayerCheeks.rgb, PlayerSkin.rgb, blendVal);

#include "model_generic_bottom.fs"
