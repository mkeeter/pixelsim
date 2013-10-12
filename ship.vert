#version 120

attribute vec2  vertex_position;

attribute vec3  color_in;
varying   vec3  color_out;

uniform  ivec2  window_size;
uniform  ivec2  ship_size;

void main()
{
    vec2 centered = vec2(vertex_position.x - (ship_size.x - 1)/2.0f,
                         (ship_size.y - 1)/2.0f - vertex_position.y);


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

    color_out = color_in;
}
