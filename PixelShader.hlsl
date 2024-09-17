#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
	return float4(input.normal.xyz, 1.0f);
}