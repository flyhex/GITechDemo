//////////////////////////////////////////////////////////////////////////
// This file is part of the "LibRenderer" 3D graphics library           //
//                                                                      //
// Copyright (C) 2014 - Iftode Bogdan-Marius <iftode.bogdan@gmail.com>  //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program. If not, see <http://www.gnu.org/licenses/>. //
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "RendererNULL.h"
#include "ResourceManagerNULL.h"
#include "RenderStateNULL.h"
#include "SamplerStateNULL.h"
using namespace LibRendererDll;

void RendererNULL::Initialize(void* /*hWnd*/)
{
	m_pResourceManager = new ResourceManagerNULL();
	m_pRenderState = new RenderStateNULL();
	m_pSamplerState = new SamplerStateNULL();
}

void RendererNULL::SetBackBufferSize(const Vec2i size, const Vec2i offset)
{
	Renderer::SetBackBufferSize(size, offset);
}

void RendererNULL::CreatePerspectiveMatrix(Matrix44f& matProj, float fovYRad, float aspectRatio, float zNear, float zFar)
{
	gmtl::setPerspective(matProj, gmtl::Math::rad2Deg(fovYRad), aspectRatio, zNear, zFar);
}

void RendererNULL::CreateOrthographicMatrix(Matrix44f& matProj, float left, float top, float right, float bottom, float zNear, float zFar)
{
	gmtl::setOrtho(matProj, left, top, right, bottom, zNear, zFar);
}

void RendererNULL::PushProfileMarker(const char* const label)
{
	Renderer::PushProfileMarker(label);
}

void RendererNULL::PopProfileMarker()
{
	Renderer::PopProfileMarker();
}
