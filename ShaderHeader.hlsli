cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    matrix invprojection;
    matrix shadow;
    matrix shadowOffsetY;
    matrix invShadowOffsetY;
    matrix lightView;
    matrix lightCamera;
    float3 lightVec;
    float3 eye;
};

cbuffer cbuff1 : register(b1)
{
    matrix world;
};

cbuffer cbuff2 : register(b2)
{
    float3 diffuse;
    float3 specular;
    float alpha;
    float shininess;
};


Texture2D<float>lightDepthTex : register(t0);

SamplerState smp : register(s0);
SamplerComparisonState shadowSmp : register(s1);

struct Output
{
    float4 pos : POSITION;
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
    float4 tpos : TPOS;
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