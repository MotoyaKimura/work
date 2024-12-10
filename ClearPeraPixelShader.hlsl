#include "ClearPeraHeader.hlsli"

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float3 calcRSM(float2 uv)
{
    float dp = depthTex.Sample(smp, uv);
    float4 respos = mul(invprojection, float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
    float3 resposDivW = respos.xyz / respos.w;
    float4 rpos = mul(projection, mul(view, float4(resposDivW, 1.0f)));
    rpos.xyz = rpos.xyz / rpos.w;

    float2 lightUVpos = (rpos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
    float w, h, miplevels;
    lightDepthTex.GetDimensions(0, w, h, miplevels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float div1 = 0.0f;
    float div2 = 0.0f;
    float3 indLight = float3(0, 0, 0);

    const int trycnt = 256;
    if (dp < 1.0f)
    {
        for (int i = 0; i < trycnt; ++i)
        {
            float PI = 3.14159265359;
               
            float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
            float rnd2 = random(float2(rnd1, i * dy)) * 2.0f - 1.0f;
            float radius = min(0.5f, random(float2(rnd1, rnd2)));
            float2 sample = lightUVpos + float2(sin(2 * PI * rnd2), cos(2 * PI * rnd2)) * rnd1 * radius;
            float3 lightNorm = normalize(lightNormalTex.Sample(smp, sample).xyz);
            lightNorm = lightNorm * 2 - 1;
            float4 lightWorld = worldTex.Sample(smp, sample);
            float3 dstVec = normalize(respos.xyz - lightWorld.xyz);
            float dstDistance = length(respos.xyz - lightWorld.xyz);
            float dt = dot(lightNorm, dstVec);
            float sgn = sign(dt);
            dt *= sgn;
            div1 += dt;
            if (dot(lightNorm, dstVec) > 0.0f)
            {
                float3 Norm = normalize(normalTex.Sample(smp, uv).xyz);
                Norm = Norm * 2 - 1;
                float dt2 = dot(Norm, -dstVec);
                    
                indLight += indirectLightTex.Sample(smp, sample).rgb * max(0.0f, dt) * max(0.0f, dt2) / max(1.0f, pow(dstDistance, 4));
                sgn = sign(dt2);
                dt2 *= sgn;
                div2 += dt2;
            }
            else
            {
                indLight += float3(0, 0, 0);
            }
        }

        indLight /= max(1.0f, div2);
    }
    return indLight;
}

float4 PS(Output input) : SV_TARGET
{
    float2 dir = float2(1, 1);
    float width, height, miplevels;
    tex.GetDimensions(0, width, height, miplevels);
    float t = dot(input.uv, normalize(dir));
    float PauseCol = 1.0f;
    if (isPause)
        PauseCol = 0.5f;

    //開始時に青幕が右にずれていく
    //if (((width - input.svpos.x) - startWipeRight) < 0)
    //    return float4(0.5f, 0.5f, 1.0f, 1.0f);

    //修了時に�@縦線が降りてくる�A赤幕が右にずれていく
    //float step = fmod(input.svpos.x, 64);
    //if (step < 2)
    //    if ((input.svpos.y - endWipeDown) < 0)
    //        return float4(1.0f, 0.5f, 0.5f, 1.0f);
    //if ((step - endWipeRight) < 0)
    //    return float4(1.0f, 0.5f, 0.5f, 1.0f);
   
    float3 indLight = calcRSM(input.uv);
    float ssao = ssaoTex.Sample(smp, (input.uv));
    float4 texColor = tex.Sample(smp, input.uv);
    float4 clear = clearTex.Sample(smp, float2((input.uv.x + 0.25) * 2, (input.uv.y ) * 4));
    clear.rgb *= clear.a;
    float4 restart = restartTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    title.rgb *= title.a;
    
   
    if (input.uv.x > 0.25 && input.uv.x < 0.75 && input.uv.y > 0.0 && input.uv.y < 0.25)
    {
        return float4((texColor * ssao + clear + indLight).rgb, texColor.a);
    }
    else if(input.uv.x > 0.2 && input.uv.x < 0.4 && input.uv.y > 0.8 && input.uv.y < 0.9)
    {
        return float4((texColor * ssao + restart * restartHoverCnt * fade + indLight).rgb, texColor.a);
    }
    else if (input.uv.x > 0.6 && input.uv.x < 0.8 && input.uv.y > 0.8 && input.uv.y < 0.9)
    {
        return float4((texColor * ssao + title * titleHoverCnt * fade + indLight).rgb, texColor.a);
    }
    
    return float4((texColor * ssao + indLight).rgb, texColor.a);
    
}
