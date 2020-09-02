#include "EasyTcpClient.hpp"

#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#include <thread>

#pragma comment(lib, "ws2_32.lib")

void cmdThread(EasyTcpClient *client)
{
	while (true)
	{
		char cmdBuf[256] = {};
		//scanf("%s", cmdBuf);

		// 一直发送请求，测试大量数据时是否粘包
		memcpy(cmdBuf, "login", sizeof("login"));

		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->closeSocket();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "刘德华");
			strcpy(login.password, "123456");
			client->sendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			Loginout loginout;
			strcpy(loginout.userName, "刘德华");
			client->sendData(&loginout);
		}
		else
		{
			printf("输入命令错误\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.connecttion("127.0.0.1", 4567);

	//EasyTcpClient client2;
	//client2.connecttion("127.0.0.1", 4567);

	// 启动线程
	std::thread t1(cmdThread, &client);
	t1.detach(); // 与主线程分离

	//// 启动线程
	//std::thread t2(cmdThread, &client2);
	//t2.detach(); // 与主线程分离

	// 循环输入请求命令
	while (client.isRun())
	{
		client.onRun();
		//client2.onRun();
		//printf("空闲时间处理其他业务\n");
	}

	client.closeSocket();
	//client2.closeSocket();

	printf("已退出");
	getchar();
	return 0;
}
