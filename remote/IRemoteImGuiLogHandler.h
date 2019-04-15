/********************************************************
* (c) Mojang. All rights reserved                       *
* (c) Microsoft. All rights reserved.                   *
*********************************************************/
#pragma once

#ifdef IMGUI_ENABLED

#include <string>

namespace imgui {
	// Interface for logging in the ImGUI submodule
	class IRemoteImGuiLogHandler {
	public:
		IRemoteImGuiLogHandler() = default;
		virtual ~IRemoteImGuiLogHandler() = default;

		// Data that might be useful
		virtual void onInformational(const std::string & message) = 0;

		// Data describing a failure
		virtual void onError(const std::string & message) = 0;
	};
}

#endif // IMGUI_ENABLED