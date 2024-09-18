#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float brightness = dot(normalize(lightVec.xyz), input.normal);
    float4 texColor = tex.Sample(smp, input.uv);
    return max(saturate(texColor * brightness), saturate(texColor * 0.2));
}