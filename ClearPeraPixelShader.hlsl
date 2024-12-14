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
   
    float3 indLight = calcRSM(input.uv);
    float ssao = ssaoTex.Sample(smp, (input.uv));
    float4 texColor = tex.Sample(smp, input.uv);
    float4 clear = clearTex.Sample(smp2, float2((input.uv.x - 0.25) * 2, (input.uv.y) * 4));
    clear.rgb *= clear.a;
    float4 restart = restartTex.Sample(smp2, float2((input.uv.x - 0.2) * 5, (input.uv.y - 0.8) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp2, float2((input.uv.x - 0.6) * 5, (input.uv.y - 0.8) * 10));
    title.rgb *= title.a;
    float4 credit1 = creditTex1.Sample(smp2, float2(input.uv.x, input.uv.y - 1.0 + creditCnt));
    credit1.rgb *= credit1.a;
    float4 credit2 = creditTex2.Sample(smp2, float2(input.uv.x, input.uv.y - 2.0 + creditCnt));
    credit2.rgb *= credit2.a;

    if(credit1.a > 0.1f)
    {
        return credit1;
    }
    if(credit2.a > 0.1f)
    {
        return credit2;
    }
    //if (input.uv.x < 0.2 && input.uv.y < 0.4 && input.uv.y > 0.2)
    //{
    //    float dep = pow(depthTex.Sample(smp, input.uv * 5), 10);
    //    dep = 1 - dep;
    //    return float4(dep, dep, dep, 1);
    //}
    //else if (input.uv.x < 0.2 && input.uv.y < 0.6 && input.uv.y > 0.2)
    //{
    //    return normalTex.Sample(smp, input.uv * 5);
    //}
    //else if (input.uv.x < 0.2 && input.uv.y < 0.8 && input.uv.y > 0.2)
    //{
    //    float s = pow(ssaoTex.Sample(smp, (input.uv - float2(0, 0.6)) * 5), 10);
    //    return float4(s, s, s, 1);
    //}
    //else if (input.uv.x > 0.2 && input.uv.x < 0.4 && input.uv.y < 0.2)
    //{
    //    float lightDep = lightDepthTex.Sample(smp, input.uv * 5);
    //    lightDep = 1 - lightDep;
    //    return float4(lightDep, lightDep, lightDep, 1);
    //}
    //else if (input.uv.x > 0.2 && input.uv.x < 0.6 && input.uv.y < 0.2)
    //{
    //    return worldTex.Sample(smp, input.uv * 5);
    //}
    //else if (input.uv.x > 0.2 && input.uv.x < 0.8 && input.uv.y < 0.2)
    //{
    //    return lightNormalTex.Sample(smp, input.uv * 5);
    //}
    //else if (input.uv.x > 0.2 && input.uv.x < 1.0 && input.uv.y < 0.2)
    //{
    //    return indirectLightTex.Sample(smp, input.uv * 5);
    //}
       
    
    return float4((texColor * ssao + clear +
    restart * restartHoverCnt * fade +
    title * titleHoverCnt * fade + gameOverFade).rgb, texColor.a);
}
