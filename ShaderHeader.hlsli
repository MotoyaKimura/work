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

Texture2D<float4> tex : register(t0);
Texture2D<float> depthTex : register(t1);
Texture2D<float>lightDepthTex : register(t2);

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