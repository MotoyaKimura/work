#include "ShaderHeader.hlsli"


float4 PS(Output input) : SV_TARGET
{
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
    float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
    float depthFromLight = lightDepthTex.Sample(smp, shadowUV);
    //return float4(depthFromLight, depthFromLight, depthFromLight, 1);
    float shadowWeight = 1.0f;
    if(depthFromLight < posFromLightVP.z)
    {
        shadowWeight = 0.5f;
    }
   
    float dep = pow(depthTex.Sample(smp, input.uv), 20);
   //return float4(dep, dep, dep, 1);
    float4 respos = mul(invprojection, float4(input.uv * float2(2, -2) + float2(-1, 1), dep, 1));
    respos.xyz /= respos.w;
    
    //if(input.instNo == 1)
    //{
    //    return float4(0, 0, 0, 1);
    //}
    float brightness = dot(normalize(lightVec.xyz), input.normal.xyz);
    float4 texColor = tex.Sample(smp, input.uv);
    //return float4(texColor.xyz, 1);
    //return float4(input.pos.xyz / input.pos.w, 1.0f);
    return max(saturate(texColor * brightness * shadowWeight), saturate(texColor * 0.2));
}

