cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    matrix invprojection;
    matrix lightView;
    matrix lightCamera;
    float3 lightVec;
    float3 eye;
};

cbuffer cbuff1 : register(b1)
{
    matrix world;
    matrix bones[512];
};

cbuffer cbuff2 : register(b2)
{
    matrix invBones[512];
};

cbuffer cbuff3 : register(b3)
{
    float4 diffuse;
    float3 specular;
    float specularPower;
    float3 ambient;
};


Texture2D<float>lightDepthTex : register(t0);
Texture2D<float4>texColor : register(t1);
Texture2D<float4>toon : register(t2);
Texture2D<float4> sphere : register(t3);


SamplerState smp : register(s0);
SamplerComparisonState shadowSmp : register(s1);
SamplerState smpToon : register(s2);

struct Output
{
    float4 pos : POSITION;
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL;
    float4 vnormal : NORMAL1;
	float2 uv : TEXCOORD;
    float4 tpos : TPOS;
    float3 ray : VECTOR;
    uint instNo : SV_InstanceID;
};

struct PixelOutput
{
    float4 col : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

struct RSMOutput
{
    float4 world : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 indirectLight : SV_TARGET2;
};