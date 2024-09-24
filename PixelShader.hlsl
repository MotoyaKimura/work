#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
    float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
    float depthFromLight = lightDepthTex.Sample(smp, shadowUV);
    float shadowWeight = 1.0f;
    if(depthFromLight < posFromLightVP.z - 0.001f)
    {
        shadowWeight = 0.5f;
    }
    
    if (input.instNo == 1)
    {
        return float4(0, 0, 0, 1);
    }
    float brightness = dot(normalize(lightVec.xyz), input.normal.xyz);
    float4 texColor = texCol.Sample(smp, input.uv);
    return max(saturate(texColor * brightness * shadowWeight), saturate(texColor * 0.2));
}

