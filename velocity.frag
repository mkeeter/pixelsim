#version 120

varying vec2 tex_coord;

uniform sampler2D vel;
uniform sampler2D accel;

uniform float dt;

void main()
{
    gl_FragColor = vec4(
            texture2D(vel, tex_coord).xyz +
            texture2D(accel, tex_coord).xyz * dt, 0);
}
