#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SICKET)(~0)
	#define SOCKET_ERROR	(-1)
#endif

#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock = INVALID_SOCKET;
public:
	EasyTcpClient()
	{

	}

	virtual ~EasyTcpClient()
	{

	}

	// 初始化socket
	void initSocket()
	{
#ifdef _WIN32
		// 启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		// 1.建立socket
		// 之前已经初始化，关闭原来的
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧的socket\n", _sock);
			close();
		}

		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock)
		{
			printf("建立socket失败\n");
		}
		else
		{
			printf("建立socket成功\n");
		}
	}

	// 连接服务器
	int connecttion(char *ip, short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			initSocket();
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("连接socket失败\n");
		}
		else
		{
			printf("连接socket成功\n");
		}
		return ret;
	}

	// 关闭socket
	void close()
	{
		// 关闭Win Sock2.x环境
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			// 清除Windows socket 2.x环境
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	// 接收数据  处理粘包 拆分包
	int RecvData(SOCKET _sock)
	{
		// 缓冲区
		char szRecv[1024] = {};

		// 5.接收客户端数据		先接收头，通过头来判断接收的什么命令
		int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			printf("与服务器断开连接，任务结束\n");
			return -1;
		}

		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(header);
	}

	// 响应网络数据
	void onNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *loginRes = (LoginResult*)header;
			printf("收到服务器消息：登录结果, 数据长度：%d\n", loginRes->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginoutResult *loginoutRes = (LoginoutResult*)header;
			printf("收到服务器消息：登出结果, 数据长度：%d\n", loginoutRes->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{	
			NewUserJoin *userJoin = (NewUserJoin*)header;
			printf("收到服务器消息：新用户加入, 数据长度：%d\n", userJoin->dataLength);
		}
		break;
		default:
		{
			DataHeader header = { 0, CMD_ERROR };
		}
		break;
		}
	}

	// 处理数据
	bool onRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);

			timeval timeva = { 1, 0 };
			int res = select(_sock, &fdReads, 0, 0, &timeva);
			if (res < 0)
			{
				printf("select任务结束1\n");
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// 发送数据
	int sendData(DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
};


#endif // !_EasyTcpClient_hpp_
