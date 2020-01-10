#include "MotionBlur.hlsl"
#include "LumaAdapt.hlsl"
#include "Bloom.hlsl"
#include "GBufferGeneration.hlsl"
#include "DepthPassAlphaTest.hlsl"
#include "DepthPass.hlsl"
#include "BilateralBlur.hlsl"
#include "DepthCopy.hlsl"
#include "RSMApply.hlsl"
#include "RSMCapture.hlsl"
#include "RSMUpscale.hlsl"
#include "ScreenSpaceReflection.hlsl"
#include "DirectionalLight.hlsl"
#include "Skybox.hlsl"
#include "SphericalLensFlareFeatures.hlsl"
#include "SSAO.hlsl"
#include "AnamorphicLensFlareBlur.hlsl"
#include "AnamorphicLensFlareFeatures.hlsl"
#include "LensFlareApply.hlsl"
#include "HDRToneMapping.hlsl"
#include "FXAA.hlsl"
#include "ColorCopy.hlsl"
#include "UI.hlsl"