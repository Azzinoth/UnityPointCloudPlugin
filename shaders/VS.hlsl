cbuffer MyCB : register(b0)
{
	float4x4 worldMatrix;
}

struct VS_INPUT
{
    uint vertexID : SV_VertexID;
};

struct pointInfo
{
	float x, y, z;
	int color;
};

StructuredBuffer<pointInfo> pointBuffer : register(t0);

void VS(VS_INPUT input, float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position)
{
	//opos = mul(worldMatrix, float4(pos, 1));
	opos = mul(worldMatrix, float4(pointBuffer[input.vertexID].x, pointBuffer[input.vertexID].y, pointBuffer[input.vertexID].z, 1.0f));
	
	
	//ocolor = pointBuffer[input.vertexID].color;
	
	ocolor[3] = (pointBuffer[input.vertexID].color >> 24) / 255.0f;
	ocolor[2] = ((pointBuffer[input.vertexID].color & 0x00ff0000) >> 16) / 255.0f;
	ocolor[1] = ((pointBuffer[input.vertexID].color & 0x0000ff00) >> 8) / 255.0f;
	ocolor[0] = (pointBuffer[input.vertexID].color & 0x000000ff) / 255.0f;
	
	//discard;
}