Texture2D<float4> inTexture : register(t0);
RWTexture2D<float4> UpSampledTexture : register(u0);

void UpSampling(uint x, uint y)
{
    uint2 outPixel1UV = uint2(2 * x, 2 * y);
    uint2 outPixel2UV = uint2((2 * x) + 1, (2 * y));
    uint2 outPixel3UV = uint2(2 * x, (2 * y) + 1);
    uint2 outPixel4UV = uint2((2 * x) + 1, 2 * y);
    
    uint2 inPixelUV = uint2(x, y);
    
    UpSampledTexture[outPixel1UV] = inTexture[inPixelUV];
    UpSampledTexture[outPixel2UV] = inTexture[inPixelUV];
    UpSampledTexture[outPixel3UV] = inTexture[inPixelUV];
    UpSampledTexture[outPixel4UV] = inTexture[inPixelUV];
}

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{    
    UpSampling(id.x, id.y);
}