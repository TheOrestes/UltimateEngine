#pragma pack_matrix(row_major)

//#include "BindlessCommon.hlsli"

//--------------------------------------------------------------------------------------
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 BiNormal : BINORMAL;
    float2 TexCoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    // Transform buffer is bound as SRV in bindless heap slot 0
    StructuredBuffer<TransformData> g_Transforms = ResourceDescriptorHeap[0];

    // Fetch this draw's transform by index
    TransformData tr = g_Transforms[g_Draw.transformIndex];

    float4 worldPos = mul(float4(input.Position, 1.0f), tr.World);
    float4 viewPos = mul(worldPos, tr.View);
    float4 clipPos = mul(viewPos, tr.Proj);

    output.Position = clipPos;

    // Normal: transform to world space (ignore view for simple lighting)
    float3 worldNormal = mul(float4(input.Normal, 0.0f), tr.World).xyz;
    output.Normal = normalize(worldNormal);

    output.BiNormal = input.BiNormal;
    output.TexCoord = input.TexCoord;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain(VSOutput input) : SV_TARGET
{
    // Fetch the texture at the bindless index
    Texture2D<float4> baseTexture = ResourceDescriptorHeap[g_Draw.albedoIndex];

    return baseTexture.Sample(g_texSampler, input.TexCoord);
}
