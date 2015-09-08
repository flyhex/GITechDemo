#include "stdafx.h"

#include <Renderer.h>
#include <RenderState.h>
#include <Texture.h>
#include <ResourceManager.h>
#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <VertexFormat.h>
using namespace LibRendererDll;

#include "GITechDemo.h"

#include "SkyPass.h"
using namespace GITechDemoApp;

#include "RenderResourcesDef.h"

SkyPass::SkyPass(const char* const passName, RenderPass* const parentPass)
	: RenderPass(passName, parentPass)
	, m_pSkyBoxCube(nullptr)
	, m_nSkyBoxCubeIdx(~0u)
{}

SkyPass::~SkyPass()
{}

void SkyPass::CreateSkyBoxVB()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	ResourceManager* ResourceMgr = RenderContext->GetResourceManager();
	if (!ResourceMgr)
		return;

	// Create a cube VB for sky
	// The cube is aligned with world axes so that the camera can rotate freely,
	// giving the impressiong that the sky is stationary. Also, the center of the
	// cube is positioned at the camera's location so as not to break the illusion
	// that the sky "hemisphere" is actually a giant cube.
	// NB: in the pixel shader responsible for drawing the sky, the visible face(s)
	// are pushed back to the maximum depth (far plane) so as not to be drawn
	// over objects that are not inside the cube (bigger depth).
	const unsigned int vfIdx = ResourceMgr->CreateVertexFormat(1, VAU_POSITION, VAT_FLOAT4, 0);
	VertexFormat* vf = ResourceMgr->GetVertexFormat(vfIdx);

	const unsigned int ibIdx = ResourceMgr->CreateIndexBuffer(36);
	IndexBuffer* ib = ResourceMgr->GetIndexBuffer(ibIdx);
	const unsigned int sbIndices[] = {
		0, 1, 2, 2, 1, 3,	// front
		5, 4, 7, 7, 4, 6,	// back
		4, 0, 6, 6, 0, 2,	// left
		1, 5, 3, 3, 5, 7,	// right
		4, 5, 0, 0, 5, 1,	// top
		2, 3, 6, 6, 3, 7	// bottom
	};
	ib->Lock(BL_WRITE_ONLY);
	ib->SetIndices(sbIndices, 36);
	ib->Update();
	ib->Unlock();

	m_nSkyBoxCubeIdx = ResourceMgr->CreateVertexBuffer(vf, 8, ib);
	m_pSkyBoxCube = ResourceMgr->GetVertexBuffer(m_nSkyBoxCubeIdx);

	m_pSkyBoxCube->Lock(BL_WRITE_ONLY);
	m_pSkyBoxCube->Position<Vec4f>(0) = Vec4f(-1.f, 1.f, 1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(1) = Vec4f(1.f, 1.f, 1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(2) = Vec4f(-1.f, -1.f, 1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(3) = Vec4f(1.f, -1.f, 1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(4) = Vec4f(-1.f, 1.f, -1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(5) = Vec4f(1.f, 1.f, -1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(6) = Vec4f(-1.f, -1.f, -1.f, 1.f);
	m_pSkyBoxCube->Position<Vec4f>(7) = Vec4f(1.f, -1.f, -1.f, 1.f);
	m_pSkyBoxCube->Update();
	m_pSkyBoxCube->Unlock();
}

void SkyPass::ReleaseSkyBoxVB()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (RenderContext)
	{
		ResourceManager* ResourceMgr = RenderContext->GetResourceManager();
		if (ResourceMgr && m_nSkyBoxCubeIdx != ~0u)
			ResourceMgr->ReleaseVertexBuffer(m_nSkyBoxCubeIdx);
	}

	m_nSkyBoxCubeIdx = ~0u;
	m_pSkyBoxCube = nullptr;
}

void SkyPass::OnUpdate(const float fDeltaTime)
{
	if (!AppMain)
		return;

	f44SkyViewProjMat = f44ViewProjMat * makeTrans(-((GITechDemo*)AppMain)->GetCamera().vPos, Type2Type<Matrix44f>());

	SkyTexture.GetTexture()->SetFilter(SF_MIN_MAG_LINEAR_MIP_LINEAR);
	SkyTexture.GetTexture()->SetSRGBEnabled(true);

	texSkyCube = SkyTexture.GetTextureIndex();
}

// Draw a world axis-aligned cube with its' center coinciding with the camera's position.
// The vertex shader sets the visible face(s) at max depth so as to not draw over
// other objects that aren't inside the cube (which, at 2x2x2, is quite small)
void SkyPass::OnDraw()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	const bool blendEnabled = RenderContext->GetRenderStateManager()->GetColorBlendEnabled();
	const Blend DstBlend = RenderContext->GetRenderStateManager()->GetColorDstBlend();
	const Blend SrcBlend = RenderContext->GetRenderStateManager()->GetColorSrcBlend();
	const bool zWriteEnable = RenderContext->GetRenderStateManager()->GetZWriteEnabled();
	const Cmp zFunc = RenderContext->GetRenderStateManager()->GetZFunc();

	RenderContext->GetRenderStateManager()->SetColorBlendEnabled(true);
	RenderContext->GetRenderStateManager()->SetColorDstBlend(BLEND_ZERO);
	RenderContext->GetRenderStateManager()->SetColorSrcBlend(BLEND_ONE);
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(false);
	RenderContext->GetRenderStateManager()->SetZFunc(CMP_LESSEQUAL);

	SkyBoxShader.Enable();
	RenderContext->DrawVertexBuffer(m_pSkyBoxCube);
	SkyBoxShader.Disable();

	RenderContext->GetRenderStateManager()->SetColorBlendEnabled(blendEnabled);
	RenderContext->GetRenderStateManager()->SetColorDstBlend(DstBlend);
	RenderContext->GetRenderStateManager()->SetColorSrcBlend(SrcBlend);
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(zWriteEnable);
	RenderContext->GetRenderStateManager()->SetZFunc(zFunc);
}