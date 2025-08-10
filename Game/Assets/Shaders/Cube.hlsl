//------------------------------------------
// Constant buffer for transformation matrices
// Slot b0 in the vertex shader
//------------------------------------------
cbuffer TransformCB : register(b0)
{
    matrix MVP; // Combined World-View-Projection matrix
    float4 COLOR;
}

//--------------------------------------------------------------------------------------
// Vertex Input and Output Structures
//--------------------------------------------------------------------------------------
struct VSInput
{
    float3 Position : POSITION;  // Object-space position
    float3 Color    : COLOR;     // Vertex color
};

struct VSOutput
{
    float4 Position : SV_POSITION; // Clip-space position
    float3 Color    : COLOR;       // Passed-through color
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    // Convert float3 to float4, w = 1.0
    output.Position = mul(float4(input.Position, 1.0), MVP);
    output.Color    = COLOR;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain(VSOutput input) : SV_TARGET
{
    // Simply output the interpolated color
    return float4(input.Color, 1.0);
}