#include "PeraShaderHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
	
	return tex.Sample(smp, input.uv);
}