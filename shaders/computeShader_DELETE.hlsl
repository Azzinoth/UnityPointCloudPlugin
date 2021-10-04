struct BufType
{
	float x, y, z;
	int color;
};

StructuredBuffer<BufType> Buffer0 : register(t0);
StructuredBuffer<float> Buffer1 : register(t1);
AppendStructuredBuffer<BufType> BufferOut : register(u0);

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 pointPosition = { Buffer0[DTid.x].x, Buffer0[DTid.x].y, Buffer0[DTid.x].z };
	float3 spherePosition = { Buffer1[0], Buffer1[1], Buffer1[2] };
	
	float distanceToSphere = distance(pointPosition, spherePosition);
	
	if (distanceToSphere > Buffer1[3])
		BufferOut.Append(Buffer0[DTid.x]);
}