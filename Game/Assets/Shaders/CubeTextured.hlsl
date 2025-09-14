#pragma pack_matrix(row_major)

//------------------------------------------
// Constant buffer for transformation matrices
// Slot b0 in the vertex shader
//------------------------------------------
cbuffer TransformCB : register(b0)
{
    float4x4 World; 
    float4x4 View;
    float4x4 Proj;
}

//--------------------------------------------------------------------------------------
// Vertex Input and Output Structures
//--------------------------------------------------------------------------------------
struct VSInput
{
    float3 Position : POSITION;  // Object-space position
    float3 Normal   : NORMAL;    // Vertex Normal
    float3 BiNormal : BINORMAL;  // Vertex Bi-Normal
    float2 TexCoord : TEXCOORD0; // UV Coordinates   
};

struct VSOutput
{
    float4 Position : SV_POSITION; // Clip-space position
    float3 NORMAL   : TEXCOORD0;   // Pass-through World Space Normals
    float3 BINORMAL : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    
    float4x4 WVP = mul(mul(World, View), Proj);
    
    output.Position = mul(float4(input.Position, 1), WVP);
    output.NORMAL =  input.Normal;
    output.BINORMAL = input.BiNormal;
    output.TexCoord = input.TexCoord;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain(VSOutput input) : SV_TARGET
{
    // Simply output the interpolated color
    // return float4(input.NORMAL, 1);
    return input.Position;
}