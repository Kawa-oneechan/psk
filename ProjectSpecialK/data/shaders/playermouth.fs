#include "model_generic_top.fs"

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 0.5;

	//vec3 edge = PlayerSkin.rgb * 0.8;
	vec3 skin = pow(PlayerSkin.rgb, vec3(1.0 / 2.01));
	albedoVal.rgb = mix(albedoVal.rgb, skin, blendVal);

#include "model_generic_bottom.fs"
