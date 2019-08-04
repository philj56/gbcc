#version 150 core

in vec2 Texcoord;

out vec4 out_colour;

uniform sampler2D tex;

const vec3 background = vec3(99.0 / 255.0, 149.0 / 255.0, 50.0 / 255.0);
const vec3 foreground = vec3(53.0 / 255.0, 99.0 / 255.0, 22.0 / 255.0);
const float maxr = sqrt(18) - 3;

float luma(vec3 col)
{
	return 0.2162 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}

void main()
{
	float src = luma(texture(tex, Texcoord).rgb);
	float x = mod(Texcoord.x * 160 * 7, 7) - 3;
	float y = mod(Texcoord.y * 144 * 7, 7) - 3;
	float weight = sqrt(x * x + y * y) - 3;
	float alpha = pow(1.0 - weight / maxr, 1.0 / 1.6);
	out_colour = vec4(mix(background, foreground, (1.0 - src) * alpha), 1.0);
}
