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
	int tmp = int(abs(mod(Texcoord.x * 160 * 7, 7) - 3));
	int y = int(abs(mod(Texcoord.y * 144 * 7, 7) - 3));
	int x = max(tmp, y);
	y = min(tmp, y);
	float alpha = 1;
	if (x == 2 && y == 2) {
		alpha = 0.959;
	} else if (x == 3) {
		switch (y) {
			case 1:
				alpha = 0.893;
				break;
			case 2:
				alpha = 0.793;
				break;
			case 3:
				alpha = 0.529;
				break;
		}
	}
	out_colour = vec4(mix(background, foreground, (1.0 - src) * alpha), 1.0);
}
