#version 150 core

in vec2 Texcoord;

out vec4 out_colour;

uniform sampler2D tex;
uniform samplerBuffer lut;
        
void main()
{
        vec4 icol = min(texture(tex, Texcoord) * 32, 31);
        int idx = int(icol.r) * 32 * 32 + int(icol.g) * 32 + int(icol.b);
        out_colour = vec4(texelFetch(lut, idx).rgb, 1);
}
