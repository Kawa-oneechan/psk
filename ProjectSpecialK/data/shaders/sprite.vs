#ifndef PULLEDPORK
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
#endif

out vec2 TexCoords;
flat out int index;

//Match this to SpriteRenderer.h
const int BatchSize = 200;

uniform mat4 model[BatchSize];
uniform mat4 projection;

void main()
{
	index = gl_InstanceID;
#ifndef PULLEDPORK
	TexCoords = vertex.zw;
	gl_Position = projection * model[gl_InstanceID] * vec4(vertex.xy, 0.0, 1.0);
#else
	const int indices[6] = int[6](2, 1, 0, 2, 3, 1);
	int index = indices[gl_VertexID];
	vec2 vx = vec2((index << 1) & 2, index & 2) * 0.5;
	gl_Position = projection * model[gl_InstanceID] * vec4(vx, 0.0, 1.0);
	TexCoords = vx * vec2(1.0, -1.0);
#endif
	gl_Position.z = gl_Position.w;
}
