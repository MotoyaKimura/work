Texture2D<float4> tex : register(t0);
Texture2D<float4> normalTex : register(t1);
Texture2D<float4> worldTex : register(t3);
Texture2D<float4> lightNormalTex : register(t4);
Texture2D<float4> indirectLightTex : register(t5);
Texture2D<float> depthTex : register(t2);

Texture2D<float> lightDepthTex : register(t6);
Texture2D<float> ssaoTex : register(t7);

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

cbuffer cbuff1 : register(b1){
    float wipeSize;
}

SamplerState smp : register(s0);


struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    
};