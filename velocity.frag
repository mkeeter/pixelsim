#version 330

smooth in vec2 tex_coord;

uniform sampler2D vel;
uniform sampler2D accel;

uniform float dt;

out vec4 fragColor;

void main()
{
    fragColor = vec4(
            texture(vel, tex_coord).xyz +
            texture(accel, tex_coord).xyz * dt, 0);
}
