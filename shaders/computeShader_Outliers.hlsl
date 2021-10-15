struct BufType
{
	float x, y, z;
	int color;
};

StructuredBuffer<BufType> Buffer0 : register(t0);
AppendStructuredBuffer<float> BufferOut : register(u0);

[numthreads(1024, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 pointPosition = { Buffer0[DTid.x].x, Buffer0[DTid.x].y, Buffer0[DTid.x].z };
	
	float min = 9999999;
	for (int i = DTid.x + 1; i < DTid.x + 10000; i++)
	{
		float3 anotherPointPosition = { Buffer0[i].x, Buffer0[i].y, Buffer0[i].z };
		
		//if (anotherPointPosition.x == pointPosition.x &&
		//	anotherPointPosition.y == pointPosition.y &&
		//	anotherPointPosition.z == pointPosition.z)
		//	continue;
		
		float currentDistance = distance(pointPosition, anotherPointPosition);
		
		if (min > currentDistance)
		{
			min = currentDistance;
			if (min < 10)
				break;
		}
	}
	
	//if (DTid.x < 100000)
		BufferOut.Append(min); // min DTid.x
}