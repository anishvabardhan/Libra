Texture2D<float4> inTexture : register(t0);
RWTexture2D<float4> downSampledTexture : register(u0);

void DownSampling(uint x, uint y)
{
    uint2 inPixel1UV = uint2(x, y);
    uint2 inPixel2UV = uint2(x + 1, y);
    uint2 inPixel3UV = uint2(x, y + 1);
    uint2 inPixel4UV = uint2(x + 1, y);    
    
    uint2 outPixelUV = uint2(x / 2, y / 2);
    
    float avgRedVal = (inTexture[inPixel1UV].r + inTexture[inPixel2UV].r + inTexture[inPixel3UV].r + inTexture[inPixel4UV].r) * 0.25;
    float avgGreenVal = (inTexture[inPixel1UV].g + inTexture[inPixel2UV].g + inTexture[inPixel3UV].g + inTexture[inPixel4UV].g) * 0.25;
    float avgBlueVal = (inTexture[inPixel1UV].b + inTexture[inPixel2UV].b + inTexture[inPixel3UV].b + inTexture[inPixel4UV].b) * 0.25;
    float avgAlphaVal = (inTexture[inPixel1UV].a + inTexture[inPixel2UV].a + inTexture[inPixel3UV].a + inTexture[inPixel4UV].a) * 0.25;
    
    downSampledTexture[outPixelUV] = float4(avgRedVal, avgGreenVal, avgBlueVal, avgAlphaVal);
}

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{    
    DownSampling(id.x, id.y);
    
    id.x += 2;
    
    if(id.x == 256)
    {
        id.y += 2;
        
        id.x = 0;
    }
}