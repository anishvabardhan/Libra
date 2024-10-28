Texture2D<float4> inTexture : register(t0);

RWTexture2D<float4> outTexture : register(u0);

void ThresholdPixel(uint x, uint y)
{
    uint2 pixelUV = uint2(x, y);
    
    if (inTexture[pixelUV].r == 1.0 && inTexture[pixelUV].g == 1.0 && inTexture[pixelUV].b == 1.0 && inTexture[pixelUV].a == 1.0)
    {
        outTexture[pixelUV] = inTexture[pixelUV];
    }
    else
    {
        outTexture[pixelUV] = float4(0.0, 0.0, 0.0, 0.0);
    }
}

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{        
    ThresholdPixel(id.x, id.y);
}