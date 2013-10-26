#version 120

#define M_PI 3.14159265

attribute vec2  vertex_position;

attribute vec3  color_in;
varying   vec3  color_out;

uniform  ivec2  window_size;
uniform  ivec2  ship_size;

uniform sampler2D pos;
uniform sampler2D filled;

void main()
{
    color_out = color_in;

    float x = 0.0f, y = 0.0f;
    float count = 0.0f;

    for (int i=0; i < 4; ++i) {
        // Each vertex is shared by four pixels.  We'll look at
        // each pixel and find an average position for the vertex.
        float x0, y0;
        if (i == 1 || i == 2) {
            x0 = ceil(vertex_position.x);
        } else {
            x0 = floor(vertex_position.x);
        }

        if (i < 2) {
            y0 = floor(vertex_position.y);
        } else {
            y0 = ceil(vertex_position.y);
        }

        // Texture sampling position
        vec2 xy = vec2((2.0f*x0 + 1.0f) / float(2.0f*ship_size.x),
                       (2.0f*y0 + 1.0f) / float(2.0f*ship_size.y));

        // Sample the position texture at the desired point to see if
        // there's a pixel there that we care about.
        if (xy.x >= 0 && xy.x <= 1 && xy.y >= 0 && xy.y <= 1 &&
            texture2D(filled, xy).r != 0)
        {
            // Get position and rotation data from the texture
            vec4 T = texture2D(pos, xy);
            float a0 = i*M_PI/2.0f + M_PI/4.0f + T.b;

            // Acumulate an average position
            x += T.r + sqrt(0.5f)*cos(a0);
            y += T.g + sqrt(0.5f)*sin(a0);

            count++;
        }
    }
    // Normalize the averaged values
    x /= count;
    y /= count;

    vec2 centered = vec2(x - (ship_size.x - 1)/2.0f,
                         y - (ship_size.y - 1)/2.0f);

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
