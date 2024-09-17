float4 VS(
float4 pos : POSITION ,
float4 normal : NORMAL,
float2 uv : TEXCOORD,
float4 weight : WEIGHT,
min16uint4 index : INDICES
) : SV_POSITION
{
	return pos;
}