#version 120

varying vec2 tex_coord;

uniform sampler2D pos;
uniform sampler2D vel;

uniform float dt;

void main()
{
    gl_FragColor = vec4(
            texture2D(pos, tex_coord).xyz +
            texture2D(vel, tex_coord).xyz * dt, 0);
}
