R"(
//*************************************************************************************************
// FsQuadToBuffer.frag
//*************************************************************************************************
#version 430

layout (std140, binding = 0) uniform buf
{
    uint rtWidth;
    uint flipX;
    uint flipY;
} uniformBuf;

layout (binding = 1) uniform sampler2D inTex;

layout (std430, binding = 2) buffer OutBuf
{
    uint outBuf[];
};

layout (location = 0) in vec4 texCoords;
layout (location = 0) out vec4 fragColor;

//*************************************************************************************************
// UpscaleRange()
//
// Convert from 0..1 range to 0..255 range.
//*************************************************************************************************
uint UpscaleRange(float val)
{
    return uint(clamp(val, 0.0f, 1.0f) * 255);
}

//*************************************************************************************************
// CompressColor()
//
// Pack vec4 float into a UINT.
//*************************************************************************************************
uint CompressColor(vec4 inColor)
{
    uint compressedColor = 0;

    compressedColor |= UpscaleRange(1);
    compressedColor <<= 8;

    compressedColor |= UpscaleRange(inColor.z);
    compressedColor <<= 8;

    compressedColor |= UpscaleRange(inColor.y);
    compressedColor <<= 8;

    compressedColor |= UpscaleRange(inColor.x);

    return compressedColor;
}

//*************************************************************************************************
// Fragment Shader
//
// Writes full screen quad to a linear output buffer where each channel fits in 1 byte.
//*************************************************************************************************
void main()
{
    vec2 coords = texCoords.xy;

    if (uniformBuf.flipX == 1)
    {
        coords.x -= 1.0f;
        coords.x *= -1.0f;
    }

    if (uniformBuf.flipY == 1)
    {
        coords.y -= 1.0f;
        coords.y *= -1.0f;
    }

    vec4 outColor = texture(inTex, coords.xy);

    uint flatPixelIdx = uint(((gl_FragCoord.y - 0.5f) * uniformBuf.rtWidth) + (gl_FragCoord.x - 0.5f));

    outBuf[flatPixelIdx] = CompressColor(outColor);

    fragColor = outColor;
}

)"