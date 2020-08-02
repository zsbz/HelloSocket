#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

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
		// 5.接收客户端数据
		int nLen = recv(_clientSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("客户端退出，任务结束");
			break;
		}
		printf("收到命令：%s\n", _recvBuf);
		// 6.处理请求，根据不同请求发送不同数据
		if (0 == strcmp(_recvBuf, "getName"))
		{
			char msgBuf[] = "刘德华";
			send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else if (0 == strcmp(_recvBuf, "getAge"))
		{
			char msgBuf[] = "18";
			send(_clientSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else
		{
			char defaultMsgBuf[] = "Hellow, I'm Server";
			send(_clientSock, defaultMsgBuf, strlen(defaultMsgBuf) + 1, 0);
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
