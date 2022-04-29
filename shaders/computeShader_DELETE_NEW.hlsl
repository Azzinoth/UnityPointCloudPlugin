struct BufType
{
	float x, y, z;
	int color;
};

StructuredBuffer<BufType> Buffer0 : register(t0);
StructuredBuffer<float> Buffer1 : register(t1);
RWStructuredBuffer<BufType> BufferOut : register(u0);

[numthreads(1024, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 pointPosition = { Buffer0[DTid.x].x, Buffer0[DTid.x].y, Buffer0[DTid.x].z };
	float3 spherePosition = { Buffer1[0], Buffer1[1], Buffer1[2] };
	
	float distanceToSphere = distance(pointPosition, spherePosition);
	
	//if (distanceToSphere > Buffer1[3])
	//{
	//	BufferOut[DTid.x].x = -10000.0;
	//	BufferOut[DTid.x].y = -10000.0;
	//	BufferOut[DTid.x].z = -10000.0;
	//}
	
	if (BufferOut[DTid.x].z > 0)
	{
		BufferOut[DTid.x].x = -10000.0;
		BufferOut[DTid.x].y = -10000.0;
		BufferOut[DTid.x].z = -10000.0;
	}
	
	//BufferOut.Append(Buffer0[DTid.x]);
}