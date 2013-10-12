#version 120

varying vec3 color_out;

void main()
{
    gl_FragColor = vec4(color_out, 1.0);
}
