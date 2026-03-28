//--------------------------------------------------------------------------------------
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 BiNormal : BINORMAL;
    float2 TexCoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : TEXCOORD0;
    float3 BiNormal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
struct DrawConstants
{
    uint albedoIndex;     // Texture SRV index in bindless heap
    uint transformIndex;  // Row in global TransformData buffer
    uint pad0;
    uint pad1;
};

//--------------------------------------------------------------------------------------
struct TransformData
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

//--------------------------------------------------------------------------------------
// Root constants at b0 
ConstantBuffer<DrawConstants> g_Draw : register(b0);

// Shared static sampler at s0
SamplerState g_texSampler : register(s0);