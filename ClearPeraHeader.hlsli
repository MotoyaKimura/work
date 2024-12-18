Texture2D<float4> worldTex : register(t0);
Texture2D<float4> lightNormalTex : register(t1);
Texture2D<float4> indirectLightTex : register(t2);
Texture2D<float> lightDepthTex : register(t3);

Texture2D<float4> tex : register(t4);
Texture2D<float4> normalTex : register(t5);
Texture2D<float> depthTex : register(t6);

Texture2D<float> ssaoTex : register(t7);
Texture2D<float4> clearTex : register(t8);
Texture2D<float4> restartTex : register(t9);
Texture2D<float4> titleTex : register(t10);
Texture2D<float4> creditTex1 : register(t11);
Texture2D<float4> creditTex2 : register(t12);


cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    matrix invprojection;
    matrix lightCamera;
    float3 lightVec;
    float3 eye;
};

cbuffer cbuff1 : register(b1)
{
    float startWipeOpen;
    float endWipeClose;
    float fade;
    float gameOverFade;
    float clearFade;
    float monochromeRate;
    float screenWidth;
    float screenHeight;
    float startHoverCnt;
    float restartHoverCnt;
    float titleHoverCnt;
    float creditCnt;
    int milliSecond;
    bool isPause;
}

SamplerState smp : register(s0);
SamplerState smp2 : register(s2);

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    
};