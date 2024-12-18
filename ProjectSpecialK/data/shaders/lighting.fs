//---Lighting---

#define LIGHTOFF 0.004

const float specPower = 4.0; //32.0;

vec3 ambientLight(vec3 litCol, float strength)
{
	return strength * litCol;
}

vec3 diffuseLight(vec3 normal, vec3 litPos, vec3 litCol)
{
	vec3 litDir = normalize(litPos - FragPos);
	float diff = max(dot(normal, litDir), 0.0);

	if (Toon)
	{
		//Toon shading for lol
			 if (diff < 0.15) diff = 0.15;
		else if (diff < 0.20) diff = 0.20;
		else if (diff < 0.50) diff = 0.50;
		else if (diff < 0.90) diff = 0.90;
		else             diff = 1.00;
	}

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
	if (l.color.a < LIGHTOFF) return vec3(0);

	vec3 ambient = ambientLight(l.color.rgb, l.color.a);
	vec3 diffuse = diffuseLight(norm, l.pos.xyz, l.color.rgb);
	vec3 specular = specularLight(norm, viewDir, l.pos.xyz, l.color.rgb, spec);
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

//--------------
