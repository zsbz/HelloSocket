#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

#include <vector>
#include <stdio.h>

#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _serverSock;
	std::vector<SOCKET> g_clients;

public:
	EasyTcpServer()
	{
		_serverSock = INVALID_SOCKET;
	}

	virtual ~EasyTcpServer()
	{
		closeSocket();
	}

	// 初始化Socket
	SOCKET initSocket()
	{
#ifdef _WIN32
		// 启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		// 之前已经初始化，关闭原来的
		if (INVALID_SOCKET != _serverSock)
		{
			printf("<socket=%d>关闭旧的socket\n", _serverSock);
			closeSocket();
		}

		_serverSock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _serverSock)
		{
			printf("建立socket失败\n");
		}
		else
		{
			printf("建立socket成功\n");
		}

		return _serverSock;
	}

	// 绑定端口号
	int bindAddr(const char *ip, unsigned short port)
	{
		if (INVALID_SOCKET == _serverSock)
		{
			initSocket();
		}

		// 2.绑定接收客户端连接的端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); // host to net unsigned short

#ifdef _WIN32
		// 如果ip不为空，就使用传入的ip
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr(127.0.0.1);
		}
#else
		// 如果ip不为空，就使用传入的ip
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY; // inet_addr(127.0.0.1);
		}
#endif
		int res = bind(_serverSock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == res)
		{
			printf("错误，绑定网络端口<%d>失败\n", port);
		}
		else
		{
			printf("绑定网络端口<%d>成功\n", port);
		}

		return res;
	}
	
	// 监听端口号
	int listenSocket(int n)
	{
		int res = listen(_serverSock, n); // 最多n个连接
		if (SOCKET_ERROR == res)
		{
			printf("Socket=<%d>错误，监听端口失败\n", _serverSock);
		}
		else
		{
			printf("Socket=<%d>监听网络端口成功\n", _serverSock);
		}
		return res;
	}

	// 接受客户端连接
	SOCKET acceptSocket()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _clientSock = INVALID_SOCKET;

		_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _clientSock)
		{
			printf("Socket=<%d>错误，接收到无效的客户端socket\n", _serverSock);
		}
		else
		{
			NewUserJoin userJoin;
			sendDataToAll(&userJoin);

			// 储存新加入的客户端
			g_clients.push_back(_clientSock);
			printf("Socket=<%d>新客户端加入：socket = %d, IP = %s \n", _serverSock, (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _clientSock;
	}

	// 关闭Socket
	void closeSocket()
	{
		// 关闭Win Sock2.x环境
		if (_serverSock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (size_t n = g_clients.size() - 1; n >= 0; --n)
			{
				closesocket(g_clients[n]);
			}
			closesocket(_serverSock);
			// 清除Windows socket 2.x环境
			WSACleanup();
#else
			for (size_t n = g_clients.size() - 1; n >= 0; --n)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}
	}

	// 处理网络消息
	bool onRun()
	{
		if (isRun())
		{
			fd_set fdRead; // 创建可读的描述符(socket)集合
			fd_set fdWrite; // 可写的描述符(socket)集合
			fd_set fdExcp; // 有异常的描述符(socket)集合

			// FD_ZERO 清空集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcp);

			FD_SET(_serverSock, &fdRead); // 把描述符(socket)放入fdRead集合
			FD_SET(_serverSock, &fdWrite);
			FD_SET(_serverSock, &fdExcp);

			for (int n = (int)g_clients.size() - 1; n >= 0; --n)
			{
				FD_SET(g_clients[n], &fdRead);
			}

			timeval timeva = { 1, 0 }; // {秒，微秒}

			// select函数调用后，把可读的socket放入fdRead，把可写的socket放入fdWrite
			// nfds是一个整数值，是指fd_set集合中所有描述符（socket）的范围，既是所有文件描述符最大值+1，Windows下没有意义，可以写0
			// 最后一个参数timeout：select()最多等待时间，对阻塞操作则为NULL。若timeout参数为空指针，则select将一直阻塞到有一个描述字满足条件（有socket变化了，可读或者可写或者有异常）
			int res = select(_serverSock + 1, &fdRead, &fdWrite, &fdExcp, &timeva); // timeva为{0,0}表示立即返回
			if (res < 0)
			{
				printf("select任务结束\n");
				closeSocket();
				return false;
			}

			// 判断描述符(socket)是在可读的socket集合中（是否有数据需要接收）
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
				else
				{
					// 发送给全部客户端
					for (int n = (int)g_clients.size() - 1; n >= 0; --n)
					{
						NewUserJoin userJoin;
						send(g_clients[n], (const char *)&userJoin, sizeof(NewUserJoin), 0);
					}
					// 储存新加入的客户端
					g_clients.push_back(_clientSock);
					printf("新客户端加入：socket = %d, IP = %s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
				}
			}

			// 处理可读的客户端数据
			for (size_t n = 0; n < fdRead.fd_count; ++n)
			{
				// 如果返回-1，表示客户端退出，从储存客户端的集合中移除
				if (-1 == RecvData(fdRead.fd_array[n]))
				{
					// 指定开始结束位置
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}

			return true;
		}
		return false;
	}

	// 是否工作中
	bool isRun()
	{
		return _serverSock != INVALID_SOCKET;
	}

	// 接受数据 处理粘包 拆分包
	int RecvData(SOCKET _clientSock)
	{
		// 缓冲区
		char szRecv[1024] = {};

		// 5.接收客户端数据		先接收头，通过头来判断接收的什么命令
		int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;

		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>退出，任务结束\n", _clientSock);
			return -1;
		}

		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		onNetMsg(_clientSock, header);
		return 0;
	}

	// 响应网络消息
	virtual void onNetMsg(SOCKET _clientSock, DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login*)header;
			printf("收到客户端<Socket=%d>命令：登录命令, 数据长度：%d, 用户名：%s 密码：%s\n", _clientSock, login->dataLength, login->userName, login->password);
			// 判断用户名密码正确

			// 返回登录结果
			LoginResult result;
			send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			Loginout *loginout = (Loginout *)header;
			printf("收到客户端<Socket=%d>命令：登出命令, 数据长度：%d, 用户名：%s\n", _clientSock, loginout->dataLength, loginout->userName);

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
	// 发送给指定socket
	int sendData(SOCKET _clientSock, DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	// 发送给全部客户端socket
	void sendDataToAll(DataHeader *header)
	{
		if (isRun() && header)
		{
			// 发送给全部客户端
			for (int n = (int)g_clients.size() - 1; n >= 0; --n)
			{
				sendData(g_clients[n], header);
			}
		}
	}
};


#endif // !_EasyTcpServer_hpp_