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
	bool shouldProceed = true;
	float distance;
	for (int p = 0; p < 6; p++)
	{
		int tempIndex = p * 4;
		distance = Buffer1[tempIndex] * Buffer0[DTid.x].x +
		Buffer1[tempIndex + 1] * Buffer0[DTid.x].y +
		Buffer1[tempIndex + 2] * Buffer0[DTid.x].z +
		Buffer1[tempIndex + 3];
		
		if (distance <= -2)
		{
			shouldProceed = false;
			break;
		}
	}
	
	if (shouldProceed)
		BufferOut.Append(Buffer0[DTid.x]);
}