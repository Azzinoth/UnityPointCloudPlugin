struct BufType
{
	float x, y, z;
	int color;
};

StructuredBuffer<BufType> Buffer0 : register(t0);
StructuredBuffer<BufType> Buffer1 : register(t1);
//RWStructuredBuffer<BufType> BufferOut : register(u0);
AppendStructuredBuffer<BufType> BufferOut : register(u0);

[numthreads(64, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    //BufferOut[DTid.x].i = Buffer0[DTid.x].i + Buffer1[DTid.x].i + 1;
    //BufferOut[DTid.x].f = Buffer0[DTid.x].f + Buffer1[DTid.x].f + 1;
	
	//BufferOut[DTid.x].x = sqrt(pow(Buffer1[DTid.x].x - Buffer0[DTid.x].x, 2) + pow(Buffer1[DTid.x].y - Buffer0[DTid.x].y, 2) + pow(Buffer1[DTid.x].z - Buffer0[DTid.x].z, 2));
	
	
	

	
	
	//BufferOut[DTid.x + 2].x = sqrt(pow(Buffer1[DTid.x].x - Buffer0[DTid.x].x, 2) + pow(Buffer1[DTid.x].y - Buffer0[DTid.x].y, 2) + pow(Buffer1[DTid.x].z - Buffer0[DTid.x].z, 2));
	//if (BufferOut[DTid.x + 2].x < BufferOut[0].x || BufferOut[0].x == 0)
	//{
	//	BufferOut[0].x = BufferOut[DTid.x + 2].x;
	//	BufferOut[1].x = DTid.x;
	//}
	
	//BufferOut[DTid.x].x = Buffer0[DTid.x].x;
	
	//BufferOut[DTid.x].x = sqrt(pow(Buffer1[DTid.x].x - Buffer0[DTid.x].x, 2) + pow(Buffer1[DTid.x].y - Buffer0[DTid.x].y, 2) + pow(Buffer1[DTid.x].z - Buffer0[DTid.x].z, 2));
		





	//float distance = sqrt(pow(Buffer1[DTid.x].x - Buffer0[DTid.x].x, 2) + pow(Buffer1[DTid.x].y - Buffer0[DTid.x].y, 2) + pow(Buffer1[DTid.x].z - Buffer0[DTid.x].z, 2));
	//GroupMemoryBarrierWithGroupSync();
	
	//if (distance < BufferOut[0].x || BufferOut[0].x == 0)
	//{
	//	//float original = 0.0;
	//	//InterlockedExchange(BufferOut[0].x, distance, original);
	//	BufferOut[0].x = distance;
	//	BufferOut[1].x = DTid.x;
	//}
	
	
	//BufferOut[DTid.x].x = Buffer1[DTid.x % 24].x;
	
	//if (Buffer0[DTid.x].y > Buffer1[0].y) // -820
	//{
	//	BufferOut[DTid.x] = Buffer0[DTid.x];
	//}
	//else
	//{
	//	float value = -10000.0;
	//	//if (Buffer1[0].x != 0)
	//	//	value = 420;
	//	BufferOut[DTid.x].x = value;
	//	BufferOut[DTid.x].y = value;
	//	BufferOut[DTid.x].z = value;
	//}
	
	
	
	
	
	
	
	
	
	
	//BufferOut[DTid.x] = Buffer0[DTid.x];
	
	//float distance;
	//for (int p = 0; p < 6; p++)
	//{
	//	distance = Buffer1[p * 4 + 0].x * Buffer0[DTid.x].x + Buffer1[p * 4 + 1].x * Buffer0[DTid.x].y + Buffer1[p * 4 + 2].x * Buffer0[DTid.x].z + Buffer1[p * 4 + 3].x;
		
	//	if (distance <= -2)
	//	{
	//		BufferOut[DTid.x].x = -10000.0;
	//		BufferOut[DTid.x].y = -10000.0;
	//		BufferOut[DTid.x].z = -10000.0;
	//		break;
	//	}
	//}
	
	
	bool shouldProceed = true;
	float distance;
	for (int p = 0; p < 6; p++)
	{
		int tempIndex = p * 4;
		distance = Buffer1[tempIndex].x * Buffer0[DTid.x].x +
		Buffer1[tempIndex + 1].x * Buffer0[DTid.x].y +
		Buffer1[tempIndex + 2].x * Buffer0[DTid.x].z +
		Buffer1[tempIndex + 3].x;
		
		if (distance <= -2)
		{
			shouldProceed = false;
			break;
		}
	}
	
	if (shouldProceed)
		BufferOut.Append(Buffer0[DTid.x]);
	
	

}
