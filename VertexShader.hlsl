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
    output.svpos = mul(mat, pos);
	output.normal = normal;
	output.uv = uv;

	return output;
}
