// Vertex Shader � file "minimal.vert"

#version 330

uniform mat4 MVP;

in  vec3 in_Position;
in  vec3 in_Color;
out vec3 ex_Color;

void main(void)
{
	ex_Color = in_Color;
	gl_Position = MVP * vec4(in_Position, 1.0);
}