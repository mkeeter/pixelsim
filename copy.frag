#version 330

smooth in vec2 tex_coord;

uniform sampler2D tex;

out vec4 fragColor;

void main()
{
    fragColor = texture(tex, tex_coord);
}
