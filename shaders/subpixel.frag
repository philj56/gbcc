#version 150 core

in vec2 Texcoord;

out vec4 out_colour;

uniform sampler2D tex;
        
const vec3 r = vec3(
        255.0 / 255.0,
        113.0 / 255.0,
        69.0 / 255.0
);
        
const vec3 g = vec3(
        193.0 / 255.0,
        214.0 / 255.0,
        80.0 / 255.0
);
        
const vec3 b = vec3(
        59.0 / 255.0,
        206.0 / 255.0,
        255.0 / 255.0
);
        
const float radius = 2.0;
const float radius2 = radius * radius;

vec3 circ(vec3 x)
{
        return sqrt(max(radius2 - x * x, 0)) / radius;
}

vec3 square(vec3 x)
{
        return ceil(circ(x));
}

void main()
{
        vec3 src;
        src.r = texture(tex, Texcoord + vec2(1.0 / 480.0, 0)).r;
        src.g = texture(tex, Texcoord).g;
        src.b = texture(tex, Texcoord - vec2(1.0 / 480.0, 0)).b;
        vec3 x = mod(Texcoord.x * 160 * 7 + vec3(3, 1, -1), 7) - 3;
        vec3 weight = circ(x);
        float y = mod(Texcoord.y * 144 * 7, 7);
        vec3 dst = vec3(0);
        dst += src.r * r * weight.r;
        dst += src.g * g * weight.g;
        dst += src.b * b * weight.b;
        float gridline = max(ceil((y - 1) / 6), 0.7);
        out_colour = vec4(gridline * dst, 1.0);
}
