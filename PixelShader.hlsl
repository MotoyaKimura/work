#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float dep = pow(depthTex.Sample(smp, input.uv), 20);
    return float4(dep, dep, dep, 1.0f);
    if(input.instNo == 1)
    {
        return float4(0, 0, 0, 1);
    }
    float brightness = dot(normalize(lightVec.xyz), input.normal);
    float4 texColor = tex.Sample(smp, input.uv);
    return max(saturate(texColor * brightness), saturate(texColor * 0.2));
}