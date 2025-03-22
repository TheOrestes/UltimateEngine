//---------------------------------------------------------------------------------------------------------------------
cbuffer RootConstants : register(b0)
{
	float3 rgb;
	float deltaTime;
};

Texture2D BaseTexture : register(t0);
SamplerState samplerState : register(s0);

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
	return float4(texColor);
}