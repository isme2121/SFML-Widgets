#pragma once
#include <string>

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
* icon_size: accurate icon sprite size
* icon_local_center: icon center coordinates local to button shape
*/

uniform sampler2D texture;
uniform vec2 size;
uniform float radius;
uniform float opacity;
uniform vec2 icon_size;
uniform vec2 icon_local_center;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    vec2 sprite_x0 = icon_local_center - icon_size * 0.5;
    vec2 pos = sprite_x0 + uv * icon_size;

    vec2 half_shape_size = size * 0.5;
    vec2 q = abs(pos - half_shape_size) - (half_shape_size - vec2(radius));

    float dist = length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - radius;
    if (dist > 0.0)
        discard;

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

#ifdef CANVAS_MORE_LINE_TYPES
constexpr std::string_view OPEN_LINE_SHADER_FRAG = R"(
/*
* Default shader for open line type in GridCanvas
*/
uniform int join_lines;             // 0->opening for each unit; 1->single opening
uniform float zero_space_width_ratio;
uniform float one_space_width_ratio; 

void main()
{
    float pos = gl_TexCoord[0].x;
    float line_length = gl_TexCoord[0].y;

    float length_in_units = (join_lines == 0) ? 1.0 : line_length;
    float center = (join_lines == 0) ? (floor(pos) + 0.5) : (line_length * 0.5);
    float width = min(zero_space_width_ratio, 1.0 - 2.0 * one_space_width_ratio) * length_in_units;

    if(abs(pos - center) < width * 0.5) discard;
    gl_FragColor = gl_Color;
}
)";

constexpr std::string_view DOTTED_LINE_SHADER_FRAG = R"(
uniform float dot_radius; // dot size

void main()
{
    vec2 cell_pos = vec2(fract(gl_TexCoord[0].x) - 0.5, fract(gl_TexCoord[0].y) - 0.5);
    vec2 pixels_per_unit = 1.0 / max(fwidth(gl_TexCoord[0].xy), vec2(1e-5));
    vec2 dist_pixels = cell_pos * pixels_per_unit;

    if(length(dist_pixels) > dot_radius) discard;
    gl_FragColor = gl_Color;
}
)";
#endif