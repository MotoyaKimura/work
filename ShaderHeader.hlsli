cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    matrix invprojection;
    matrix shadow;
    matrix shadowOffsetY;
    matrix invShadowOffsetY;
    matrix lightCamera;
    float3 lightVec;
    float3 eye;
};

cbuffer cbuff1 : register(b1)
{
    matrix world;
};

Texture2D<float4> texCol : register(t0);
Texture2D<float>lightDepthTex : register(t1);

SamplerState smp : register(s0);

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