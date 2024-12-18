#include "ShaderHeader.hlsli"


PixelOutput PS(Output input) : SV_TARGET
{
    PixelOutput output;
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
    float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
    float depthFromLight = lightDepthTex.SampleCmp(shadowSmp, shadowUV, posFromLightVP.z - 0.001f);
    float shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    
    if (input.instNo == 1)
    {
        output.col = float4(0, 0, 0, 1);
        output.normal = float4(0, 0, 0, 1);
        return output;
    }
    float brightness = saturate(dot(normalize(lightVec), input.normal.xyz));
    float4 texCol = texColor.Sample(smp, input.uv);
    float3 refLight = normalize(reflect(-lightVec, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, normalize(input.ray))), specularPower);
    float diffuseB = saturate(dot(normalize(-lightVec), input.normal.xyz));
    float4 toonDif = toon.Sample(smpToon, float2(0, 1.0 - diffuseB));
    float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);
    float4 spa = sphere.Sample(smp, sphereMapUV);

    //spa = saturate(spa);
    //spa.rgb *= spa.a;
    //output.col.xyz = (spa.xyz);
    //output.col.a = 1.0f;
   
	output.col = max(saturate(diffuse * texCol * brightness * shadowWeight * toonDif
    + spa * texCol), float4(diffuse * texCol * ambient, 1.0f));
    
	
    //output.col = spa;
    //max(saturate(float4(diffuse.xyz * brightness * shadowWeight * texCol * toonDif 
    //+ saturate(spa * texCol), 1.0f)),
    //saturate(float4(diffuse.xyz * 0.2, 1.0f)));
    output.normal.rgb = float3((input.normal.xyz + 1.0f) / 2.0f);
    output.normal.a = 1.0f;
    return output;
}

RSMOutput rsmPS(Output input) : SV_TARGET
{
    RSMOutput output;
    float lambert = saturate(dot(normalize(lightVec), input.normal.xyz));
    
    output.world = float4(normalize(input.pos.xyz), 1.0f);
    output.normal = float4(float3((input.normal.xyz + 1.0f) / 2.0f), 1.0f);
    float4 texCol = texColor.Sample(smp, input.uv);
    output.indirectLight = float4(diffuse.xyz * texCol, 1);
    return output;
}


