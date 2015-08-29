/**
\file SSAO_minify.pix
\author Morgan McGuire and Michael Mara, NVIDIA Research
DX11 HLSL port by Leonardo Zide, Tryearch

Open Source under the "BSD" license: http://www.opensource.org/licenses/bsd-license.php

Copyright (c) 2011-2012, NVIDIA
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D<float> g_source : register(t0);

float main(GSOutput input) : SV_TARGET
{
	//int2 ssP = input.texCoords * float2(renderTargetSize[SIZECONST_WIDTH], renderTargetSize[SIZECONST_HEIGHT]);
	int2 ssP = int2(input.pos.xy);

	// Rotated grid subsampling to avoid XY directional bias or Z precision bias while downsampling
	float color = g_source.Load(int3(ssP * 2 + int2((ssP.y & 1) ^ 1, (ssP.x & 1) ^ 1), 0)).r;

	return color;
}