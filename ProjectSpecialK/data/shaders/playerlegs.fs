#include "model_generic_top.fs"

	float blendVal = albedoVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	const float fresnelVal = 1.0;

	//TODO: get a "sockless" texture with zero alpha maybe.
	//albedoVal.rgb = mix(PlayerSkinEdge.rgb, PlayerSkin.rgb, albedoVal.r);
	albedoVal.rgb = PlayerSkin.rgb;
	albedoVal.rgb = pow(albedoVal.rgb, vec3(1.0 / 2.01));
	//No need to worry about the normals.

#include "model_generic_bottom.fs"
