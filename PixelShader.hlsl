#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1.0f, -1.0f, 1.0f));
    float brightness = dot(-light, input.normal);
	//return float4(input.normal.xyz, 1.0f);

    return float4(tex.Sample(smp, input.uv).xyz * brightness, 1.0f);
}