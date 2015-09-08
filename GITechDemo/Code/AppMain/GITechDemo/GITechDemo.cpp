#include "stdafx.h"

#include <time.h>
#include <sstream>

#include <Renderer.h>
#include <ResourceManager.h>
#include <Texture.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>
#include <VertexFormat.h>
using namespace LibRendererDll;

#include "Framework.h"
using namespace AppFramework;

#include "GITechDemo.h"
#include "RenderResourcesDef.h"
#include "RenderScheme.h"
#include "PerlinNoise.h"
#include "ArtistParameter.h"
#include "SkyPass.h"
#include "HUDPass.h"
using namespace GITechDemoApp;

CREATE_APP(GITechDemo)

namespace GITechDemoApp
{
	static bool bExtraResInit = false;

	extern SkyPass SKY_PASS;
	extern HUDPass HUD_PASS;
	extern const char* const ResourceTypeMap[RenderResource::RES_MAX];
}

namespace GITechDemoApp
{
	bool CAMERA_ANIMATION_ENABLED = true;
	int CAMERA_ANIMATION_TIMEOUT_SECONDS = 30;

	bool DIRECTIONAL_LIGHT_ANIMATION_ENABLED = true;

	Perlin PerlinNoise(1, USHRT_MAX, 1, (int)time(NULL));

	extern AABoxf SceneAABB;

	float CAMERA_MOVE_SPEED = 15.f;
	float CAMERA_SPEED_UP_FACTOR = 5.f;
	float CAMERA_SLOW_DOWN_FACTOR = 0.1f;
	float CAMERA_ROTATE_SPEED = 75.f;

}

GITechDemo::GITechDemo()
	: App()
	, m_fDeltaTime(0.f)
	, m_pHWND(nullptr)
	, m_pInputMap(nullptr)
{
	MUTEX_INIT(mResInitMutex);
}

GITechDemo::~GITechDemo()
{
	MUTEX_DESTROY(mResInitMutex);
}

bool GITechDemo::Init(void* hWnd)
{
	m_pHWND = hWnd;

	// Renderer MUST be initialized on the SAME thread as the target window
	Renderer::CreateInstance(API_DX9);
	//Renderer::CreateInstance(API_NULL);

	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return false;

	RenderContext->Initialize(hWnd);

	ResourceManager* ResourceMgr = RenderContext->GetResourceManager();
	if (!ResourceMgr)
		return false;

	// Set initial camera position
	m_tCamera.vPos = Vec3f(-840.f, -600.f, -195.f);
	m_tCamera.mRot.set(
		-0.440301329f, 0.00776965916f, 0.897806108f, 0.f,
		-0.142924204f, 0.986597657f, -0.0786283761f, 0.f,
		-0.886387110f, -0.162937075f, -0.433295786f, 0.f,
		0.f, 0.f, 0.f, 1.f
		);

	ArtistParameterManager::CreateInstance();

	// Setup Gainput
	if (m_pInputManager)
	{
		gainput::DeviceId mouseId = m_pInputManager->CreateDevice<gainput::InputDeviceMouse>();
		gainput::DeviceId keyboardId = m_pInputManager->CreateDevice<gainput::InputDeviceKeyboard>(gainput::InputDevice::AutoIndex, gainput::InputDevice::DV_RAW);

		m_pInputMap = new gainput::InputMap(*m_pInputManager);

		if (m_pInputMap && mouseId != gainput::InvalidDeviceId && keyboardId != gainput::InvalidDeviceId)
		{
			// Key bindings
			m_pInputMap->MapBool(APP_CMD_CTRL_YAW, mouseId, gainput::MouseButtonLeft);
			m_pInputMap->MapBool(APP_CMD_CTRL_PITCH, mouseId, gainput::MouseButtonLeft);
			m_pInputMap->MapBool(APP_CMD_CTRL_ROLL, mouseId, gainput::MouseButtonRight);
			m_pInputMap->MapFloat(APP_CMD_YAW_AXIS, mouseId, gainput::MouseAxisX);
			m_pInputMap->MapFloat(APP_CMD_PITCH_AXIS, mouseId, gainput::MouseAxisY);
			m_pInputMap->MapFloat(APP_CMD_ROLL_AXIS, mouseId, gainput::MouseAxisX);
			m_pInputMap->MapBool(APP_CMD_FORWARD, keyboardId, gainput::KeyW);
			m_pInputMap->MapBool(APP_CMD_BACKWARD, keyboardId, gainput::KeyS);
			m_pInputMap->MapBool(APP_CMD_LEFT, keyboardId, gainput::KeyA);
			m_pInputMap->MapBool(APP_CMD_RIGHT, keyboardId, gainput::KeyD);
			m_pInputMap->MapBool(APP_CMD_SPEED_UP, keyboardId, gainput::KeyShiftL);
			m_pInputMap->MapBool(APP_CMD_SLOW_DOWN, keyboardId, gainput::KeyCtrlL);

			// Setup the artist parameter manager key bindings
			ArtistParameterManager* const pAPM = ArtistParameterManager::GetArtistParameterManager();
			if (pAPM)
				pAPM->SetupInput(m_pInputManager);
		}
	}

	RenderResource::SetResourceManager(ResourceMgr);

	return true;
}

void GITechDemo::Release()
{
	if (m_pInputMap)
	{
		delete m_pInputMap;
		m_pInputMap = nullptr;
	}

	bExtraResInit = false;
	FullScreenTri = nullptr;
	SKY_PASS.ReleaseSkyBoxVB();
	HUD_PASS.ReleaseHUDTexture();

	RenderResource::FreeAll();
	ArtistParameterManager::DestroyInstance();
	Renderer::DestroyInstance();
}

void GITechDemo::LoadResources(unsigned int thId, unsigned int thCount)
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	ResourceManager* ResourceMgr = RenderContext->GetResourceManager();
	if (!ResourceMgr)
		return;

	bool bAllInitialized = false;
	do
	{
		bAllInitialized = true;
		const vector<RenderResource*>& resList = RenderResource::GetResourceList();
		for (unsigned int i = 0; i < resList.size(); i++)
		{
			if (!resList[i]->IsInitialized())
			{
				bAllInitialized = false;
				if (resList[i]->TryLockRes())
				{
					std::stringstream msg;
					msg << "Thread " << thId << " - ";
					msg << ResourceTypeMap[resList[i]->GetResourceType()] << ": \"" << resList[i]->GetDesc() << "\"";
					cout << msg.str() + " (start)\n";
					resList[i]->Init();
					cout << msg.str() + " (finish)\n";
					resList[i]->UnlockRes();
				}
			}
		}
	} while (!bAllInitialized);

	// Allow only the first thread to get here to initialize the rest of the resources
	if (MUTEX_TRYLOCK(mResInitMutex))
	{
		if (!bExtraResInit)
		{
			// Create a full screen quad (it's actually an over-sized triangle) for fullscreen effects and processing
			unsigned int vfIdx = ResourceMgr->CreateVertexFormat(1, VAU_POSITION, VAT_FLOAT4, 0);
			VertexFormat* vf = ResourceMgr->GetVertexFormat(vfIdx);

			unsigned int ibIdx = ResourceMgr->CreateIndexBuffer(3);
			IndexBuffer* ib = ResourceMgr->GetIndexBuffer(ibIdx);
			const unsigned int fsqIndices[] = { 0, 1, 2 };
			ib->SetIndices(fsqIndices, 3);

			unsigned int vbIdx = ResourceMgr->CreateVertexBuffer(vf, 3, ib);
			FullScreenTri = ResourceMgr->GetVertexBuffer(vbIdx);

			FullScreenTri->Lock(BL_WRITE_ONLY);
			FullScreenTri->Position<Vec4f>(0) = Vec4f(-1.f, 1.f, 1.f, 1.f);
			FullScreenTri->Position<Vec4f>(1) = Vec4f(3.f, 1.f, 1.f, 1.f);
			FullScreenTri->Position<Vec4f>(2) = Vec4f(-1.f, -3.f, 1.f, 1.f);
			FullScreenTri->Update();
			FullScreenTri->Unlock();

			SKY_PASS.CreateSkyBoxVB();

			bExtraResInit = true;
		}
		MUTEX_UNLOCK(mResInitMutex);
	}
}

void GITechDemo::Update(const float fDeltaTime)
{
	m_fDeltaTime = fDeltaTime;
	
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	PUSH_PROFILE_MARKER(__FUNCSIG__);

	int cLeft, cTop, cRight, cBottom;
	Framework* const pFW = Framework::GetInstance();
	pFW->GetClientArea(cLeft, cTop, cRight, cBottom);
	const Vec2i viewportSize = Vec2i(cRight - cLeft, cBottom - cTop);

	// Handle user input
	unsigned int cmd = APP_CMD_NONE;
	if (m_pInputManager && m_pInputMap)
	{
		// Handle user input for camera movement
		if (m_pInputMap->GetBool(APP_CMD_FORWARD))
			cmd |= APP_CMD_FORWARD;
		if (m_pInputMap->GetBool(APP_CMD_BACKWARD))
			cmd |= APP_CMD_BACKWARD;
		if (m_pInputMap->GetBool(APP_CMD_LEFT))
			cmd |= APP_CMD_LEFT;
		if (m_pInputMap->GetBool(APP_CMD_RIGHT))
			cmd |= APP_CMD_RIGHT;
		if (m_pInputMap->GetBool(APP_CMD_SPEED_UP))
			cmd |= APP_CMD_SPEED_UP;
		if (m_pInputMap->GetBool(APP_CMD_SLOW_DOWN))
			cmd |= APP_CMD_SLOW_DOWN;

		// Handle user input for camera rotation
		if (m_pInputMap->GetBool(APP_CMD_CTRL_YAW))
			cmd |= APP_CMD_CTRL_YAW;
		if (m_pInputMap->GetBool(APP_CMD_CTRL_PITCH))
			cmd |= APP_CMD_CTRL_PITCH;
		if (m_pInputMap->GetBool(APP_CMD_CTRL_ROLL))
			cmd |= APP_CMD_CTRL_ROLL;

		// If any mouse button was just pressed, the mouse is unlikely to be in
		// the center of the screen and this might result in a "jump" in the
		// rotation of the camera. Ignore this frame's mouse movement.
		// This will introduce a one frame lag.
		Vec3f rotateDelta;
		if (m_pInputMap->GetBoolIsNew(APP_CMD_CTRL_YAW) ||
			m_pInputMap->GetBoolIsNew(APP_CMD_CTRL_PITCH) ||
			m_pInputMap->GetBoolIsNew(APP_CMD_CTRL_ROLL))
		{
			rotateDelta *= 0.f;
		}
		else
		{
			// When we have an even number of pixels on one axis, the center is between
			// two adjacent pixels. Because of this, the normalized value we obtain
			// below can never be exactly 0.5f, so we apply a half-pixel offset.
			const Vec2f halfPixel(
				0.5f / (float)(viewportSize[0] - 1) * (float)(viewportSize[0] % 2 == 0),
				0.5f / (float)(viewportSize[1] - 1) * (float)(viewportSize[1] % 2 == 0)
				);
			rotateDelta.set(
				m_pInputMap->GetFloat(APP_CMD_PITCH_AXIS),
				m_pInputMap->GetFloat(APP_CMD_YAW_AXIS),
				m_pInputMap->GetFloat(APP_CMD_ROLL_AXIS)
				);
			rotateDelta -= Vec3f(0.5f - halfPixel[1], 0.5f - halfPixel[0], 0.5f - halfPixel[0]);
			rotateDelta *= CAMERA_ROTATE_SPEED;
		}

		// Update camera rotation matrix
		if (cmd & APP_CMD_CTRL_PITCH)
			m_tCamera.mRot = makeRot(EulerAngleXYZf(Math::deg2Rad(-rotateDelta[0]), 0.f, 0.f), Type2Type<Matrix44f>()) * m_tCamera.mRot;

		if (cmd & APP_CMD_CTRL_YAW)
			m_tCamera.mRot = makeRot(EulerAngleXYZf(0.f, Math::deg2Rad(-rotateDelta[1]), 0.f), Type2Type<Matrix44f>()) * m_tCamera.mRot;

		if (cmd & APP_CMD_CTRL_ROLL)
			m_tCamera.mRot = makeRot(EulerAngleXYZf(0.f, 0.f, Math::deg2Rad(rotateDelta[2])), Type2Type<Matrix44f>()) * m_tCamera.mRot;

		// Update camera position vector
		if (cmd & APP_CMD_FORWARD)
			m_tCamera.vMoveVec[2] = 1.f;
		else if (cmd & APP_CMD_BACKWARD)
			m_tCamera.vMoveVec[2] = -1.f;
		else
			m_tCamera.vMoveVec[2] = 0.f;

		if (cmd & APP_CMD_LEFT)
			m_tCamera.vMoveVec[0] = -1.f;
		else if (cmd & APP_CMD_RIGHT)
			m_tCamera.vMoveVec[0] = 1.f;
		else
			m_tCamera.vMoveVec[0] = 0.f;

		if (cmd & APP_CMD_SPEED_UP)
			m_tCamera.fSpeedFactor = CAMERA_SPEED_UP_FACTOR;
		else if (cmd & APP_CMD_SLOW_DOWN)
			m_tCamera.fSpeedFactor = CAMERA_SLOW_DOWN_FACTOR;
		else
			m_tCamera.fSpeedFactor = 1.f;

		gmtl::normalize(m_tCamera.vMoveVec);
		m_tCamera.vMoveVec *= CAMERA_MOVE_SPEED * m_tCamera.fSpeedFactor;
		m_tCamera.vPos -=
			Vec3f(m_tCamera.mRot[2][0] * m_tCamera.vMoveVec[2], m_tCamera.mRot[2][1] * m_tCamera.vMoveVec[2], m_tCamera.mRot[2][2] * m_tCamera.vMoveVec[2]) +
			Vec3f(m_tCamera.mRot[0][0] * m_tCamera.vMoveVec[0], m_tCamera.mRot[0][1] * m_tCamera.vMoveVec[0], m_tCamera.mRot[0][2] * m_tCamera.vMoveVec[0]);

		// Update mouse cursor
		if (cmd & APP_CMD_CTRL_PITCH ||
			cmd & APP_CMD_CTRL_YAW ||
			cmd & APP_CMD_CTRL_ROLL)
		{
			int wLeft, wTop, wRight, wBottom;
			pFW->GetWindowArea(wLeft, wTop, wRight, wBottom);
			const int sideBorderSize = (wRight - wLeft) - viewportSize[0];
			const int topBorderSize = (wBottom - wTop) - viewportSize[1] - sideBorderSize;
			pFW->SetCursorAtPos((wRight + wLeft - 1) / 2, (wBottom + wTop + topBorderSize - 1) / 2);
			if (!pFW->IsCursorHidden())
				pFW->ShowCursor(false);
		}
		else
		{
			if (pFW->IsCursorHidden())
				pFW->ShowCursor(true);
		}
	}

	// Animate camera
	if (CAMERA_ANIMATION_ENABLED)
	{
		static float lastInput = 0.f;
		if (cmd == APP_CMD_NONE)
		{
			lastInput += fDeltaTime;
			if (lastInput > CAMERA_ANIMATION_TIMEOUT_SECONDS)
			{
				static float time = 0.f;
				const float periodPos = 5000000.f;
				time += fDeltaTime;
				Vec3f perlinPos(
					PerlinNoise.Get(time / periodPos, 0.f),
					PerlinNoise.Get(0.f, time / periodPos),
					PerlinNoise.Get(time / periodPos, time / periodPos));
				perlinPos[0] = perlinPos[0] + 1.f / 2.f;
				perlinPos[1] = perlinPos[1] + 1.f / 2.f;
				perlinPos[2] = perlinPos[2] + 1.f / 2.f;
				perlinPos[0] = perlinPos[0] - floor(perlinPos[0]);
				perlinPos[1] = perlinPos[1] - floor(perlinPos[1]);
				perlinPos[2] = perlinPos[2] - floor(perlinPos[2]);

				m_tCamera.vPos = -Vec3f(
					((SceneAABB.getMax()[0] - SceneAABB.getMin()[0]) * perlinPos[0] + SceneAABB.getMin()[0]) / 1.5f,
					((SceneAABB.getMax()[1] - SceneAABB.getMin()[1]) * perlinPos[1] + SceneAABB.getMin()[1]) / 1.5f,
					((SceneAABB.getMax()[2] - SceneAABB.getMin()[2]) * perlinPos[2] + SceneAABB.getMin()[2]) / 1.5f
					);

				Vec3f sponzaCenter(
					(SceneAABB.getMax()[0] - SceneAABB.getMin()[0]) / 2.f + SceneAABB.getMin()[0],
					((SceneAABB.getMax()[1] - SceneAABB.getMin()[1]) / 2.f + SceneAABB.getMin()[1]) / 2.f,
					(SceneAABB.getMax()[2] - SceneAABB.getMin()[2]) / 2.f + SceneAABB.getMin()[2]);
				Vec3f zAxis = makeNormal(Vec3f(sponzaCenter + m_tCamera.vPos));
				//static Vec3f upVec = abs(zAxis[1]) == 1.f ? Vec3f(0.f, 0.f, 1.f) : Vec3f(0.f, 1.f, 0.f);
				Vec3f upVec = Vec3f(0.f, 1.f, 0.f);
				Vec3f xAxis = makeNormal(makeCross(upVec, zAxis));
				Vec3f yAxis = makeCross(zAxis, xAxis);
				upVec = yAxis;
				m_tCamera.mRot.set(
					xAxis[0], xAxis[1], xAxis[2], 0.f,
					yAxis[0], yAxis[1], yAxis[2], 0.f,
					zAxis[0], zAxis[1], zAxis[2], 0.f,
					0.f, 0.f, 0.f, 1.f
					);
			}
		}
		else
			lastInput = 0.f;
	}

	// Animate directional light using Perlin Noise
	if (DIRECTIONAL_LIGHT_ANIMATION_ENABLED)
	{
		static float time = 0.f;
		time += fDeltaTime;
		float noiseX = PerlinNoise.Get(time * 1000.f / (float)INT_MAX, 0);
		float noiseZ = PerlinNoise.Get(0, time * 1000.f / (float)INT_MAX);
		((Vec3f&)f3LightDir)[0] = noiseX;
		((Vec3f&)f3LightDir)[2] = noiseZ;
		((Vec3f&)f3LightDir)[1] = -1.f;
	}
	gmtl::normalize((Vec3f&)f3LightDir);

	// Precalculate some parts of the equation for reconstructing
	// linear depth from hyperbolic depth
	f2LinearDepthEquation = Vec2f(fZNear * fZFar / (fZNear - fZFar), fZFar / (fZFar - fZNear));

	// Calculate world matrix
	f44WorldMat = makeTrans(Vec3f(0, 0, 0), Type2Type<Matrix44f>());

	// Calculate view matrix
	f44ViewMat = m_tCamera.mRot * makeTrans(m_tCamera.vPos, Type2Type<Matrix44f>());

	// Calculate projection matrix
	RenderContext->CreatePerspectiveMatrix(f44ProjMat, Math::deg2Rad(60.f), (float)viewportSize[0] / (float)viewportSize[1], fZNear, fZFar);
	gmtl::invertFull((Matrix44f&)f44InvProjMat, (Matrix44f&)f44ProjMat);

	// Calculate some composite matrices
	gmtl::invertFull((Matrix44f&)f44InvViewMat, (Matrix44f&)f44ViewMat);
	f44ViewProjMat = f44ProjMat * f44ViewMat;
	f44InvViewProjMat = f44InvViewMat * f44InvProjMat;

	// Set the size of the backbuffer accordingly 
	RenderContext->SetBackBufferSize(viewportSize);

	// Update the artist parameter manager
	ArtistParameterManager* const pAPM = ArtistParameterManager::GetArtistParameterManager();
	if (pAPM)
		pAPM->Update();

	POP_PROFILE_MARKER();
}

void GITechDemo::Draw()
{
	Renderer* RenderContext = Renderer::GetInstance();
	if (!RenderContext)
		return;

	// For maximum parallelism, swap buffers at the beginning of the next CPU frame,
	// so as the GPU has as much time as possible to process the last frame.
	static bool bLastFrameValid = false;
	if (bLastFrameValid)
		RenderContext->SwapBuffers();

	if (RenderContext->BeginFrame())
	{
		RenderScheme::Draw();
		RenderContext->EndFrame();
		bLastFrameValid = true;
	}
	else
		bLastFrameValid = false;
}
