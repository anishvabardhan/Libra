StructuredBuffer<float4> renderTargetData : register(t0);

RWTexture2D<float4> backBufferData : register(u0);

[numthreads(32, 32, 1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    backBufferData[id.xy] = float4(renderTargetData[id.y].r, renderTargetData[id.y].g, renderTargetData[id.y].b, renderTargetData[id.y].a);
}