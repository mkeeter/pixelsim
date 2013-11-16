#version 120

smooth in vec2 tex_coord;
uniform sampler2D y;

uniform sampler2D k1;
uniform sampler2D k2;
uniform sampler2D k3;
uniform sampler2D k4;

uniform float dt;

out vec4 fragColor;

void main()
{
    fragColor = vec4(
            texture2D(y, tex_coord).xyz + dt/6.0f * (
                texture2D(k1, tex_coord).xyz +
                2*texture2D(k2, tex_coord).xyz +
                2*texture2D(k3, tex_coord).xyz +
                texture2D(k4, tex_coord).xyz), 0.0f);
}
