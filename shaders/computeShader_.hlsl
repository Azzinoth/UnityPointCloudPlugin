RWByteAddressBuffer srcParticleBuffer : register(u0);
RWByteAddressBuffer dstParticleBuffer : register(u1);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 pos = asfloat(srcParticleBuffer.Load3(DTid.x * 12) );
	pos.x += 5;
	dstParticleBuffer.Store3(DTid.x * 12, asuint(pos) );
}
