#include "ShaderHeader.hlsli"

Output VS(
float4 pos : POSITION,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float3 morphPos : MORPHPOSITION,
float4 morphUV : MORPHUV,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
	Output output;
	output.pos = mul(world, pos);
    output.pos = mul(shadowOffsetY, output.pos);
    if(instNo == 1)
    {
        output.pos = mul(shadow, output.pos);
    }
    output.pos = mul(invShadowOffsetY, output.pos);
    output.svpos = mul(mul(projection, view), output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);
    output.normal = mul(world, normal);
    output.vnormal = mul(view, output.normal);
	output.uv = uv;
    output.ray = normalize(output.pos.xyz - mul(view, eye));
    output.instNo = instNo;
	return output;
}

Output rsmVS(
float4 pos : POSITION ,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float3 morphPos : MORPHPOSITION,
float4 morphUV : MORPHUV,
uint4 boneNo : BONENO,
float4 boneWeight : WEIGHT,
min16uint weightType : WEIGHTTYPE,
uint instNo : SV_InstanceID
) 
{
    Output output;

    output.pos = mul(world, pos);
    output.pos = mul(shadowOffsetY, output.pos);
    output.pos = mul(invShadowOffsetY, output.pos);
    output.svpos = mul(lightCamera, output.pos);
    normal.w = 0;
    output.tpos = mul(lightCamera, output.pos);
    output.normal = mul(world, normal);
    output.uv = uv;
    output.instNo = instNo;
    return output;
}