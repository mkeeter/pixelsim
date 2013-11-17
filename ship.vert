#version 330

layout(location=0) in vec2 vertex_position;
layout(location=1) in vec3 color_in;

flat out vec4 color_out;

uniform  ivec2  window_size;
uniform  ivec2  ship_size;

uniform sampler2D pos;

void main()
{
    color_out = vec4(color_in, 1.0f);

    vec2 xy = texture(pos, vec2(
        (vertex_position.x + 1.0f) / float(ship_size.x + 2),
        (vertex_position.y + 1.0f) / float(ship_size.y + 2))).xy;

    vec2 centered = xy - ship_size/2.0f;

    // If the ship is wider than the window, fit the x axis
    if ((ship_size.x*window_size.y) / (ship_size.y*window_size.x) >= 1)
    {
        centered   *= 2.0f / ship_size.x;
        centered.y *= float(window_size.x) / float(window_size.y);
    }
    else // otherwise, fit the y axis
    {
        centered   *= 2.0f / ship_size.y;
        centered.x *= float(window_size.y) / float(window_size.x);
    }

    gl_Position = vec4(centered*0.9, 0.0, 1.0);
}
