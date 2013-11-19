#version 330

uniform sampler2D pos;
uniform sampler2D vel;

uniform float dt;
uniform ivec2 size;

out vec4 fragColor;

void main()
{
    vec2 tex_coord = vec2(gl_FragCoord.x / float(size.x + 1),
                          gl_FragCoord.y / float(size.y + 1));

    fragColor = vec4(
            texture(pos, tex_coord).xyz +
            texture(vel, tex_coord).xyz * dt, 0);
}
