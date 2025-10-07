#include "model_generic_top.fs"

	float blendVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	float eyeBlendVal = albedoVal.a;
	const float fresnelVal = 1.0;

	vec3 a = mix(albedoVal.rgb, PlayerEyes.rgb, eyeBlendVal);
	vec3 b = mix(a, PlayerSkin.rgb, blendVal);
	albedoVal.rgb = b;

#include "model_generic_bottom.fs"
