#version 120

uniform sampler2D texture;
varying vec2 tex_coord;

void main()
{
    gl_FragColor = texture2D(texture, tex_coord);
}
