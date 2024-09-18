cbuffer cbuff0 : register(b0)
{
    matrix view;
    matrix projection;
    float4 lightVec;
    float3 eye;
};

cbuffer cbuff1 : register(b1)
{
    matrix world;
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

struct Output
{
	float4 svpos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;

};