struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : Color;
};

float4 main(VS_OUTPUT In) : SV_TARGET
{
	return In.Color;
}