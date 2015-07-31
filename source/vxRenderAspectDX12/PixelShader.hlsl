struct VSOUT
{
	float4 position : SV_POSITION;
};

struct PSOUT
{
	float4 color : SV_TARGET0;
};

PSOUT main(VSOUT vsout)
{
	PSOUT psout;
	psout.color = float4(1, 1, 1, 1);

	return psout;
}