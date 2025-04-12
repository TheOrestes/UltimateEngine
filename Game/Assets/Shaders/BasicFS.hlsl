//---------------------------------------------------------------------------------------------------------------------
cbuffer RootConstants : register(b0)
{
	float3 rgb;
	float deltaTime;
};

Texture2D BaseTexture : register(t0);
SamplerState samplerState : register(s0);
cbuffer ConstantBuffer : register(b1)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4 ambientColor;
};

//---------------------------------------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
};

//---------------------------------------------------------------------------------------------------------------------
float4 main(VS_OUTPUT In) : SV_TARGET
{
	float4 texColor = BaseTexture.Sample(samplerState, In.Texcoord);
	return float4(texColor * ambientColor);
}