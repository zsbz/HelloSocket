#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#include <thread>

#pragma comment(lib, "ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
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
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scokId = 0;
	}
	int scokId;
};

int processor(SOCKET _sock)
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

	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult *loginRes = (LoginResult*)szRecv;
		printf("收到服务器消息：登录结果, 数据长度：%d\n", loginRes->dataLength);
	}
	break;
	case CMD_LOGINOUT_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult *loginoutRes = (LoginoutResult*)szRecv;
		printf("收到服务器消息：登出结果, 数据长度：%d\n", loginoutRes->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin *userJoin = (NewUserJoin*)szRecv;
		printf("收到服务器消息：新用户加入, 数据长度：%d\n", userJoin->dataLength);
	}
	break;
	default:
	{
		DataHeader header = { 0, CMD_ERROR };
		send(_sock, (const char *)&header, sizeof(DataHeader), 0);
	}
	break;
	}
}

bool g_bRun = false;

void cmdThread(SOCKET _sock)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "刘德华");
			strcpy(login.password, "123456");
			send(_sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			Loginout loginout;
			strcpy(loginout.userName, "刘德华");
			send(_sock, (const char*)&loginout, sizeof(Loginout), 0);
		}
		else
		{
			printf("输入命令错误\n");
		}
	}
}

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

	// 启动线程
	std::thread t1(cmdThread, _sock);
	t1.detach(); // 与主线程分离
	g_bRun = true;

	// 循环输入请求命令
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		timeval timeva = { 1, 0 };
		int res = select(_sock, &fdReads, 0, 0, &timeva);
		if (res < 0)
		{
			printf("select任务结束1\n");
			break;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads); 
			if (-1 == processor(_sock))
			{
				printf("select任务结束2\n");
				break;
			}
		}

		//printf("空闲时间处理其他业务\n");
	}

	// 7.关闭套接字
	closesocket(_sock);

	// 清除Windows socket 2.x环境
	WSACleanup();
	printf("已退出");
	getchar();
	return 0;
}
