cbuffer MyCB : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}
void VS(float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position)
{
	opos = mul(worldMatrix, float4(pos, 1));
	opos = mul(viewMatrix, opos);
	opos = mul(projectionMatrix, opos);
	ocolor = color;
}
float4 PS(float4 color : COLOR) : SV_TARGET
{
	return color;
}