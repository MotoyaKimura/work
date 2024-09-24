#include "PeraShaderHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
	float dep = pow(depthTex.Sample(smp, input.uv), 20);
	return tex.Sample(smp, input.uv);
	return float4(dep, dep, dep, 1);
}