#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float brightness = dot(normalize(lightVec.xyz), input.normal);

    return float4(tex.Sample(smp, input.uv).xyz * brightness, 1.0f);
}