#version 120

attribute vec2 vertex_position;
varying vec2 tex_coord;

// Expects to get a rectangle from -1, -1 to 1, 1
// Outputs a normalized texture coordinate
void main()
{
    gl_Position = vec4(vertex_position, 0.0f, 1.0f);

    tex_coord = vec2((vertex_position.x+1.0f)/2.0f,
                     (vertex_position.y+1.0f)/2.0f);
}
