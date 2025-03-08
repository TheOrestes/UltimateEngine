//---------------------------------------------------------------------------------------------------------------------
cbuffer RootConstants : register(b0)
{
	float3 rgb;
	float deltaTime;
};

cbuffer ConstantBuffer : register(b1)
{
	float4 Offset;
};

//---------------------------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

//---------------------------------------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

//---------------------------------------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT In) 
{
	VS_OUTPUT Out;

	Out.Position = float4(In.Position, 1.0f) + Offset;
	Out.Color = In.Color;

	return Out;
}