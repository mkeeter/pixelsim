#version 330

flat in vec3 color_out;

out vec4 fragColor;

void main()
{
    fragColor = vec4(color_out, 1.0);
}
