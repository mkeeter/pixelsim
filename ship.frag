#version 330

flat in vec4 color_out;

out vec4 fragColor;

void main()
{
    fragColor = color_out;
}
