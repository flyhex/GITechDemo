
#ifndef GAINPUTINPUTDEVICEMOUSEWIN_H_
#define GAINPUTINPUTDEVICEMOUSEWIN_H_

#include "GainputInputDeviceMouseImpl.h"
#include "../GainputHelpers.h"

#include "../GainputWindows.h"

namespace gainput
{

class InputDeviceMouseImplWin : public InputDeviceMouseImpl
{
public:
	InputDeviceMouseImplWin(InputManager& manager, InputDevice& device, InputState& state, InputState& previousState) :
		manager_(manager),
		device_(device),
		state_(&state),
		previousState_(&previousState),
		nextState_(manager.GetAllocator(), MouseButtonCount + MouseAxisCount),
		delta_(0)
	{
	}

	InputDevice::DeviceVariant GetVariant() const
	{
		return InputDevice::DV_STANDARD;
	}

	void Update(InputDeltaState* delta)
	{
		delta_ = delta;

		// Reset mouse wheel buttons
		if (previousState_->GetBool(MouseButton3))
		{
			HandleButton(device_, nextState_, delta_, MouseButton3, false);
		}

		if (previousState_->GetBool(MouseButton4))
		{
			HandleButton(device_, nextState_, delta_, MouseButton4, false);
		}

		*state_ = nextState_;
	}

	void HandleMessage(const MSG& msg)
	{
		GAINPUT_ASSERT(state_);
		GAINPUT_ASSERT(previousState_);

		DeviceButtonId buttonId;
		bool pressed;
		bool moveMessage = false;
		int ax = -1;
		int ay = -1;
		switch (msg.message)
		{
		case WM_KILLFOCUS:
		case WM_ACTIVATE:
			if (msg.message == WM_KILLFOCUS || (msg.message == WM_ACTIVATE && LOWORD(msg.wParam) == WA_INACTIVE))
			{
                // Reset input state when window is out of focus
				for (unsigned int i = 0; i < MouseButtonCount_; i++)
				{
					HandleButton(device_, nextState_, delta_, i, false);
				}
			}
			return;
		case WM_LBUTTONDOWN:
			buttonId = MouseButtonLeft;
			pressed = true;
			break;
		case WM_LBUTTONUP:
			buttonId = MouseButtonLeft;
			pressed = false;
			break;
		case WM_RBUTTONDOWN:
			buttonId = MouseButtonRight;
			pressed = true;
			break;
		case WM_RBUTTONUP:
			buttonId = MouseButtonRight;
			pressed = false;
			break;
		case WM_MBUTTONDOWN:
			buttonId = MouseButtonMiddle;
			pressed = true;
			break;
		case WM_MBUTTONUP:
			buttonId = MouseButtonMiddle;
			pressed = false;
			break;
		case WM_XBUTTONDOWN:
			buttonId = MouseButton4 + GET_XBUTTON_WPARAM(msg.wParam);
			pressed = true;
			break;
		case WM_XBUTTONUP:
			buttonId = MouseButton4 + GET_XBUTTON_WPARAM(msg.wParam);
			pressed = false;
			break;
		case WM_MOUSEMOVE:
			moveMessage = true;
			ax = GET_X_LPARAM(msg.lParam);
			ay = GET_Y_LPARAM(msg.lParam);
			break;
		case WM_MOUSEWHEEL:
			{
				int wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam);
				if (wheel < 0)
				{
					buttonId = MouseButton4;
					pressed = true;
				}
				else if (wheel > 0)
				{
					buttonId = MouseButton3;
					pressed = true;
				}
				break;
			}
		default: // Non-mouse message
			return;
		}

		if (moveMessage)
		{
			float x = float(ax)/float(manager_.GetDisplayWidth());
			float y = float(ay)/float(manager_.GetDisplayHeight());
			HandleAxis(device_, nextState_, delta_, MouseAxisX, x);
			HandleAxis(device_, nextState_, delta_, MouseAxisY, y);
		}
		else
		{
			HandleButton(device_, nextState_, delta_, buttonId, pressed);
		}
	}

private:
	InputManager& manager_;
	InputDevice& device_;
	InputState* state_;
	InputState* previousState_;
	InputState nextState_;
	InputDeltaState* delta_;
};

}

#endif

