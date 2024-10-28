Texture2D<float4> inThresholdTexture : register(t0);
Texture2D<float4> inBlurTexture : register(t1);

RWTexture2D<float4> compositeTexture : register(u0);

void CompositePixel(uint x, uint y)
{
    uint2 pixelUV = uint2(x, y);
    
    compositeTexture[pixelUV] += inThresholdTexture[pixelUV] + inBlurTexture[pixelUV];
}

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{        
    CompositePixel(id.x, id.y);
}