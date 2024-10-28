// Light types.
#define LIGHT_TYPE 0

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
cbuffer DirectionalLight : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
    float AmbientIntensity;
    float pad0;
    float pad1;
    float pad2;
};

cbuffer PointLight : register(b4)
{
    float3 PointPosition;
    float3 PointColor;
    float PointAmbient;
    float pad00;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4 ModelColor;
	float4x4 ModelMatrix;
};

//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localTangent : TANGENT;
	float3 localBitangent : BITANGENT;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 tangent : TANGENT;
	float4 bitangent : BITANGENT;
	float4 normal : NORMAL;
    float4 fragPosition : POSITION;
};

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);
	
	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.tangent = float4(0, 0, 0, 0);
	v2p.bitangent = float4(0, 0, 0, 0);
	v2p.normal = worldNormal;
    v2p.fragPosition = worldPosition;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
    float4 color;
	
	if(LIGHT_TYPE == 0)
    {
        float ambient = AmbientIntensity;
        float directional = SunIntensity * saturate(dot(normalize(input.normal.xyz), -SunDirection));
        float4 lightColor = float4((ambient + directional).xxx, 1);
        float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
        float4 vertexColor = input.color;
        float4 modelColor = ModelColor;
        color = lightColor * textureColor * vertexColor * modelColor;
        clip(color.a - 0.01f);
    }
	else
    {
        float ambient = 0.5;
		
        float distance = length(PointPosition - input.fragPosition.xyz);
		
        float attenuation = 1.0 / ((0.2 * distance) + (0.1 * distance * distance));
		
        float diffuse = max(dot(normalize(PointPosition - input.fragPosition.xyz), normalize(input.normal.xyz)), 0.0);
        diffuse *= attenuation;
		
        float4 lightColor = (ambient + diffuse) * float4(PointColor.xyz, 1);
		
		float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
		float4 vertexColor = input.color;
		float4 modelColor = ModelColor;
		
		color = lightColor * textureColor * vertexColor * modelColor;
    }
	
	return color;
}
