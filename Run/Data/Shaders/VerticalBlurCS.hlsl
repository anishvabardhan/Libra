Texture2D<float4> inTexture : register(t0);
RWTexture2D<float4> blurredTexture : register(u0);

void VerticalGBlur(uint x, uint y)
{
    uint2 inTextureDims;
    inTexture.GetDimensions(inTextureDims.x, inTextureDims.y);
    
    float weight0 = 1.0;
    float weight1 = 0.9;
    float weight2 = 0.75;
    float weight3 = 0.58;
    float weight4 = 0.42;
    float weight5 = 0.35;
    float weight6 = 0.27;
    float weight7 = 0.16;
    float weight8 = 0.1;
        
    uint2 inPixel1UV    = uint2(x, y + 1);
    uint2 inPixel2UV    = uint2(x, y + 2);
    uint2 inPixel3UV    = uint2(x, y + 3);
    uint2 inPixel4UV    = uint2(x, y + 4);
    uint2 inPixel5UV    = uint2(x, y + 5);
    uint2 inPixel6UV    = uint2(x, y + 6);
    uint2 inPixel7UV    = uint2(x, y + 7);
    uint2 inPixel8UV    = uint2(x, y + 8);
    
    uint2 inPixel9UV    = uint2(x, y - 1);
    uint2 inPixel10UV   = uint2(x, y - 2);
    uint2 inPixel11UV   = uint2(x, y - 3);
    uint2 inPixel12UV   = uint2(x, y - 4);
    uint2 inPixel13UV   = uint2(x, y - 5);
    uint2 inPixel14UV   = uint2(x, y - 6);
    uint2 inPixel15UV   = uint2(x, y - 7);
    uint2 inPixel16UV   = uint2(x, y - 8);

    uint2 outPixelUV = uint2(x, y);
    
    float normalization = (weight0 + 2.0 * (weight1 + weight2 + weight3 + weight4 + weight5 + weight6 + weight7 + weight8));

    weight0 /= normalization;
    weight1 /= normalization;
    weight2 /= normalization;
    weight3 /= normalization;
    weight4 /= normalization;
    weight5 /= normalization;
    weight6 /= normalization;
    weight7 /= normalization;
    weight8 /= normalization;

    blurredTexture[outPixelUV] = float4(inTexture[outPixelUV].r * weight0, inTexture[outPixelUV].g * weight0, inTexture[outPixelUV].b * weight0,  inTexture[outPixelUV].a * weight0);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel1UV].r * weight1, inTexture[inPixel1UV].g * weight1, inTexture[inPixel1UV].b * weight1, inTexture[inPixel1UV].a * weight1);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel9UV].r * weight1, inTexture[inPixel9UV].g * weight1, inTexture[inPixel9UV].b * weight1, inTexture[inPixel9UV].a * weight1);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel2UV].r * weight2, inTexture[inPixel2UV].g * weight2, inTexture[inPixel2UV].b * weight2, inTexture[inPixel2UV].a * weight2);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel10UV].r * weight2, inTexture[inPixel10UV].g * weight2, inTexture[inPixel10UV].b * weight2, inTexture[inPixel10UV].a * weight2);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel3UV].r * weight3, inTexture[inPixel3UV].g * weight3, inTexture[inPixel3UV].b * weight3, inTexture[inPixel3UV].a * weight3);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel11UV].r * weight3, inTexture[inPixel11UV].g * weight3, inTexture[inPixel11UV].b * weight3, inTexture[inPixel11UV].a * weight3);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel4UV].r * weight4, inTexture[inPixel4UV].g * weight4, inTexture[inPixel4UV].b * weight4, inTexture[inPixel8UV].a * weight4);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel12UV].r * weight4, inTexture[inPixel12UV].g * weight4, inTexture[inPixel12UV].b * weight4, inTexture[inPixel12UV].a * weight4);
    
    blurredTexture[outPixelUV] += float4(inTexture[inPixel5UV].r * weight5, inTexture[inPixel5UV].g * weight5, inTexture[inPixel5UV].b * weight5, inTexture[inPixel5UV].a * weight5);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel13UV].r * weight5, inTexture[inPixel13UV].g * weight5, inTexture[inPixel13UV].b * weight5, inTexture[inPixel13UV].a * weight5);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel6UV].r * weight6, inTexture[inPixel6UV].g * weight6, inTexture[inPixel6UV].b * weight6, inTexture[inPixel6UV].a * weight6);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel14UV].r * weight6, inTexture[inPixel14UV].g * weight6, inTexture[inPixel14UV].b * weight6, inTexture[inPixel14UV].a * weight6);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel7UV].r * weight7, inTexture[inPixel7UV].g * weight7, inTexture[inPixel7UV].b * weight7, inTexture[inPixel7UV].a * weight7);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel15UV].r * weight7, inTexture[inPixel15UV].g * weight7, inTexture[inPixel15UV].b * weight7, inTexture[inPixel15UV].a * weight7);

    blurredTexture[outPixelUV] += float4(inTexture[inPixel8UV].r * weight8, inTexture[inPixel8UV].g * weight8, inTexture[inPixel8UV].b * weight8, inTexture[inPixel8UV].a * weight8);
    blurredTexture[outPixelUV] += float4(inTexture[inPixel16UV].r * weight8, inTexture[inPixel16UV].g * weight8, inTexture[inPixel16UV].b * weight8, inTexture[inPixel16UV].a * weight8);
}

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{        
    VerticalGBlur(id.x, id.y);
}