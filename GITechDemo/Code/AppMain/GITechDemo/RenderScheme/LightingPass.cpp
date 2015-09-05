#include "stdafx.h"

#include <Renderer.h>
#include <RenderState.h>
#include <ResourceManager.h>
#include <Texture.h>
#include <RenderTarget.h>
using namespace LibRendererDll;

#include "LightingPass.h"
using namespace GITechDemoApp;

#include "RenderResourcesDef.h"

LightingPass::LightingPass(const char* const passName, RenderPass* const parentPass)
	: RenderPass(passName, parentPass)
{}

LightingPass::~LightingPass()
{}

void LightingPass::OnUpdate(const float fDeltaTime)
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	ResourceManager* ResourceMgr = RenderContext->GetResourceManager();
	if (!ResourceMgr)
		return;

	ResourceMgr->GetTexture(
		LightAccumulationBuffer.GetRenderTarget()->GetColorBuffer(0)
		)->SetFilter(SF_MIN_MAG_POINT_MIP_NONE);
	ResourceMgr->GetTexture(
		LightAccumulationBuffer.GetRenderTarget()->GetColorBuffer(0)
		)->SetAddressingMode(SAM_CLAMP);

	f2HalfTexelOffset = Vec2f(
		0.5f / GBuffer.GetRenderTarget()->GetWidth(),
		0.5f / GBuffer.GetRenderTarget()->GetHeight()
		);
}

// This function copy-resolves the INTZ depth texture we use when generating the
// G-Buffer (since we can sample it like a regular texture) to the D24S8 depth
// surface of the light accumulation buffer for depth testing (e.g. when rendering the sky)
// Having the correct depth allows us to do some more optimizations which also make use of
// the stencil (e.g. rendering a cone corresponding to the spot light and marking the pixels
// that pass the depth test, using the stencil buffer, so that we only calculate the light
// for those pixels and thus reducing pointless shading of pixels which are not lit)
void LightingPass::CopyDepthBuffer()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	PUSH_PROFILE_MARKER("Copy-resolve Depth Buffer");

	bool red, blue, green, alpha;
	const bool zWrite = RenderContext->GetRenderStateManager()->GetZWriteEnabled();
	const Cmp zFunc = RenderContext->GetRenderStateManager()->GetZFunc();
	RenderContext->GetRenderStateManager()->GetColorWriteEnabled(red, green, blue, alpha);

	texSource = GBuffer.GetRenderTarget()->GetDepthBuffer();

	RenderContext->GetRenderStateManager()->SetColorWriteEnabled(false, false, false, false);
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(true);
	RenderContext->GetRenderStateManager()->SetZFunc(CMP_ALWAYS);

	DepthCopyShader.Enable();
	RenderContext->DrawVertexBuffer(FullScreenTri);
	DepthCopyShader.Disable();

	RenderContext->GetRenderStateManager()->SetColorWriteEnabled(red, green, blue, alpha);
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(zWrite);
	RenderContext->GetRenderStateManager()->SetZFunc(zFunc);

	POP_PROFILE_MARKER();
}

void LightingPass::OnDraw()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	LightAccumulationBuffer.Enable();

	RenderContext->Clear(Vec4f(0.f, 0.f, 0.f, 0.f), 1.f, 0);

	// Copy-resolve the depth buffer for later usage
	CopyDepthBuffer();

	const bool zWrite = RenderContext->GetRenderStateManager()->GetZWriteEnabled();
	const Cmp zFunc = RenderContext->GetRenderStateManager()->GetZFunc();
	const bool blendEnabled = RenderContext->GetRenderStateManager()->GetColorBlendEnabled();
	const Blend DstBlend = RenderContext->GetRenderStateManager()->GetColorDstBlend();
	const Blend SrcBlend = RenderContext->GetRenderStateManager()->GetColorSrcBlend();

	// Disable Z writes, since we already have the correct depth buffer
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(false);
	RenderContext->GetRenderStateManager()->SetZFunc(CMP_ALWAYS);
	
	// Additive color blending is required for accumulating light
	RenderContext->GetRenderStateManager()->SetColorBlendEnabled(true);
	RenderContext->GetRenderStateManager()->SetColorDstBlend(BLEND_ONE);
	RenderContext->GetRenderStateManager()->SetColorSrcBlend(BLEND_ONE);

	DrawChildren();

	// Reset the render states
	RenderContext->GetRenderStateManager()->SetZWriteEnabled(zWrite);
	RenderContext->GetRenderStateManager()->SetZFunc(zFunc);
	RenderContext->GetRenderStateManager()->SetColorBlendEnabled(blendEnabled);
	RenderContext->GetRenderStateManager()->SetColorDstBlend(DstBlend);
	RenderContext->GetRenderStateManager()->SetColorSrcBlend(SrcBlend);

	LightAccumulationBuffer.Disable();
}
