#include "PeraShaderHeader.hlsli"

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float4 PS(Output input) : SV_TARGET
{
    //float s = ssaoTex.Sample(smp, (input.uv));
    //float4 texColor = tex.Sample(smp, input.uv);
    //return float4(texColor.rgb * s, texColor.a);

	if(input.uv.x < 0.2 && input.uv.y < 0.4 && input.uv.y > 0.2)
    {
        float dep = pow(depthTex.Sample(smp, input.uv * 5), 50);
        dep = 1 - dep;
        return float4(dep, dep, dep, 1);
    }
    else if (input.uv.x < 0.2 && input.uv.y < 0.6 && input.uv.y > 0.2)
    {
        return normalTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x < 0.2 && input.uv.y < 0.8 && input.uv.y > 0.2)
    {
        float s = pow(ssaoTex.Sample(smp, (input.uv - float2(0, 0.6)) * 5), 10);
        return float4(s, s, s, 1);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.4 && input.uv.y < 0.2)
    {
        float lightDep = lightDepthTex.Sample(smp, input.uv * 5);
        lightDep = 1 - lightDep;
        return float4(lightDep, lightDep, lightDep, 1);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.6 && input.uv.y < 0.2)
    {
        return worldTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 0.8 && input.uv.y < 0.2)
    {
        return lightNormalTex.Sample(smp, input.uv * 5);
    }
    else if (input.uv.x > 0.2 && input.uv.x < 1.0 && input.uv.y < 0.2)
    {
        return indirectLightTex.Sample(smp, input.uv * 5);
    }
    else
    {
        float dp = depthTex.Sample(smp, input.uv);
        float4 respos = mul(invprojection, float4(input.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
        float3 resposDivW = respos.xyz / respos.w;
        //respos.xyz /= respos.w;
       // return float4(respos.xyz, 1);
        float4 rpos = mul(projection, mul(view, float4(resposDivW, 1.0f)));
        rpos.xyz = rpos.xyz / rpos.w;
        
        float2 lightUVpos = (rpos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
        //return indirectLightTex.Sample(smp, lightUVpos);
        float lightdp = lightDepthTex.Sample(smp, input.uv);
        float w, h, miplevels;
        lightDepthTex.GetDimensions(0, w, h, miplevels);
        float dx = 1.0f / w;
        float dy = 1.0f / h;
        float div = 0.0f;
        float3 indLight = float3(0,0,0);

        const int trycnt = 256;
        if (dp < 1.0f)
        {
            for (int i = 0; i < trycnt; ++i)
            {
                float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
                float rnd2 = random(float2(rnd1, i * dy)) * 2.0f - 1.0f;
                float2 sample = lightUVpos + float2(rnd1, rnd2) * 0.1;
                float3 lightNorm = normalize(lightNormalTex.Sample(smp, sample).xyz);
                
                float4 lightWorld = worldTex.Sample(smp, sample);
                lightWorld.xyz = lightWorld.xyz / lightWorld.w;
                float3 dstVec = normalize(respos.xyz - lightWorld.xyz);
                float dstDistance = length(respos.xyz - lightWorld.xyz);
                float dt = dot(lightNorm, dstVec);
                float sgn = sign(dt);
                dt *= sgn;
                div += dt;
                if(dot(lightNorm, dstVec) > 0.0f)
                {

                    float3 Norm = normalize(normalTex.Sample(smp, input.uv).xyz);
                    float dt2 = dot(Norm, -dstVec);
                    sgn = sign(dt2);
                    dt2 *= sgn;
                    div += dt2;
                    if (dot(Norm, -dstVec) > 0.0f)
                		indLight += indirectLightTex.Sample(smp, sample).rgb * dt * dt2 / pow(dstDistance, 4);
                    else
                        indLight += float3(0,0,0);
                }
                
            }
            indLight /= div;
        }

        float s = max(ssaoTex.Sample(smp, (input.uv)), 0.7);
        float4 texColor = tex.Sample(smp, input.uv);
    	return float4(indLight, texColor.a);
    }
}