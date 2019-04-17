/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#include "RemoteImGui.h"

#include "RemoteImGuiFrameBuilder.h"
#include "IRemoteImGuiLogHandler.h"
#include <sstream>

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
		if (_getIsActive()) {
			if (mFrame - mFrameReceived < IMGUI_REMOTE_INPUT_FRAMES) {
				input = mInput;
				return true;
			}
		}
		return false;
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

	void RemoteImGui::_handleMessage(RemoteMessageType messageType, const void * data, int) {
		switch (messageType) {
			case RemoteMessageType::ImInit: {
				// ImGUI initialization request received - send init data
				std::stringstream s;
				s << (unsigned char)RemoteMessageType::ImInit;
				_sendText(s.str());

				// Send the canvas information
				ImGuiIO& io = ImGui::GetIO();
				s.clear();
				s.str(std::string());
				s << (unsigned char)RemoteMessageType::ImCanvasUpdate;
				s << io.DisplaySize.x;
				s << ',';
				s << io.DisplaySize.y;
				_sendText(s.str());

				// Send the font textures
				_sendFontFrame();

				break;
			}
			case RemoteMessageType::ImMouseMove: {
				int x, y, mouse_left, mouse_right;
				if (sscanf((char *)data, "%d,%d,%d,%d", &x, &y, &mouse_left, &mouse_right) == 4) {
					_onImMouseMove(x, y, mouse_left, mouse_right);
				}
				break;
			}
			case RemoteMessageType::ImMousePress: {
				int l, r;
				if (sscanf((char *)data, "%d,%d", &l, &r) == 2) {
					_onImMousePress(l, r);
				}
				break;
			}
			case RemoteMessageType::ImMouseWheelDelta: {
				float mouseWheelDelta;
				if (sscanf((char *)data, "%f", &mouseWheelDelta) == 1) {
					_onImMouseWheelDelta(mouseWheelDelta);
				}
				break;
			}
			case RemoteMessageType::ImKeyDown: {
				int key, shift, ctrl;
				if (sscanf((char *)data, "%d,%d,%d", &key, &shift, &ctrl) == 3) {
					_onImKeyDown(key, shift, ctrl);
				}
				break;
			}
			case RemoteMessageType::ImKeyUp: {
				int key, shift, ctrl;
				if (sscanf((char *)data, "%d,%d,%d", &key, &shift, &ctrl) == 3) {
					_onImKeyUp(key, shift, ctrl);
				}
				break;
			}
			case RemoteMessageType::ImKeyPress: {
				int key;
				if (sscanf((char *)data, "%d", &key) == 1) {
					_onImKeyPress(key);
				}
				break;
			}
			case RemoteMessageType::ImClipboard: {
				_onImClipboard(nullptr, reinterpret_cast<const char *>(data));
				break;
			}
			case RemoteMessageType::RelayRoomJoined:
			case RemoteMessageType::RelayRoomUpdate:
			case RemoteMessageType::ImCanvasUpdate: {
				// Valid, but unhandled
				break;
			}
			case RemoteMessageType::BadMessageType:
			default: {
				// All message types should have been caught by now
				std::shared_ptr<IRemoteImGuiLogHandler> logger{ mLogger.lock() };
				if (logger) {
					std::ostringstream stream;
					stream << "Unsupported message type: " << (unsigned char)messageType;
					logger->onError(stream.str());
				}
				break;
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
		mInput.KeyShift = shift != 0;
		mInput.KeyCtrl = ctrl != 0;
		_mapRemoteKey(key, mInput.KeyCtrl);
		mInput.KeysDown[key] = true;
	}

	void RemoteImGui::_onImKeyUp(int key, int shift, int ctrl) {
		mFrameReceived = mFrame;
		mInput.KeyShift = shift != 0;
		mInput.KeyCtrl = ctrl != 0;
		_mapRemoteKey(key, mInput.KeyCtrl);
		mInput.KeysDown[key] = false;
	}

	void RemoteImGui::_onImKeyPress(int key) {
		ImGui::GetIO().AddInputCharacter(static_cast<ImWchar>(key));
	}

	void RemoteImGui::_onImClipboard(void* userData, const char* text) {
		ImGui::GetIO().SetClipboardTextFn(userData, text);
	}
}

#endif // IMGUI_ENABLED