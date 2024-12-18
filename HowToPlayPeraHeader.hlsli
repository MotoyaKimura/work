Texture2D<float4> howToPlayTex : register(t0);

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


struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    
};