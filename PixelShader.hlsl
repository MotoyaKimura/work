#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
	//return float4(input.normal.xyz, 1.0f);
    return float4(tex.Sample(smp, input.uv).xyz, 1.0f);
}