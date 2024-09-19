#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    if(input.instNo == 1)
    {
        return float4(0, 0, 0, 1);
    }
    float brightness = dot(normalize(lightVec.xyz), input.normal);
    float4 texColor = tex.Sample(smp, input.uv);
    return max(saturate(texColor * brightness), saturate(texColor * 0.2));
}