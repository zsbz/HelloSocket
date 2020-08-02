#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#include <vector>

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
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Loginout : public DataHeader
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

// 储存连接的客户端socket
std::vector<SOCKET> g_clients;

int processor(SOCKET _clientSock)
{
	// 缓冲区
	char szRecv[1024] = {};

	// 5.接收客户端数据		先接收头，通过头来判断接收的什么命令
	int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader *)szRecv;
	if (nLen <= 0)
	{
		printf("客户端退出，任务结束\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		// 接收登录信息		上面已经接收到了头，需要进行偏移，减去头的长度
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login *login = (Login*)szRecv;
		printf("收到命令：登录命令, 数据长度：%d, 用户名：%s 密码：%s\n", login->dataLength, login->userName, login->password);
		// 判断用户名密码正确

		// 返回登录结果
		LoginResult result;
		send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{
		// 接收登出信息
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout *loginout = (Loginout *)szRecv;
		printf("收到命令：登出命令, 数据长度：%d, 用户名：%s\n", loginout->dataLength, loginout->userName);

		// 返回登出结果
		LoginoutResult result;
		send(_clientSock, (const char *)&result, sizeof(LoginoutResult), 0);
	}
	break;
	default:
	{
		DataHeader header = { 0, CMD_ERROR };
		send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
	}
	break;
	}
}

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

	while (true)
	{
		fd_set fdRead; // 创建可读的socket集合
		fd_set fdWrite; // 可写的socket集合
		fd_set fdExcp; // 有异常的socket集合

		// FD_ZERO 清空集合
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcp);

		FD_SET(_serverSock, &fdRead); // 把服务器socket放入fdRead集合
		FD_SET(_serverSock, &fdWrite);
		FD_SET(_serverSock, &fdExcp);

		for (int n = (int)g_clients.size() - 1; n >= 0 ; --n)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		timeval timeva = { 0, 0 }; // {秒，微秒}

		// select函数调用后，把可读的socket放入fdRead，把可写的socket放入fdWrite
		// nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，既是所有文件描述符最大值+1，Windows下没有意义，可以写0
		// 最后一个参数timeout：select()最多等待时间，对阻塞操作则为NULL。若timeout参数为空指针，则select将一直阻塞到有一个描述字满足条件（有socket变化了，可读或者可写或者有异常）
		int res = select(_serverSock + 1, &fdRead, &fdWrite, &fdExcp, &timeva); // timeva为{0,0}表示立即返回
		if (res < 0)
		{
			printf("select任务结束\n");
			break;
		}

		// 判断服务器socket是在可读的socket集合中（是否有数据需要接收）
		// 在调用select()函数后，用FD_ISSET来检测fd是否在set集合中，当检测到fd在set中则返回真，否则，返回假（0）
		if (FD_ISSET(_serverSock, &fdRead))
		{
			// 接收数据，然后从可读集合中移除
			FD_CLR(_serverSock, &fdRead);
			// 4 accept 等待接收客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _clientSock = INVALID_SOCKET;

			_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _clientSock)
			{
				printf("错误，接收到无效的客户端socket\n");
			}
			// 储存新加入的客户端
			g_clients.push_back(_clientSock);
			printf("新客户端加入：socket = %d, IP = %s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}

		// 处理可读的客户端数据
		for (size_t n = 0; n < fdRead.fd_count; ++n)
		{
			// 如果返回-1，表示客户端退出，从储存客户端的集合中移除
			if (-1 == processor(fdRead.fd_array[n]))
			{
				// 指定开始结束位置
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
	}

	// 7.关闭所有套接字
	for (size_t n = g_clients.size() - 1; n >= 0; --n)
	{
		closesocket(g_clients[n]);
	}

	// 清除Windows socket 2.x环境
	WSACleanup();
	printf("已退出");
	getchar();
	return 0;
}