Texture2D<float4> tex : register(t0);
Texture2D<float4> normalTex : register(t1);
Texture2D<float> depthTex : register(t2);
Texture2D<float> lightDepthTex : register(t3);
Texture2D<float> ssaoTex : register(t4);

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

SamplerState smp : register(s0);


struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    
};