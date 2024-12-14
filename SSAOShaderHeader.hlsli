Texture2D<float4> normalTex : register(t5);
Texture2D<float> depthTex : register(t6);


cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    matrix invprojection;
    matrix lightCamera;
    float3 lightVec;
    float3 eye;
};

SamplerState smp : register(s0);
SamplerComparisonState shadowSmp : register(s1);

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};