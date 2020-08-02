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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			// 5.发送请求命令给服务器
			Login login = { "刘德华", "123456" };
			DataHeader header = { sizeof(login), CMD_LOGIN};

			send(_sock, (const char *)&header, sizeof(header), 0);
			send(_sock, (const char *)&login, sizeof(Login), 0);

			// 接收服务器返回数据
			DataHeader resultHeader = {};
			LoginResult loginResult = {};
			recv(_sock, (char *)&resultHeader, sizeof(resultHeader), 0);
			recv(_sock, (char *)&loginResult, sizeof(loginResult), 0);
			printf("LoginResult：%d \n", loginResult.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			// 5.发送请求命令给服务器
			Loginout loginout = { "刘德华"};
			DataHeader header = { sizeof(loginout), CMD_LOGIN_OUT};
			send(_sock, (const char *)&header, sizeof(DataHeader), 0);
			send(_sock, (const char *)&loginout, sizeof(Loginout), 0);

			// 接收服务器返回数据
			DataHeader resultHeader = {};
			LoginoutResult loginoutResult = {};
			recv(_sock, (char *)&resultHeader, sizeof(resultHeader), 0);
			recv(_sock, (char *)&loginoutResult, sizeof(loginoutResult), 0);
			printf("LoginoutResult：%s \n", loginoutResult.userName);
		} 
		else
		{
			printf("不支持的命令。\n");
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
