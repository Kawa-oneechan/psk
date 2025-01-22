layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;
flat out int index;

//Match this to SpriteRenderer.h
const int BatchSize = 200;

uniform mat4 model[BatchSize];
uniform mat4 projection;

void main()
{
	index = gl_InstanceID;
	TexCoords = vertex.zw;
	gl_Position = projection * model[gl_InstanceID] * vec4(vertex.xy, 0.0, 1.0);
}
