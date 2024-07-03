#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;  

uniform sampler2D ourTexture;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

void main()
{
	vec4 objectColor = texture(ourTexture, TexCoord);

	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;
  	
	// diffuse 
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = vec3(0); //specularStrength * spec * lightColor;  
		
	vec3 result = (ambient + diffuse + specular) * objectColor.rgb;
	FragColor = vec4(result, objectColor.a);

	//FragColor = texture(ourTexture, TexCoord);
	if(FragColor.a < 0.1) discard;
}
