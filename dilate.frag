#version 120

uniform sampler2D texture;
varying vec2 tex_coord;

void main()
{
    gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i=-4; i <= 4; ++i) {
        for (int j=-4; j <= 4; ++j) {
            if (texture2D(texture, tex_coord + vec2(i/200., j/200.)).r != 0) {
                gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            }
        }
    }
}
