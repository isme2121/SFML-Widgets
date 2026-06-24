/*
* Handles clipping a single side of a sprite in a button while also affecting it's transparency
*
* This shader is applied to the button's image sprite since a button with rounded corners requires
* clipping the sprite's edges to blend in the image. It does not handle color mixing with the
* sprite's fill color since for this current implementation, sprites remain with
* a white fill color.
*
* texture: Takes the current texture of the shape
* size: The dimensions of the button (no transform)
* radius: radius value of the rounded corners
* opacity: Transparency ratio between 0 & 1, scales within the alpha channel value of the texture itself
* side: The position of the sprite image relative to the button with 0.0 = left, 1.0 = center, and 2.0 right
*/

uniform sampler2D texture;
uniform vec2 size;
uniform float radius;
uniform float opacity;
uniform float side;

void main()
{
    vec2 uv  = gl_TexCoord[0].xy;
    vec2 pos;

    // left side
    if(side < 0.5)
    {
        pos = uv * (size / vec2(3.0, 1.0));
    }

    // right side
    else if(side > 1.5)
    {
        // right side — image occupies right x/3
        pos = vec2(size.x * (2.0 / 3.0), 0.0) + uv * (size / vec2(3.0, 1.0));
    }

    // center
    else
    {
        // center — image occupies middle x/3
        pos = vec2(size.x * (1.0 / 3.0), 0.0) + uv * (size / vec2(3.0, 1.0));
    }

    // clip
    vec2 h = size * 0.5;
    vec2 q = abs(pos - h) - (h - vec2(radius));
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - radius;
    if(dist > 0.0) discard;

    // transparency
    vec4 texPixel = texture2D(texture, uv);
    gl_FragColor = vec4(texPixel.rgb, texPixel.a * opacity);
}