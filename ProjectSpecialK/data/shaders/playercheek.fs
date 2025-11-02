#include "model_generic_top.fs"

	float blendVal = albedoVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 1.0;

	albedoVal.rgb = mix(PlayerSkinEdge.rgb, PlayerSkin.rgb, albedoVal.r);

	albedoVal.rgb = mix(PlayerCheeks.rgb, albedoVal.rgb, blendVal);

	albedoVal.rgb = pow(albedoVal.rgb, vec3(1.0 / 2.01));

#include "model_generic_bottom.fs"
