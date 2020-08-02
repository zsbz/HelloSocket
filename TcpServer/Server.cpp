#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_ERROR
};

// 包头
struct DataHeader
{
	short dataLength; // 包长
	short cmd;		  // 请求类型
};

// 登录 DataPackage
struct Login: public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};

struct LoginResult: public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Loginout: public DataHeader
{
	Loginout()
	{
		dataLength = sizeof(Loginout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LoginoutResult : public DataHeader
{
	LoginoutResult()
	{
		dataLength = sizeof(LoginoutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result = 1;
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
		// 5.接收客户端数据		先接收头，通过头来判断接收的什么命令
		int nLen = recv(_clientSock, (char *)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("客户端退出，任务结束");
			break;
		}

		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			// 接收登录信息		上面已经接收到了头，需要进行偏移，减去头的长度
			Login login = {};
			recv(_clientSock, (char *)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0); 
			printf("收到命令：登录命令, 数据长度：%d, 用户名：%s 密码：%s\n", login.dataLength, login.userName, login.password);
			// 判断用户名密码正确

			// 返回登录结果
			LoginResult result;
			send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			// 接收登出信息
			Loginout loginout = {};
			recv(_clientSock, (char *)&loginout + sizeof(DataHeader), sizeof(Loginout) - sizeof(DataHeader), 0);
			printf("收到命令：登出命令, 数据长度：%d, 用户名：%s\n", loginout.dataLength, loginout.userName);

			// 返回登出结果
			LoginoutResult result;
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
