#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 FragPos;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D mixTexture;

uniform vec3 viewPos; 

//lighting
#define NUMLIGHTS 4
struct light {
	vec3 pos;
	vec3 color;
	float strength;
};
uniform light lights[NUMLIGHTS];

//uniform float ambientStrength;
//uniform vec3 lightPos; 
//uniform vec3 lightColor;

const float specPower = 4.0; //32.0;

vec3 ambientLight(vec3 litCol, float strength)
{
	return strength * litCol;
}

vec3 diffuseLight(vec3 normal, vec3 litPos, vec3 litCol)
{
	vec3 litDir = normalize(litPos - FragPos);
	float diff = max(dot(normal, litDir), 0.0);
	return diff * litCol;
}

vec3 specularLight(vec3 normal, vec3 viewDir, vec3 litPos, vec3 litCol, float strength)
{
	vec3 litDir = normalize(litPos - FragPos);
	vec3 reflectDir = reflect(-litDir, normal);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specPower);
	return strength * spec * litCol;
}

vec3 getLight(light l, vec3 albedo, vec3 norm, vec3 viewDir, float spec)
{
	if (l.strength < 0.1) return vec3(0);

	vec3 ambient = ambientLight(l.color, l.strength);
	vec3 diffuse = diffuseLight(norm, l.pos, l.color);
	vec3 specular = specularLight(norm, viewDir, l.pos, l.color, spec);
	return (ambient + diffuse + specular) * albedo;
}

vec3 calcNormal(vec3 mapCol)
{
	vec3 n = normalize(Normal);
	vec3 t = normalize(Tangent);
	t = normalize(t - dot(t, n) * n);
	vec3 b = cross(t, n);

	mapCol = 2.0 * mapCol - 1.0;
	
	vec3 newNormal;
	mat3 tbn = mat3(t, b, n);
	newNormal = tbn * mapCol;
	newNormal = normalize(newNormal);
	return newNormal;
}

void main()
{
	vec4 albedo = texture(albedoTexture, TexCoord);
	vec3 normal = texture(normalTexture, TexCoord).rgb;
	vec4 mix = texture(mixTexture, TexCoord);

	//normal = normal * 2.0 - 1.0;
	//vec3 norm = normalize(Normal);
	vec3 norm = calcNormal(normal);

	//vec3 ambient = ambientLight(lightColor);
	vec3 viewDir = normalize(viewPos - FragPos);

	//vec3 diffuse = diffuseLight(norm, lightPos, lightColor);
	//vec3 specular = vec3(0); //specularLight(norm, viewDir, lightPos, lightColor, mix.g);
	//vec3 result = (ambient + diffuse + specular) * albedo.rgb;
	vec3 result;
	for (int i = 0; i < NUMLIGHTS; i++)
		result += getLight(lights[i], albedo.rgb, norm, viewDir, 0.2);
	FragColor = vec4(result, albedo.a);

	//FragColor = texture(albedoTexture, TexCoord);
	//FragColor = vec4(norm, 1.0);
	//FragColor = mix;
	if(FragColor.a < 0.1) discard;
}
