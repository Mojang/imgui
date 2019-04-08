/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#include "RemoteImGui.h"

#include "RemoteImGuiFrameBuilder.h"

#ifdef IMGUI_ENABLED

#define IMGUI_REMOTE_KEY_FRAME    60 // send a keyframe every so many frames
#define IMGUI_REMOTE_INPUT_FRAMES 60 // input is valid this many frames after reception

namespace imgui {

	RemoteImGui::RemoteImGui()
		: mFrameBuilder(std::make_unique<RemoteImGuiFrameBuilder>()) {
	}

	RemoteImGui::~RemoteImGui() {
	}

	void RemoteImGui::update() {
		mFrame++;
	}

	bool RemoteImGui::getRemoteInput(RemoteInput &input) {
		bool res = false;
		if (_getIsActive()) {
			if (mFrame - mFrameReceived < IMGUI_REMOTE_INPUT_FRAMES) {
				input = mInput;
				res = true;
			}
		}

		memset(mInput.KeysDown, 0, 256 * sizeof(bool));
		mInput.MouseWheelDelta = 0;
		return res;
	}

	void RemoteImGui::draw(ImDrawList** const cmd_lists, int cmd_lists_count) {
		if (_getIsActive()) {
			_sendDrawFrame(cmd_lists, cmd_lists_count);
		}
	}

	void RemoteImGui::_sendFontFrame() {
		Frame frame;
		mFrameBuilder->buildFontFrame(frame);
		_sendFrame(frame);
	}

	void RemoteImGui::_sendDrawFrame(ImDrawList** const cmd_lists, int cmd_lists_count) {
		Frame frame;
		if (mFrameBuilder->buildDrawFrame(cmd_lists, cmd_lists_count, (mFrame % IMGUI_REMOTE_KEY_FRAME) == 0, frame)) {
			_sendFrame(frame);
		}
	}

	void RemoteImGui::_handleMessage(RemoteMessageType messageType, const void * data, int size) {
		// We should never receive anything other than a text message
		//((char *)data)[size] = 0;

		std::string s((char*)data, size);
		mDebug("RemoteImGui::_handleMessage() recieved [" + s + "]");

		switch (messageType) {
			case RemoteMessageType::ImMouseMove: {
				mDebug("RemoteImGui::_handleMessage() recieved ImMouseMove event [" + s + "]");
				int x, y, mouse_left, mouse_right;
				if (sscanf((char *)data, "%d,%d,%d,%d", &x, &y, &mouse_left, &mouse_right) == 4) {
					_onImMouseMove(x, y, mouse_left, mouse_right);
				}
				break;
			}
			case RemoteMessageType::ImMousePress: {
				mDebug("RemoteImGui::_handleMessage() recieved ImMousePress event [" + s + "]");
				int l, r;
				if (sscanf((char *)data, "%d,%d", &l, &r) == 2) {
					_onImMousePress(l, r);
				}
				break;
			}
			case RemoteMessageType::ImMouseWheelDelta: {
				mDebug("RemoteImGui::_handleMessage() recieved ImMouseWheelDelta event [" + s + "]");
				float mouseWheelDelta;
				if (sscanf((char *)data, "%f", &mouseWheelDelta) == 1) {
					_onImMouseWheelDelta(mouseWheelDelta);
				}
				break;
			}
			case RemoteMessageType::ImKeyDown: {
				mDebug("RemoteImGui::_handleMessage() recieved ImKeyDown event [" + s + "]");
				int key, shift, ctrl;
				if (sscanf((char *)data, "%d,%d,%d", &key, &shift, &ctrl) == 3) {
					_onImKeyDown(key, shift, ctrl);
				}
				break;
			}
			case RemoteMessageType::ImKeyUp: {
				mDebug("RemoteImGui::_handleMessage() recieved ImKeyUp event [" + s + "]");
				int key;
				if (sscanf((char *)data, "%d", &key) == 1) {
					_onImKeyUp(key);
				}
				break;
			}
			case RemoteMessageType::ImKeyPress: {
				mDebug("RemoteImGui::_handleMessage() recieved ImKeyPress event [" + s + "]");
				unsigned int key;
				if (sscanf((char *)data, "%d", &key) == 1) {
					_onImKeyPress(key);
				}
				break;
			}
			case RemoteMessageType::ImClipboard: {
				mDebug("RemoteImGui::_handleMessage() recieved ImClipboard event [" + s + "]");
				_onImClipboard(nullptr, reinterpret_cast<const char *>(data));
				break;
			}
			case RemoteMessageType::ImInit:
			case RemoteMessageType::LocalIgnored:
			case RemoteMessageType::LocalDisconnect:
			case RemoteMessageType::RelayRoomJoined:
			case RemoteMessageType::RelayRoomUpdate: {
				// Valid, but unhandled
				break;
			}
			default: {
				// Unsupported message type
				assert(0);
				return;
			}
		}
	}

	void RemoteImGui::_onImMouseMove(int x, int y, int left, int right) {
		mFrameReceived = mFrame;
		mInput.MousePos.x = (float)x;
		mInput.MousePos.y = (float)y;
		mInput.MouseButtons = left | (right << 1);
	}

	void RemoteImGui::_onImMousePress(int left, int right) {
		mFrameReceived = mFrame;
		mInput.MouseButtons = left | (right << 1);
	}

	void RemoteImGui::_onImMouseWheelDelta(float mouseWheel) {
		mFrameReceived = mFrame;
		mInput.MouseWheelDelta = mouseWheel * 0.01f;
	}

	void RemoteImGui::_onImKeyDown(int key, int shift, int ctrl) {
		mFrameReceived = mFrame;
		(shift);
		(ctrl);
		//mInput.KeyShift = shift > 0;
		//mInput.KeyCtrl = ctrl > 0;
		//mapRemoteKey(&key, mInput.KeyCtrl);
		mInput.KeysDown[key] = true;
	}

	void RemoteImGui::_onImKeyUp(int key) {
		mFrameReceived = mFrame;
		mInput.KeysDown[key] = false;
		//mInput.KeyShift = false;
		//mInput.KeyCtrl = false;
	}

	void RemoteImGui::_onImKeyPress(int key) {
		ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(key));
	}

	void RemoteImGui::_onImClipboard(void* userData, const char* text) {
		ImGui::GetIO().SetClipboardTextFn(userData, text);
	}
}

#endif // IMGUI_ENABLED