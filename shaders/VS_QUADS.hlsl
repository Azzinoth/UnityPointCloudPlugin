cbuffer MyCB : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

//float4x4 Identity =
//{
//    { 1, 0, 0, 0 },
//    { 0, 1, 0, 0 },
//    { 0, 0, 1, 0 },
//    { 0, 0, 0, 1 }
//};



//float4x4 Identity = float4( {1,0,0,0},
//		{0,1,0,0},
//		{0,0,1,0},
//		{0,0,0,1}
//);

void VS(float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position)
{
	float4x4 worldMatrix_ = { {0,0,0,0}, {1,1,1,1}, {2,2,2,2}, {3,3,3,3} };
	//matrix newWorldMatrix = Identity;


	float4x4 newWorldMatrix = worldMatrix;
	newWorldMatrix[0][0] = 1.0;
	newWorldMatrix[0][1] = 1.0;
	newWorldMatrix[2][2] = 1.0;
	newWorldMatrix[3][3] = 1.0;
	
	opos = mul(newWorldMatrix, float4(pos, 1));
	
	
	//opos = mul(Identity, opos);
	
	opos = mul(viewMatrix, opos);
	opos = mul(projectionMatrix, opos);
	
	
	
	
	ocolor = color;
}