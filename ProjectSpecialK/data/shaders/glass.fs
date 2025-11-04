#include "model_generic_top.fs"

	float occlusionVal = mixVal.r;
	float roughnessVal = mixVal.g;
	float specularVal = mixVal.b;
	float fresnelVal = albedoVal.a;

	vec3 norm = Toon ? normalize(Normal) : calcNormal(normalVal);
	vec3 viewPos = ((InvView - model) * vec4(0.0, 0.0, 0.0, 0.1)).xyz;
	vec3 viewDir = normalize(viewPos - FragPos);
	float fresnel = getFresnel(model, norm);
	
	opacityVal = mix(opacityVal, 1.0, fresnel);

	albedoVal.rgb += vec3(fresnelVal * fresnel);

	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(Lights[i], albedoVal.rgb, norm, viewDir, specularVal);
	fragColor = vec4(result, opacityVal);

	if(fragColor.a < OPACITY_CUTOFF) discard;
}
