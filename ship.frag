#version 330

flat in vec4 color_out;

uniform int thrustEnginesOn;
uniform int leftEnginesOn;
uniform int rightEnginesOn;

out vec4 fragColor;

void main()
{
    if (color_out.r == 1.0f && color_out.g == 0.0f && (
            (color_out.b == 0.0f / 255 && thrustEnginesOn == 0) ||
            (color_out.b == 1.0f / 255 && rightEnginesOn == 0) ||
            (color_out.b == 2.0f / 255 && leftEnginesOn == 0)))
    {
        fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        fragColor = color_out;
    }
}
