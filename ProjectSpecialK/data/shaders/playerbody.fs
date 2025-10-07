#include "model_generic_top.fs"

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 1.0;
	
	albedoVal.rgb = mix(albedoVal.rgb, PlayerSkin.rgb, blendVal);

#include "model_generic_bottom.fs"
