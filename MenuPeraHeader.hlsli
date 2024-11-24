Texture2D<float4> startTex : register(t0);
Texture2D <float4> restartTex : register(t1);
Texture2D<float4> titleTex : register(t2);
Texture2D<float4> menuTex : register(t3);


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
    float startWipeRight;
    float endWipeRight;
    float endWipeDown;
    float endWipeCenter;
    float fade;
    bool isPause;
}

SamplerState smp : register(s0);


struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    
};