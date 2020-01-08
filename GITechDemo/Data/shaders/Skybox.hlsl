/*=============================================================================
 * This file is part of the "GITechDemo" application
 * Copyright (C) Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
 *
 *      File:   Skybox.hlsl
 *      Author: Bogdan Iftode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
=============================================================================*/

#include "Common.hlsli"

struct SkyboxConstantTable
{
    CB_float4x4 SkyViewProjMat;
    CB_float3 LightDir;    // Direction of sunlight
    CB_float SunRadius;     // Determines sun radius
    CB_float SunBrightness; // Determines sun brightness
};

#ifdef HLSL
cbuffer SkyboxResourceTable
{
    samplerCUBE SkyboxSkyCube;   // Sky cubemap

    SkyboxConstantTable SkyboxParams;
};

struct VSOut
{
    float4 Position   :   SV_POSITION;
    float3 TexCoord   :   TEXCOORD0;
};

// Vertex shader /////////////////////////////////////////////////
#ifdef VERTEX
void vsmain(float4 position : POSITION, out VSOut output)
{
    output.Position   =   mul(SkyboxParams.SkyViewProjMat, position);
    output.Position.z =   output.Position.w; // Position sky at far plane
    output.TexCoord   =   normalize(position.xyz);
}
#endif // VERTEX
////////////////////////////////////////////////////////////////////

// Pixel shader ///////////////////////////////////////////////////
#ifdef PIXEL
void psmain(VSOut input, out float4 color : SV_TARGET)
{
    // Do a dot product of the view direciton and the sunlight direction
    const float3 sunDir = normalize(-SkyboxParams.LightDir);
    const float sunDot  = dot(normalize(input.TexCoord), sunDir);

    color  = texCUBE(SkyboxSkyCube, input.TexCoord);
    color += pow(max(0.f, sunDot), SkyboxParams.SunRadius) * SkyboxParams.SunBrightness;
}
#endif // PIXEL
////////////////////////////////////////////////////////////////////
#endif // HLSL
