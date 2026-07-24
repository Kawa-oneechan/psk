#include "model_generic_top.fs"

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 0.5;
	
	albedoVal.rgb = mix(PlayerHairHi.rgb, PlayerHair.rgb, albedoVal.r);
	albedoVal.rgb = pow(albedoVal.rgb, vec3(1.0 / 2.01));
	albedoVal.rgb = mix(albedoVal.rgb, albedoVal.rgb * 2.0, mixVal.b);

#include "model_generic_bottom.fs"
