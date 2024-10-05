struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

VS_OUTPUT main( VS_INPUT In) 
{
	VS_OUTPUT Out;

	Out.Position = float4(In.Position, 1.0f);
	Out.Color = In.Color;

	return Out;
}