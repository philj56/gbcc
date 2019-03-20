#version 150 core

in vec2 position;
in vec2 texcoord;

out vec2 Texcoord;

void main()
{
	Texcoord = texcoord;
	Texcoord.y = 1.0 - Texcoord.y;
	gl_Position = vec4(position, 0.0, 1.0);
}
