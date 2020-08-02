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
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("建立socket失败\n");
	}
	else
	{
		printf("建立socket成功\n");
	}
	// 2.连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		printf("连接socket失败\n");
	}
	else
	{
		printf("连接socket成功\n");
	}

	// 循环输入请求命令
	while (true)
	{
		// 3.输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4.处理请求命令
		if (0 == strcmp(cmdBuf, "exit"))
		{
			break;
		}
		else
		{
			// 5.发送请求命令给服务器
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		// 6.接收服务器信息
		char recvBuf[256] = {};
		int nlen = recv(_sock, recvBuf, 256, 0);
		if (nlen > 0)
		{
			printf("接收到的数据：%s \n", recvBuf);
		}
	}


	// 7.关闭套接字
	closesocket(_sock);

	// 清除Windows socket 2.x环境
	WSACleanup();
	printf("已退出");
	getchar();
	return 0;
}
