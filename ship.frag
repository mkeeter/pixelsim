#version 330

flat in vec4 color_out;

out vec4 fragColor;

uniform int boost;

void main()
{
    if (color_out.r == 1.0 && color_out.g == 0 && color_out.b == 0)
    {
        fragColor = (boost == 0) ? vec4(0.0f) : color_out;
    }
    else
    {
        fragColor = color_out;
    }
}
