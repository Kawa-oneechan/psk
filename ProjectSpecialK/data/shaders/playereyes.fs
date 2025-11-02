#include "model_generic_top.fs"

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	float eyeBlendVal = albedoVal.a;
	const float fresnelVal = 1.0;

	//albedoVal.rgb = mix(PlayerSkinEdge.rgb, PlayerSkin.rgb, albedoVal.r);
	vec3 skin = pow(PlayerSkin.rgb, vec3(1.0 / 2.01));
	vec3 eyes = pow(PlayerEyes.rgb, vec3(1.0 / 2.01));

	vec3 a = mix(albedoVal.rgb, eyes, eyeBlendVal);
	vec3 b = mix(a, skin, blendVal);
	albedoVal.rgb = b;

#include "model_generic_bottom.fs"
