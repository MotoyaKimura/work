#include "ShaderHeader.hlsli"

Output VS(
float4 pos : POSITION ,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float4 weight : WEIGHT,
min16uint4 index : INDICES,
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
    output.normal = mul(world, normal);
	output.uv = uv;
    output.instNo = instNo;
	return output;
}
