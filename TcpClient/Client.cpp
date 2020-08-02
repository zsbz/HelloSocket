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
			Login login;
			strcpy(login.userName, "刘德华");
			strcpy(login.password, "123456");

			send(_sock, (const char *)&login, sizeof(Login), 0);

			// 接收服务器返回数据
			LoginResult loginResult = {};
			recv(_sock, (char *)&loginResult, sizeof(loginResult), 0);
			printf("登录结果：%d \n", loginResult.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			// 5.发送请求命令给服务器
			Loginout loginout;
			strcpy(loginout.userName, "刘德华");
			send(_sock, (const char *)&loginout, sizeof(Loginout), 0);

			// 接收服务器返回数据
			LoginoutResult loginoutResult = {};
			recv(_sock, (char *)&loginoutResult, sizeof(loginoutResult), 0);
			printf("登出结果：%d \n", loginoutResult.result);
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
