#version 330

flat in vec4 color_out;

uniform int thrustEnginesOn;
uniform int leftEnginesOn;
uniform int rightEnginesOn;

out vec4 fragColor;

void main()
{
    if ((color_out.r == SHIP_ENGINE_THRUST_R/255.0f &&
         color_out.g == SHIP_ENGINE_THRUST_G/255.0f &&
         color_out.b == SHIP_ENGINE_THRUST_B/255.0f && thrustEnginesOn == 0) ||
        (color_out.r == SHIP_ENGINE_LEFT_R/255.0f &&
         color_out.g == SHIP_ENGINE_LEFT_G/255.0f &&
         color_out.b == SHIP_ENGINE_LEFT_B/255.0f && leftEnginesOn == 0) ||
        (color_out.r == SHIP_ENGINE_RIGHT_R/255.0f &&
         color_out.g == SHIP_ENGINE_RIGHT_G/255.0f &&
         color_out.b == SHIP_ENGINE_RIGHT_B/255.0f && rightEnginesOn == 0))
    {
        discard;
    }
    else
    {
        fragColor = color_out;
    }
}
