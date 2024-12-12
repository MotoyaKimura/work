#include "PeraShaderHeader.hlsli"

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float hash(float n)
{
    return frac(sin(n)*43758.5453);
}

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
    float4 pause = PauseTex.Sample(smp, input.uv * 5);
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
        
      //  float4 startTexColor = startTex.Sample(smp, float2((input.uv.x + 0.05) * 10, (input.uv.y) * 10));
        //startTexColor.rgb *= startTexColor.a;
    float4 color = float4(texColor.rgb * ssao, texColor.a);
    float Y = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    float3 monochromeColor = float3(Y, Y, Y);
    color.xyz = lerp(monochromeColor, color, monochromeRate);

    if (isPause)
    {
        if (timer.a > 0.1)
        {
            if (num1.a > 0.1)
                return (num1 + redTime) * 0.5 * fade * gameOverFade + clearFade;
            else if (num2.a > 0.1)
                return (num2 + redTime) * 0.5 * fade * gameOverFade + clearFade;
            else if (num3.a > 0.1)
                return (num3 + redTime) * 0.5 * fade * gameOverFade + clearFade;
            else if (num4.a > 0.1)
                return (num4 + redTime) * 0.5 * fade * gameOverFade + clearFade;
            else if (colon.a > 0.1)
                return (colon + redTime) * 0.5 * fade * gameOverFade + clearFade;
            return timer * 0.5 * fade * gameOverFade + clearFade;
        }
        if (input.uv.x > 0.4 && input.uv.x < 0.6 && input.uv.y > 0.4 && input.uv.y < 0.6)
        {
            return float4((color.xyz * 0.5 + pause.rgb) * fade * gameOverFade + clearFade, color.a);
        }
        return float4((color.xyz * 0.5) * fade * gameOverFade + clearFade, color.a);
    }
    if (timer.a > 0.1)
    {
        if(num1.a > 0.1)
            return (num1 + redTime) * fade * gameOverFade + clearFade;
        else if (num2.a > 0.1)
            return (num2 + redTime) * fade * gameOverFade + clearFade;
        else if (num3.a > 0.1)
            return (num3 + redTime) * fade * gameOverFade + clearFade;
        else if (num4.a > 0.1)
            return (num4 + redTime) * fade * gameOverFade + clearFade;
        else if(colon.a > 0.1)
            return (colon + redTime) * fade * gameOverFade + clearFade;
        return timer * fade * gameOverFade + clearFade;
    }
        
    return float4(color.xyz * fade * gameOverFade + clearFade, color.a);
}