Texture2D<float4> normalTex : register(t1);
Texture2D<float> depthTex : register(t2);

SamplerState smp : register(s0);

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};