/*
* Handles clipping the sprite texture while affecting it's alpha value
*
* This shader is applied to sprites, it does not handle color mixing with the
* sprite's fill color since for this current implementation, sprites remain with
* a white fill color.
*
* texture: Takes the current texture of the shape
* size: The dimensions of the sprite (no transform)
* radius: radius value of the rounded corners
* opacity: Transparency ratio between 0 & 1, scales within the alpha channel value of the texture itself
*
*/

uniform sampler2D texture;
uniform vec2 size;
uniform float radius;
uniform float opacity;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 pos = uv * size;

    vec2 q = abs(pos - size * 0.5) - (size * 0.5 - vec2(radius));
    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - radius;

    if(dist > 0.0) discard;

    vec4 texPixel = texture2D(texture, uv);

    gl_FragColor = vec4(texPixel.rgb, texPixel.a * opacity);
}