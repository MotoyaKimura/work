#include "ShaderHeader.hlsli"

Output VS(
float4 pos : POSITION ,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float4 weight : WEIGHT,
min16uint4 index : INDICES
) 
{
	Output output;
	pos = mul(world, pos);
    output.svpos = mul(mul(projection, view), pos);
    normal.w = 0;
    output.normal = mul(world, normal);
	output.uv = uv;

	return output;
}
