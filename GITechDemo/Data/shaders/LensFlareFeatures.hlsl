// Vertex shader /////////////////////////////////////////////////
const float2 f2HalfTexelOffset;

struct VSOut
{
	float4	f4Position	:	SV_POSITION;
	float2	f2TexCoord	:	TEXCOORD0;
};

void vsmain(float4 f4Position : POSITION, float2 f2TexCoord : TEXCOORD, out VSOut output)
{
	output.f4Position = f4Position;
	// Flip texture coordinates horizontally/vertically
	output.f2TexCoord = float2(1.f, 1.f) - (f4Position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) + f2HalfTexelOffset);
}
////////////////////////////////////////////////////////////////////

// Pixel shader ///////////////////////////////////////////////////
const sampler2D	texSource;	// Source texture
const float2 f2TexelSize;	// Size of source texture texel

// Ghost features
const sampler1D texGhostColorLUT;
const int nGhostSamples;
const float fGhostDispersal;
const float fGhostRadialWeightExp;

// Halo feature
const float fHaloSize;
const float fHaloRadialWeightExp;

// Chromatic abberation feature
const bool bChromaShift;
const float fShiftFactor;

// Replacement for tex2D() which adds a
// chromatic abberation effect to texture samples
float4 FetchChromaShiftedTextureSample(sampler2D tex, float2 texCoord)
{
	if (bChromaShift)
	{
		const float3 f2ShiftAmount = float3(
			-f2TexelSize.x * fShiftFactor,
			0.f,
			f2TexelSize.x * fShiftFactor);
		const float2 f2Dir = normalize(float2(0.5f, 0.5f) - texCoord);
		return float4(
			tex2D(tex, texCoord + f2Dir * f2ShiftAmount.r).r,
			tex2D(tex, texCoord + f2Dir * f2ShiftAmount.g).g,
			tex2D(tex, texCoord + f2Dir * f2ShiftAmount.b).b,
			1.f);
	}
	else
		return tex2D(tex, texCoord);
}

// Convert tex2D() calls to FetchChromaShiftedTextureSample() calls
#define tex2D(tex, texCoord) FetchChromaShiftedTextureSample(tex, texCoord)

void psmain(VSOut input, out float4 f4Color : SV_TARGET)
{
	//////////////////////////////////////////////////////////////////////////////
	// Lens flare effect														//
	// http://john-chapman-graphics.blogspot.ro/2013/02/pseudo-lens-flare.html	//
	//////////////////////////////////////////////////////////////////////////////

	f4Color = float4(0.f, 0.f, 0.f, 1.f);

	// Ghost vector to image center
	const float2 f2GhostVec = (float2(0.5f, 0.5f) - input.f2TexCoord) * fGhostDispersal;

	// Generate ghost features
	for (int i = 0; i < nGhostSamples; i++)
	{
		const float2 f2Offset = input.f2TexCoord + f2GhostVec * float(i);
		const float fGhostWeight =
			pow(abs(1.f -
				length(float2(0.5f, 0.5f) - f2Offset) *
				rcp(length(float2(0.5f, 0.5f)))),
				fGhostRadialWeightExp);
		f4Color.rgb += tex2D(texSource, f2Offset).rgb * fGhostWeight;
	}

	// Adjust ghosts' color using a LUT
	f4Color.rgb *= tex1D(texGhostColorLUT, length(float2(0.5f, 0.5f) - input.f2TexCoord) * rcp(length(float2(0.5f, 0.5f)))).rgb;

	// Generate halo feature
	const float2 f2HaloVec = normalize(f2GhostVec) * fHaloSize;
	const float fHaloWeight =
		pow(abs(1.f -
			length(
				float2(0.5f, 0.5f) -
				(input.f2TexCoord + f2HaloVec)
				) *
			rcp(length(float2(0.5f, 0.5f)))),
			fHaloRadialWeightExp);
	f4Color.rgb += tex2D(texSource, input.f2TexCoord + f2HaloVec).rgb * fHaloWeight;
}
////////////////////////////////////////////////////////////////////