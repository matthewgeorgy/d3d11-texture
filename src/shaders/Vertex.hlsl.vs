struct VS_OUTPUT
{
	float4  Pos : SV_POSITION;
	float2 TexCoord: TEXCOORD;
};

VS_OUTPUT
main(float4 InPos : POSITION,
	 float2 InTexCoord : TEXCOORD)
{
	VS_OUTPUT		Output;


	Output.Pos = InPos;
	Output.TexCoord = InTexCoord;

	return (Output);
}

