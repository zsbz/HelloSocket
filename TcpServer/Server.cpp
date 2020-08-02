#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_OUT,
	CMD_ERROR
};

// 包头
struct DataHeader
{
	short dataLength; // 包长
	short cmd;		  // 请求类型
};

// 登录 DataPackage
struct Login
{
	char userName[32];
	char password[32];
};

struct LoginResult
{
	int result;
};

struct Loginout
{
	char userName[32];
};

struct LoginoutResult
{
	char userName[32];
};

int main()
{
	// 启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1.建立socket
	SOCKET _serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2.绑定接收客户端连接的端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr(127.0.0.1);
	if (SOCKET_ERROR == bind(_serverSock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("错误，绑定网络端口失败\n");
	}
	else
	{
		printf("绑定网络端口成功\n");
	}
	// 3 listen 监听网络端口
	if (SOCKET_ERROR == listen(_serverSock, 5))// 最多5个连接
	{
		printf("错误，监听端口失败\n");
	}
	else
	{
		printf("监听网络端口成功\n");
	}

	// 4 accept 等待接收客户端连接
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _clientSock = INVALID_SOCKET;

	_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _clientSock)
	{
		printf("错误，接收到无效的客户端socket\n");
	}
	printf("新客户端加入：socket = %d, IP = %s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true)
	{
		DataHeader header = {};
		// 5.接收客户端数据
		int nLen = recv(_clientSock, (char *)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("客户端退出，任务结束");
			break;
		}
		printf("收到命令：%d, 数据长度：%d\n", header.cmd, header.dataLength);

		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			// 接收登录信息
			Login login = {};
			recv(_clientSock, (char *)&login, sizeof(Login), 0);
			// 判断用户名密码正确

			// 返回登录结果  先发头
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			LoginResult result = { 1 };
			send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGIN_OUT:
		{
			// 接收登出信息
			Loginout loginout = {};
			recv(_clientSock, (char *)&loginout, sizeof(Loginout), 0);

			// 返回登出结果  先发头
			LoginoutResult result = { "成功" };
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			send(_clientSock, (const char *)&result, sizeof(LoginoutResult), 0);
		}
		break;
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			break;
		}
	}

	// 7.关闭套接字
	closesocket(_serverSock);

	// 清除Windows socket 2.x环境
	WSACleanup();
	printf("已退出");
	getchar();
	return 0;

}
