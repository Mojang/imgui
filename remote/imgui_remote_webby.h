//-----------------------------------------------------------------------------
// Remote ImGui https://github.com/JordiRos/remoteimgui
// Uses
// ImGui https://github.com/ocornut/imgui 1.3
// Webby https://github.com/deplinenoise/webby
// LZ4   https://code.google.com/p/lz4/
//-----------------------------------------------------------------------------
#pragma once

#include "webby/webby.h"

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <cstring> //memset

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

class IWebSocketServer;



static void onLog         (const char* text);
static int  onDispatch    (struct WebbyConnection *connection);
static int  onConnect     (struct WebbyConnection *connection);
static void onConnected   (struct WebbyConnection *connection);
static void onDisconnected(struct WebbyConnection *connection);
static int  onFrame       (struct WebbyConnection *connection, const struct WebbyWsFrame *frame);

class IWebSocketServer
{
public:
	static IWebSocketServer *s_WebSocketServer;
	virtual ~IWebSocketServer() {};

	enum OpCode // based on websocket connection opcodes
	{
		Continuation = 0,
		Text = 1,
		Binary = 2,
		Disconnect = 8,
		Ping = 9,
		Pong = 10,
	};

	void *Memory;
	int MemorySize;
	struct WebbyServer *Server;
	struct WebbyServerConfig ServerConfig;
	struct WebbyConnection *Client;

	int Init(const char *local_address, int local_port)
	{
		s_WebSocketServer = this;

#if defined(_WIN32)
		{
			WORD wsa_version = MAKEWORD(2,2);
			WSADATA wsa_data;
			if (0 != WSAStartup(wsa_version, &wsa_data))
				return -1;
		}
#endif

		memset(&ServerConfig, 0, sizeof ServerConfig);
		ServerConfig.bind_address = local_address;
		ServerConfig.listening_port = static_cast<unsigned short>(local_port);
		ServerConfig.flags = WEBBY_SERVER_WEBSOCKETS;
		ServerConfig.connection_max = 255;
		ServerConfig.request_buffer_size = 2048;
		ServerConfig.io_buffer_size = 8192;
		ServerConfig.dispatch = &onDispatch;
		ServerConfig.log = &onLog;
		ServerConfig.ws_connect = &onConnect;
		ServerConfig.ws_connected = &onConnected;
		ServerConfig.ws_closed = &onDisconnected;
		ServerConfig.ws_frame = &onFrame;

		MemorySize = WebbyServerMemoryNeeded(&ServerConfig);
		Memory = malloc(MemorySize);
		Server = WebbyServerInit(&ServerConfig, Memory, MemorySize);
		Client = NULL;
		if (!Server)
			return -2;

		return 0;
	}

	void Update()
	{
		if (Server)
			WebbyServerUpdate(Server);
	}

	void Shutdown()
	{
		if (Server)
		{
			WebbyServerShutdown(Server);
			free(Memory);
		}

#if defined(_WIN32)
		WSACleanup();
#endif
	}

	void WsOnConnected(struct WebbyConnection *connection)
	{
		Client = connection;
	}

	void WsOnDisconnected(struct WebbyConnection*)
	{
		Client = NULL;
		OnMessage(Disconnect, NULL, 0);
	}

	int WsOnFrame(struct WebbyConnection *connection, const struct WebbyWsFrame *frame)
	{
//		printf("WebSocket frame incoming\n");
//		printf("  Frame OpCode: %d\n", frame->opcode);
//		printf("  Final frame?: %s\n", (frame->flags & WEBBY_WSF_FIN) ? "yes" : "no");
//		printf("  Masked?     : %s\n", (frame->flags & WEBBY_WSF_MASKED) ? "yes" : "no");
//		printf("  Data Length : %d\n", (int) frame->payload_length);

		std::vector<unsigned char> buffer(frame->payload_length+1);
		WebbyRead(connection, &buffer[0], frame->payload_length);
		buffer[frame->payload_length] = 0;
//        if(!strstr((char*)&buffer[0],"ImMouseMove"))
//            printf("  Data : %s\n", &buffer[0]);

		OnMessage((OpCode)frame->opcode, &buffer[0], frame->payload_length);

		return 0;
	}

	virtual void OnMessage(OpCode, const void*, int) { }
	virtual void OnError() { }

	virtual void SendText(const void *data, int size)
	{
		if (Client)
		{
			WebbySendFrame(Client, WEBBY_WS_OP_TEXT_FRAME, data, size);
		}
	}

	virtual void SendBinary(const void *data, int size)
	{
		if (Client)
		{
			WebbySendFrame(Client, WEBBY_WS_OP_BINARY_FRAME, data, size);
		}
	}
};


static void onLog(const char*)
{
	//printf("[WsOnLog] %s\n", text);
}

#include "imgui_remote_html.h"

static int onDispatch(struct WebbyConnection *connection)
{
	std::string htmlStr;
	for(size_t i = 0; i < sizeof(imgui_html) / sizeof(imgui_html[0]); ++i) {
		htmlStr += imgui_html[i];
	}

	WebbyBeginResponse(connection, 200, static_cast<int>(htmlStr.size()), NULL, 0);
	WebbyWrite(connection, htmlStr.data(), htmlStr.size());
	WebbyEndResponse(connection);

	return 0;
}

static int onConnect(struct WebbyConnection *)
{
	//printf("[WsOnConnect] %s\n", connection->request.uri);

	//std::ifstream htmlFile("imgui.html");
	//std::string str((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());
	//s_WebSocketServer->SendText(str.data(), str.size());

	return 0;
}

static void onConnected(struct WebbyConnection *connection)
{
	//printf("[WsOnConnected]\n");
	IWebSocketServer::s_WebSocketServer->WsOnConnected(connection);
}

static void onDisconnected(struct WebbyConnection *connection)
{
	//printf("[WsOnDisconnected]\n");
	IWebSocketServer::s_WebSocketServer->WsOnDisconnected(connection);
}

static int onFrame(struct WebbyConnection *connection, const struct WebbyWsFrame *frame)
{
	//printf("[WsOnFrame]\n");
	return IWebSocketServer::s_WebSocketServer->WsOnFrame(connection, frame);
}
