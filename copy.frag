#version 330

smooth in vec2 tex_coord;

uniform sampler2D texture;

out vec4 fragColor;

void main()
{
    fragColor = texture2D(texture, tex_coord);
}
