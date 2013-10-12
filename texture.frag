#version 120

uniform sampler2D texture;
varying vec2 tex_coord;

void main()
{
    gl_FragColor = texture2D(texture, tex_coord);
    //gl_FragColor = vec4(tex_coord.x, tex_coord.y, 0.0f, 1.0f);
}
