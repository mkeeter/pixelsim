#version 330

uniform sampler2D state;
uniform sampler2D accel;

uniform float dt;
uniform ivec2 size;

out vec4 fragColor;

void main()
{
    vec2 tex_coord = vec2(gl_FragCoord.x / float(size.x + 1),
                          gl_FragCoord.y / float(size.y + 1));

    fragColor = texture(state, tex_coord) + texture(accel, tex_coord) * dt;
}
