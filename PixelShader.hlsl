#include "ShaderHeader.hlsli"


PixelOutput PS(Output input) : SV_TARGET
{
    PixelOutput output;
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
    float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
    float depthFromLight = lightDepthTex.SampleCmp(shadowSmp, shadowUV, posFromLightVP.z - 0.001f);
    float shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    //if(depthFromLight < posFromLightVP.z - 0.001f)
    //{
    //    shadowWeight = 0.5f;
    //}
    
    if (input.instNo == 1)
    {
        output.col = float4(0, 0, 0, 1);
        output.normal = float4(0, 0, 0, 1);
        return output;
    }
    float brightness = saturate(dot(normalize(lightVec), input.normal.xyz));
    float4 texColor = texCol.Sample(smp, input.uv);
    output.col = max(saturate(texColor * brightness * shadowWeight), saturate(texColor * 0.2));
    //output.col = float4(normalize(input.pos.xyz), 1.0f);
    output.normal.rgb = float3((input.normal.xyz + 1.0f) / 2.0f);
    output.normal.a = 1.0f;
    return output;
}

