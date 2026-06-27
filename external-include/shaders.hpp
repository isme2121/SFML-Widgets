#pragma once
#include <string>

/*
* Shaders that were previously available within assets are hard-coded here.
* The purpose is to avoid hard-coded paths entirely and allow the generated
* executable to run freely without having to exist in the same directory layer
* as assets/ by using sf::Shader::loadFromMemory method at runtime
*/

constexpr std::string_view BUTTON_SHADER_FRAG = R"(
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
)";

constexpr std::string_view SPRITE_SHADER_FRAG = R"(
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
)";

constexpr std::string_view TEXTURE_SHADER_FRAG = R"(
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
)";