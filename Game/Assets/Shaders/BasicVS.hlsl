//---------------------------------------------------------------------------------------------------------------------
cbuffer RootConstants : register(b0)
{
	float3 rgb;
	float deltaTime;
};

cbuffer ConstantBuffer : register(b1)
{
	float4x4 worldMVP;
	//float4x4 viewMatrix;
	//float4x4 projectionMatrix;
	float4 ambientColor;
};

//---------------------------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD;
};

//---------------------------------------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
};

//---------------------------------------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT In) 
{
	VS_OUTPUT Out;

	//float4 worldPos = mul(In.Position, worldMatrix);
	//float4 viewPos = mul(worldPos, viewMatrix);

	Out.Position = mul(In.Position, worldMVP);
	Out.Texcoord = In.Texcoord;

	return Out;
}