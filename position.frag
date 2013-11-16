#version 330

smooth in vec2 tex_coord;

uniform sampler2D pos;
uniform sampler2D vel;

uniform float dt;

out vec4 fragColor;

void main()
{
    fragColor = vec4(
            texture2D(pos, tex_coord).xyz +
            texture2D(vel, tex_coord).xyz * dt, 0);
}
