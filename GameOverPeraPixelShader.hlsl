#include "GameOverPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 gameOver = gameOverTex.Sample(smp, float2((input.uv.x + 0.25) * 2, (input.uv.y + 0.125) * 4));
    gameOver.gb = 0.2;
    gameOver.rgb *= gameOver.a;
    float4 restart = restartTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp, float2((input.uv.x) * 5, (input.uv.y) * 10));
    title.rgb *= title.a;

    float4 backGround = float4(0.2f, 0.2f, 0.2f, 1.0f);
    
    if (input.uv.x > 0.25 && input.uv.x < 0.75 && input.uv.y > 0.175 && input.uv.y < 0.425)
    {
        return float4(backGround.rgb + gameOver.rgb, backGround.a);
    }
    
    if (input.uv.x > 0.4 && input.uv.x < 0.6 && input.uv.y > 0.4 && input.uv.y < 0.5)
    {
        return float4((backGround.rgb + restart * restartHoverCnt * fade).rgb, backGround.a);
    }

    if (input.uv.x > 0.4 && input.uv.x < 0.6 && input.uv.y > 0.6 && input.uv.y < 0.7)
    {
        return float4((backGround.rgb + title * titleHoverCnt * fade).rgb, backGround.a);
    }
    return float4(0.2f, 0.2f, 0.2f, 1.0f);
}