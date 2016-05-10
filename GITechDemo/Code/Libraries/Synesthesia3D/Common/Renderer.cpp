/**
 *	@file		Renderer.cpp
 *
 *	@note		This file is part of the "Synesthesia3D" graphics engine
 *
 *	@copyright	Copyright (C) 2014-2015 Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
 *
 *	@copyright
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	@copyright
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *	@copyright
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "RenderState.h"
#include "SamplerState.h"

#include "Utility/Mutex.h"

#ifdef _WINDOWS
#include "RendererDX9.h"
#endif

#include "RendererNULL.h"

using namespace Synesthesia3D;

Renderer* Renderer::ms_pInstance = nullptr;
API Renderer::ms_eAPI = API_NONE;
int Renderer::ms_nProfileMarkerCounter = 0;
static bool bCounterMutexInit = false;

// A mutex to guarantee thread-safety for ms_nProfileMarkerCounter
static MUTEX gCounterMutex;

Renderer::Renderer()
	: m_vBackBufferOffset(0, 0)
	, m_pResourceManager(nullptr)
	, m_pRenderStateManager(nullptr)
	, m_pSamplerStateManager(nullptr)
{
	if (!bCounterMutexInit)
	{
		bCounterMutexInit = true;
		MUTEX_INIT(gCounterMutex);
	}
}

Renderer::~Renderer()
{
	if (m_pResourceManager)
		delete m_pResourceManager;

	if (m_pRenderStateManager)
		delete m_pRenderStateManager;

	if (m_pSamplerStateManager)
		delete m_pSamplerStateManager;

	if (bCounterMutexInit)
	{
		bCounterMutexInit = false;
		MUTEX_DESTROY(gCounterMutex);
	}
}

void Renderer::CreateInstance(API api)
{
	assert(ms_pInstance == nullptr);
	if (ms_pInstance != nullptr)
		return;

	switch (api)
	{
		case API_DX9:
			ms_pInstance = new RendererDX9;
			ms_eAPI = API_DX9;
			break;
		case API_NULL:
			ms_pInstance = new RendererNULL;
			ms_eAPI = API_NULL;
			break;
		default:
			assert(false);
	}
}

void Renderer::DestroyInstance()
{
	if (ms_pInstance)
	{
	#if ENABLE_PROFILE_MARKERS
		assert(ms_nProfileMarkerCounter == 0);
	#endif
		ms_pInstance->GetResourceManager()->ReleaseAll();
		Renderer* tmp = ms_pInstance;
		ms_pInstance = nullptr;
		delete tmp;
	}
}

Renderer* const Renderer::GetInstance()
{
	return ms_pInstance;
}

const API Renderer::GetAPI()
{
	return ms_eAPI;
}

const Vec2i Renderer::GetScreenOffset() const
{
	return m_vBackBufferOffset;
}

void Renderer::ValidateScreenResolution(Vec2i& size) const
{
	if (GetAPI() == API_NULL)
		return;

	Vec2i bestMatch(0, 0);
	for (unsigned int i = 0; i < m_tDeviceCaps.arrSupportedScreenFormats.size(); i++)
	{
		if (GetBackBufferFormat() == m_tDeviceCaps.arrSupportedScreenFormats[i].ePixelFormat)
		{
			if (((int)m_tDeviceCaps.arrSupportedScreenFormats[i].nWidth > bestMatch[0] &&
				(int)m_tDeviceCaps.arrSupportedScreenFormats[i].nWidth <= size[0]) ||
				((int)m_tDeviceCaps.arrSupportedScreenFormats[i].nHeight > bestMatch[1] &&
					(int)m_tDeviceCaps.arrSupportedScreenFormats[i].nHeight <= size[1]))
			{
				bestMatch[0] = m_tDeviceCaps.arrSupportedScreenFormats[i].nWidth;
				bestMatch[1] = m_tDeviceCaps.arrSupportedScreenFormats[i].nHeight;
			}
		}
	}

	size = bestMatch;
}

void Renderer::ConvertOGLProjMatToD3D(Matrix44f& matProj)
{
	matProj[2][2] = matProj[2][2] * -1 - (matProj[2][2] + 1) / -2.f;
	matProj[3][2] *= -1;
	matProj[2][3] *= 0.5f;
}

void Renderer::ConvertOGLProjMatToD3D(Matrix44f* const matProj)
{
	ConvertOGLProjMatToD3D(*matProj);
}


ResourceManager* const Renderer::GetResourceManager() const
{
	return m_pResourceManager;
}

RenderState* const Renderer::GetRenderStateManager() const
{
	return m_pRenderStateManager;
}

SamplerState* const Renderer::GetSamplerStateManager() const
{
	return m_pSamplerStateManager;
}

const DeviceCaps& Renderer::GetDeviceCaps() const
{
	return m_tDeviceCaps;
}

void Renderer::DrawVertexBuffer(VertexBuffer* const vb)
{
	GetSamplerStateManager()->Flush();
	GetRenderStateManager()->Flush();
}

void Renderer::PushProfileMarker(const char* const /*label*/)
{
	MUTEX_LOCK(gCounterMutex);
	ms_nProfileMarkerCounter++;
	MUTEX_UNLOCK(gCounterMutex);
}

void Renderer::PopProfileMarker()
{
	MUTEX_LOCK(gCounterMutex);
	ms_nProfileMarkerCounter--;
	assert(ms_nProfileMarkerCounter >= 0);
	MUTEX_UNLOCK(gCounterMutex);
}
