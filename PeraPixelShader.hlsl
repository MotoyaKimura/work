#include "PeraShaderHeader.hlsli"

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float hash(float n)
{
    return frac(sin(n)*43758.5453);
}

//シンプレックスノイズ計算
float SimplexNoise(float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
                     lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
                 lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
                     lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

//numberの数のテクスチャを返す
float4 defineNum(float2 uv, float diffx, int number)
{
    switch (number)
    {
        case 0:
            return zeroTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 1:
            return oneTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 2:
            return twoTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 3:
            return threeTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 4:
            return fourTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 5:
            return fiveTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 6:
            return sixTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 7:
            return sevenTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 8:
            return eightTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        case 9:
            return nineTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
        default:
            return zeroTex.Sample(smp2, float2((uv.x - diffx) * 4, (uv.y - 0.005) * 5));
            break;
    }
}

//ミリ秒を秒に変換
int4 calcMilliToSecond()
{
    int a = 30 * 1000 - milliSecond;
    int ten = a / 10000;
    int b = a % 10000;
    int one = b / 1000;
    int c = b % 1000;
    int tenth = c / 100;
    int d = c % 100;
    int hundredth = d / 10;
    return int4(ten, one, tenth, hundredth);
}


//RSMから間接光を計算
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

    if ((input.svpos.y - startWipeOpen) <= 0 || input.svpos.y + startWipeOpen >= height)
    {
        return float4(0.2f, 0.2f, 0.2f, 1.0f);
    }

    float t = SimplexNoise(input.svpos.xyz);
    t = (t - 0.5f) * 2.0f;
    float2 noise = input.uv + t * 0.05f * (1 - monochromeRate);
   
    float ssao = ssaoTex.Sample(smp, (input.uv));
    float4 texColor = tex.Sample(smp, noise);
    float4 pause = PauseTex.Sample(smp2, (input.uv - float2(0.4, 0.4)) * 5);
    pause.rgb *= pause.a;
    int4 limit = calcMilliToSecond();
    float4 redTime = float4(0, 0, 0, 1);
    if(limit.x == 0)
    {
	    redTime = float4(0.3, 0.0, 0.0, 1.0);
    }
        
    float4 num1 = defineNum(input.uv, 0.68, limit.x);
    float4 num2 = defineNum(input.uv, 0.72, limit.y);
    float4 num3 = defineNum(input.uv, 0.78, limit.z);
    float4 num4 = defineNum(input.uv, 0.82, limit.w);
    
    float4 colon = colonTex.Sample(smp2, float2((input.uv.x - 0.75) * 4, (input.uv.y - 0.005) * 5));
    float4 timer = timerTex.Sample(smp2, float2((input.uv.x - 0.75) * 4, (input.uv.y) * 5));

    float4 color = float4(texColor.rgb * ssao, texColor.a);
    float Y = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    float3 monochromeColor = float3(Y, Y, Y);
    color.xyz = lerp(monochromeColor, color, monochromeRate);
    float pauseWeight = 1.0;
    if (isPause)
    {
        pauseWeight = 0.5;
        if (pause.a > 0.1)
        {
            return pause;
        }
    }
    else
    {
        pauseWeight = 1.0;
    }
  
    if (timer.a > 0.1)
    {
        if(num1.a > 0.1)
            return (num1 + redTime) * pauseWeight * fade * gameOverFade + clearFade;
        else if (num2.a > 0.1)
            return (num2 + redTime) * pauseWeight * fade * gameOverFade + clearFade;
        else if (num3.a > 0.1)
            return (num3 + redTime) * pauseWeight * fade * gameOverFade + clearFade;
        else if (num4.a > 0.1)
            return (num4 + redTime) * pauseWeight * fade * gameOverFade + clearFade;
        else if(colon.a > 0.1)
            return (colon + redTime) * pauseWeight * fade * gameOverFade + clearFade;
        return timer * pauseWeight * fade * gameOverFade + clearFade;
    }


   // float3 indLight = calcRSM(input.uv);
  
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
        
   // return float4(color.xyz * pauseWeight * fade * gameOverFade + clearFade + indLight, color.a);
   return float4(color.xyz * pauseWeight * fade * gameOverFade + clearFade, color.a);
}