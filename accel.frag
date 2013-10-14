#version 120

varying vec2 tex_coord;

uniform sampler2D pos;
uniform sampler2D filled;

uniform ivec2 ship_size;
uniform float dt;

uniform float k;     // linear spring constant
uniform float theta; // angular spring constant

// near and far are x,y,a offsets.
vec3 accel(vec3 near, vec3 far)
{
    return vec3((far.xy - near.xy)*k, (far.z - near.z)*theta);
}

void main()
{
    vec3 near = texture2D(pos, tex_coord).xyz;

    vec3 total = vec3(0.0f);

    for (int x=-1; x <= 1; ++x) {
        for (int y=-1; y <= 1; ++y) {
            // Pick an offset that will give us the next pixel
            // in the desired direction.
            float dx = float(x) / float(ship_size.x);
            float dy = float(y) / float(ship_size.y);
            vec2 delta = vec2(dx, dy);

            // If the chosen pixel is within the image (i.e. it has texture
            // coordinates between 0 and 1) and is marked as filled in the
            // pixel occupancy texture, then find and add its acceleration.
            if (tex_coord.x + dx >= 0.0f && tex_coord.x + dx <= 1.0f &&
                tex_coord.y + dy >= 0.0f && tex_coord.y + dy <= 1.0f &&
                texture2D(filled, tex_coord + delta).r != 0)
            {
                vec3 far = texture2D(pos, tex_coord + delta).xyz;
                total += accel(near, far);
            }
        }
    }

    gl_FragColor = vec4(total, 1.0f);
}
