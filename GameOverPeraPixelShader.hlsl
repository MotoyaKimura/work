#include "GameOverPeraHeader.hlsli"

float4 PS(Output input) : SV_TARGET
{
    float4 gameOver = gameOverTex.Sample(smp2, float2((input.uv.x - 0.25) * 2, (input.uv.y - 0.125) * 4));

    gameOver.gb = 0.2;
    gameOver.rgb *= gameOver.a;
    float4 restart = restartTex.Sample(smp2, float2((input.uv.x - 0.4) * 5, (input.uv.y - 0.4) * 10));
    restart.rgb *= restart.a;
    float4 title = titleTex.Sample(smp2, float2((input.uv.x - 0.4) * 5, (input.uv.y - 0.6) * 10));
    title.rgb *= title.a;

    float4 backGround = float4(0.2f, 0.2f, 0.2f, 1.0f);

    if (gameOver.a > 0.1f)
    {
        return float4(backGround.rgb + gameOver.rgb, backGround.a);
    }
    if (restart.a > 0.1f)
    {
        return float4((backGround.rgb + restart * restartHoverCnt * fade).rgb, backGround.a);
    }
    if (title.a > 0.1f)
    {
        return float4((backGround.rgb + title * titleHoverCnt * fade).rgb, backGround.a);
    }

    return float4(0.2f, 0.2f, 0.2f, 1.0f);
}