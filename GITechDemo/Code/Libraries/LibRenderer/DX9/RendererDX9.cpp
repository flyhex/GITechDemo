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

#include "RendererDX9.h"

#include "VertexFormatDX9.h"
#include "VertexBufferDX9.h"
#include "IndexBufferDX9.h"
#include "TextureDX9.h"
#include "ResourceManagerDX9.h"
#include "RenderStateDX9.h"
#include "SamplerStateDX9.h"
using namespace LibRendererDll;

#include <d3dx9.h>

RendererDX9::RendererDX9()
	: m_pD3D(nullptr)
	, m_pd3dDevice(nullptr)
	, m_bDeviceLost(false)
{}

RendererDX9::~RendererDX9()
{
	ULONG refCount = 0;

	if (m_pd3dDevice)
		refCount = m_pd3dDevice->Release();
	assert(refCount == 0);

	if (m_pD3D)
		refCount = m_pD3D->Release();
	assert(refCount == 0);
}

void RendererDX9::Initialize(void* hWnd)
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	// Create the D3D object, which is needed to create the D3DDevice.
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	assert(m_pD3D);

	// Verify supported adapter modes
	int nMode = 0;
	D3DDISPLAYMODE d3ddm;
	int nMaxAdaptorModes = m_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
	for (nMode = 0; nMode < nMaxAdaptorModes; ++nMode)
	{
		if (FAILED(m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, nMode, &d3ddm)))
		{
			// TODO: Respond to failure of EnumAdapterModes
			assert(false);
			return;
		}

		DeviceCaps::ScreenFormat sf;
		sf.nWidth = d3ddm.Width;
		sf.nHeight = d3ddm.Height;
		sf.nRefreshRate = d3ddm.RefreshRate;
		m_tDeviceCaps.arrSupportedScreenFormats.push_back(sf);
	}

	// Can we get a 32-bit back buffer?
	if (FAILED(m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		D3DFMT_X8R8G8B8,
		FALSE)))
	{
		// TODO: Handle lack of support for a 32-bit back buffer...
		assert(false);
		return;
	}

	// Can we get a z-buffer that's at least 16 bits?
	if (FAILED(m_pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE,
		D3DFMT_D16)))
	{
		// TODO: Handle lack of support for a 16-bit z-buffer...
		assert(false);
		return;
	}

	// Query the device for its capabilities.
	D3DCAPS9 deviceCaps;
	HRESULT hr = m_pD3D->GetDeviceCaps(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		&deviceCaps);
	assert(SUCCEEDED(hr));

	m_tDeviceCaps.bCanAutoGenMipmaps = (deviceCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
	m_tDeviceCaps.bDynamicTextures = (deviceCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) != 0;
	m_tDeviceCaps.bPresentIntervalImmediate = (deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) != 0;
	m_tDeviceCaps.bPresentIntervalOne = (deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) != 0;
	m_tDeviceCaps.bPresentIntervalTwo = (deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO) != 0;
	m_tDeviceCaps.bPresentIntervalThree = (deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE) != 0;
	m_tDeviceCaps.bPresentIntervalFour = (deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR) != 0;
	m_tDeviceCaps.bMrtIndependentBitDepths = (deviceCaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS) != 0;
	m_tDeviceCaps.bMrtPostPixelShaderBlending = (deviceCaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING) != 0;
	m_tDeviceCaps.bAnisotropicFiltering = (deviceCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0;
	m_tDeviceCaps.bDepthBias = (deviceCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) != 0;
	m_tDeviceCaps.bSlopeScaledDepthBias = (deviceCaps.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS) != 0;
	m_tDeviceCaps.bMipmapLodBias = (deviceCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) != 0;
	m_tDeviceCaps.bWBuffer = (deviceCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER) != 0;
	m_tDeviceCaps.bTexturePow2 = (deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0;
	m_tDeviceCaps.nMaxTextureWidth = deviceCaps.MaxTextureWidth;
	m_tDeviceCaps.nMaxTextureHeight = deviceCaps.MaxTextureHeight;
	m_tDeviceCaps.nMaxTextureDepth = deviceCaps.MaxVolumeExtent;
	m_tDeviceCaps.nVertexShaderVersionMajor = D3DSHADER_VERSION_MAJOR(deviceCaps.VertexShaderVersion);
	m_tDeviceCaps.nVertexShaderVersionMinor = D3DSHADER_VERSION_MINOR(deviceCaps.VertexShaderVersion);
	m_tDeviceCaps.nPixelShaderVersionMajor = D3DSHADER_VERSION_MAJOR(deviceCaps.PixelShaderVersion);
	m_tDeviceCaps.nPixelShaderVersionMinor = D3DSHADER_VERSION_MINOR(deviceCaps.PixelShaderVersion);
	m_tDeviceCaps.nNumSimultaneousRTs = deviceCaps.NumSimultaneousRTs;

	// Set up the structure used to create the D3DDevice. Most parameters are
	// zeroed out. We set Windowed to TRUE, since we want to do D3D in a
	// window, and then set the SwapEffect to "discard", which is the most
	// efficient method of presenting the back buffer to the display. And 
	// we request a back buffer format that matches the current desktop display 
	// format.
	ZeroMemory(&m_ePresentParameters, sizeof(m_ePresentParameters));
	m_ePresentParameters.Windowed = TRUE;
	m_ePresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_ePresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	m_ePresentParameters.BackBufferWidth = m_vBackBufferSize[0];
	m_ePresentParameters.BackBufferHeight = m_vBackBufferSize[1];
	m_ePresentParameters.BackBufferCount = 1;
	m_ePresentParameters.EnableAutoDepthStencil = TRUE;
	m_ePresentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_ePresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	
	// Create the Direct3D device. Here we are using the default adapter (most
	// systems only have one, unless they have multiple graphics hardware cards
	// installed) and requesting the HAL (which is saying we want the hardware
	// device rather than a software one). Software vertex processing is 
	// specified since we know it will work on all cards. On cards that support 
	// hardware vertex processing, though, we would see a big performance gain 
	// by specifying hardware vertex processing.

	//################# EXPERIMENTAL FIX ################################
	//-----------------------------------------------------------------------------
	// There is a problem with the function in GTK's graphics library, Cairo:
	//		static inline cairo_fixed_t _cairo_fixed_from_double (double d)
	// It relies on some floating point math that stops working when
	// computations are done with lower accuracy. Windows changes some
	// floating point precision flags when initializing a Direct3D device,
	// causing Cairo to malfunction and stop rendering our window. This
	// behaviour has only been encountered in the 32-bit version of the app.
	// Unfortunately, using D3DCREATE_FPU_PRESERVE may reduce Direct3D performance
	// in double-precision mode and could also cause undefined behavior.
	// http://msdn.microsoft.com/en-us/library/bb172527(VS.85).aspx
#if defined(_WIN32) && !defined(_WIN64)
	DWORD BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE;
#elif defined(_WIN32) && defined(_WIN64)
	DWORD BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
#endif
	//------------------------------------------------------------------------------

	if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hWnd,
		BehaviorFlags,
		&m_ePresentParameters, &m_pd3dDevice)))
	{
		m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hWnd,
			BehaviorFlags,
			&m_ePresentParameters, &m_pd3dDevice);
	}

	assert(m_pd3dDevice);

	m_pResourceManager = new ResourceManagerDX9();
	m_pRenderState = new RenderStateDX9();
	m_pSamplerState = new SamplerStateDX9();

	POP_PROFILE_MARKER();
}

void RendererDX9::SetBackBufferSize(const Vec2i size, const Vec2i offset)
{
	m_bDeviceLost = false;

	// minimized?
	if (!size[0] || !size[1])
	{
		// not technically lost, but shouldn't render
		m_bDeviceLost = true;
		return;
	}

	if (size == m_vBackBufferSize && offset == m_vBackBufferOffset)
		return;

	if (!m_pd3dDevice)
		return;
	
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	HRESULT hr;
	// Get swap chain
	LPDIRECT3DSWAPCHAIN9 sc;
	hr = m_pd3dDevice->GetSwapChain(0, &sc);
	assert(SUCCEEDED(hr));

	// Get present parameters
	D3DPRESENT_PARAMETERS pp;
	hr = sc->GetPresentParameters(&pp);
	assert(SUCCEEDED(hr));

	ULONG refCount = 0;
	refCount = sc->Release();
	assert(refCount == 0);

	// Set back buffer size
	pp.BackBufferWidth = size[0];
	pp.BackBufferHeight = size[1];

	// Unbind resources
	GetResourceManager()->UnbindAll();

	// Reset the device
	hr = m_pd3dDevice->Reset(&pp);
	m_ePresentParameters = pp;
	assert(SUCCEEDED(hr));

	if (SUCCEEDED(hr))
	{
		Renderer::SetBackBufferSize(size, offset);

		// Rebind resources
		GetResourceManager()->BindAll();

		// Reset sampler states
		GetSamplerStateManager()->Reset();

		// Reset render states
		GetRenderStateManager()->Reset();
	}

	POP_PROFILE_MARKER();
}

void RendererDX9::SetViewport(const Vec2i size, const Vec2i offset)
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	D3DVIEWPORT9 vp;
	vp.X = offset[0];
	vp.Y = offset[1];
	vp.Width = size[0];
	vp.Height = size[1];
	vp.MinZ = 0.f;
	vp.MaxZ = 1.f;
	
	HRESULT hr = m_pd3dDevice->SetViewport(&vp);
	assert(SUCCEEDED(hr));

	POP_PROFILE_MARKER();
}

const bool RendererDX9::BeginFrame()
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	HRESULT hr = m_pd3dDevice->TestCooperativeLevel();
	if (hr == D3DERR_DEVICELOST)
	{
		if (!m_bDeviceLost)
			m_bDeviceLost = true;

		Sleep(1);
		POP_PROFILE_MARKER();
		return false;
	}
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		// We need to recreate all resources, apart from shaders and textures,
		// which are managed. However, we recreate everything for safety.
		GetResourceManager()->UnbindAll();

		// Reset the device
		hr = m_pd3dDevice->Reset(&m_ePresentParameters);
		if (FAILED(hr))
		{
			POP_PROFILE_MARKER();
			return false;
		}

		// Rebind resources
		GetResourceManager()->BindAll();

		// Reset sampler states
		GetSamplerStateManager()->Reset();

		// Reset render states
		GetRenderStateManager()->Reset();

		m_bDeviceLost = false;
	}

	if (m_bDeviceLost)
	{
		Sleep(1);
		POP_PROFILE_MARKER();
		return false;
	}

	hr = m_pd3dDevice->BeginScene();
	assert(SUCCEEDED(hr));

	POP_PROFILE_MARKER();

	return true;
}

void RendererDX9::EndFrame()
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	HRESULT hr = m_pd3dDevice->EndScene();
	assert(SUCCEEDED(hr));

	POP_PROFILE_MARKER();
}

void RendererDX9::SwapBuffers()
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	// Present the backbuffer contents to the display
	RECT dstRect;
	dstRect.left = m_vBackBufferOffset[0];
	dstRect.top = m_vBackBufferOffset[1];
	dstRect.right = m_vBackBufferSize[0] + m_vBackBufferOffset[0];
	dstRect.bottom = m_vBackBufferSize[1] + m_vBackBufferOffset[1];

	HRESULT hr = m_pd3dDevice->Present(NULL, &dstRect, NULL, NULL);
	assert(SUCCEEDED(hr) || hr == D3DERR_DEVICELOST);

	POP_PROFILE_MARKER();
}

void RendererDX9::DrawVertexBuffer(VertexBuffer* vb)
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	assert(vb);
	vb->Enable();

	if (vb->GetIndexBuffer())
		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vb->GetElementCount(), 0, vb->GetIndexBuffer()->GetElementCount() / 3);
	else
		m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vb->GetElementCount() / 3);

	vb->Disable();

	POP_PROFILE_MARKER();
}

void RendererDX9::Clear(const Vec4f rgba, const float z, const unsigned int stencil)
{
	PUSH_PROFILE_MARKER(__FUNCSIG__);

	HRESULT hr;
	DWORD flags = D3DCLEAR_TARGET;
	IDirect3DSurface9* depthStencil = nullptr;

	hr = m_pd3dDevice->GetDepthStencilSurface(&depthStencil);
	if (hr != D3DERR_NOTFOUND && depthStencil)
	{
		flags |= D3DCLEAR_ZBUFFER;
		D3DSURFACE_DESC desc;
		memset(&desc, 0, sizeof(desc));
		depthStencil->GetDesc(&desc);
		if (desc.Format == D3DFMT_D24S8)
			flags |= D3DCLEAR_STENCIL;
		depthStencil->Release();
	}

	hr = m_pd3dDevice->Clear(0, NULL, flags, D3DCOLOR_RGBA((DWORD)rgba[0], (DWORD)rgba[1], (DWORD)rgba[2], (DWORD)rgba[3]), 1.0f, 0);
	assert(SUCCEEDED(hr));

	POP_PROFILE_MARKER();
}

void RendererDX9::CreatePerspectiveMatrix(Matrix44f& matProj, float fovYRad, float aspectRatio, float zNear, float zFar)
{
	D3DXMATRIXA16 mat;
	D3DXMatrixPerspectiveFovLH(&mat, fovYRad, aspectRatio, zNear, zFar);

	// Transpose our matrix, making it column-major in order to adhere to the GMTL standard
	matProj(0, 0) = mat._11; matProj(0, 1) = mat._21; matProj(0, 2) = mat._31; matProj(0, 3) = mat._41;
	matProj(1, 0) = mat._12; matProj(1, 1) = mat._22; matProj(1, 2) = mat._32; matProj(1, 3) = mat._42;
	matProj(2, 0) = mat._13; matProj(2, 1) = mat._23; matProj(2, 2) = mat._33; matProj(2, 3) = mat._43;
	matProj(3, 0) = mat._14; matProj(3, 1) = mat._24; matProj(3, 2) = mat._34; matProj(3, 3) = mat._44;
}

void RendererDX9::CreateOrthographicMatrix(Matrix44f& matProj, float left, float top, float right, float bottom, float zNear, float zFar)
{
	D3DXMATRIXA16 mat;
	D3DXMatrixOrthoOffCenterLH(&mat, left, right, bottom, top, zNear, zFar);

	// Transpose our matrix, making it column-major in order to adhere to the GMTL standard
	matProj(0, 0) = mat._11; matProj(0, 1) = mat._21; matProj(0, 2) = mat._31; matProj(0, 3) = mat._41;
	matProj(1, 0) = mat._12; matProj(1, 1) = mat._22; matProj(1, 2) = mat._32; matProj(1, 3) = mat._42;
	matProj(2, 0) = mat._13; matProj(2, 1) = mat._23; matProj(2, 2) = mat._33; matProj(2, 3) = mat._43;
	matProj(3, 0) = mat._14; matProj(3, 1) = mat._24; matProj(3, 2) = mat._34; matProj(3, 3) = mat._44;
}

void RendererDX9::PushProfileMarker(const char* const label)
{
#ifdef ENABLE_PROFILE_MARKERS
	Renderer::PushProfileMarker(label);
	unsigned int len = (unsigned int)strlen(label) + 1;
	wchar_t* labelWide = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, label, -1, labelWide, len);
	D3DPERF_BeginEvent((D3DCOLOR)0xffffffff, labelWide);
	delete[] labelWide;
#endif
}

void RendererDX9::PopProfileMarker()
{
#ifdef ENABLE_PROFILE_MARKERS
	Renderer::PopProfileMarker();
	D3DPERF_EndEvent();
#endif
}
