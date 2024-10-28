Texture2D srcTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer CameraConstants : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

cbuffer ModelConstants : register(b1)
{
    float4 ModelColor;
    float4x4 ModelMatrix;
};

PS_INPUT VertexMain(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 localPosition = float4(input.pos, 1.0f);

    float4 worldPosition = mul(ModelMatrix, localPosition);
    float4 viewPosition = mul(ViewMatrix, worldPosition);
    float4 clipPosition = mul(ProjectionMatrix, viewPosition);
    
    output.position = clipPosition;
    output.color = input.color;
    output.uv = input.uv;
    
    return output;
}

float4 PixelMain(PS_INPUT input) : SV_TARGET0
{
    float4 textureColor = srcTexture.Sample(diffuseSampler, input.uv);

    return input.color * ModelColor * textureColor;
}