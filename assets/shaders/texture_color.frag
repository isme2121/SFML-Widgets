/*
* Made for rendering texture within a shape
*
* Texture is affected by the color values of the shape pointing at it
* Applying this texture to the shape would minimize the fill color effects on the texture
*
* texture: Takes the current texture of the shape
* bg_color: The current fill color of the shape
* opacity: Transparency ratio between 0 & 1, scales within the alpha channel value of the texture itself
* outline_color: Skips processing the outline, to identify it, fill color must not be equal to the outline color
* has_texture: No need to process if no texture is assigned, 0 for false & 1 for true
*
*/

uniform sampler2D texture;
uniform vec4 bg_color;
uniform float opacity;
uniform vec4 outline_color;
uniform float has_texture;

void main()
{
    vec4 diff = abs(gl_Color - outline_color);
    float epsilon = 0.001;
    bool isOutline = (diff.r + diff.g + diff.b + diff.a) < epsilon;

    // skip if it's an outline
    if(isOutline)
    {
        gl_FragColor = outline_color;
        return;
    }

    // if no texture, just render bg_color directly
    if(has_texture < 0.5)
    {
        gl_FragColor = bg_color;
        return;
    }

    vec2 uv = gl_TexCoord[0].xy;
    vec4 texPixel = texture2D(texture, uv);

    float finalAlpha = texPixel.a * opacity;
    vec4 result = mix(bg_color, texPixel, finalAlpha);
    result.a = max(bg_color.a, finalAlpha);

    gl_FragColor = result;
}