/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#version 150 core

in vec2 Texcoord;

out vec4 out_colour;

uniform sampler2D tex;

void main()
{
	vec4 old = texture(tex, Texcoord * vec2(0.5f, 1.0f) + vec2(0.5f, 0.0f));
	vec4 new = texture(tex, Texcoord * vec2(0.5f, 1.0f));
	out_colour = mix(new, old, 0.5);
}
