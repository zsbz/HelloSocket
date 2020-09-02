#include "EasyTcpClient.hpp"

#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
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

		// һֱ�������󣬲��Դ�������ʱ�Ƿ�ճ��
		memcpy(cmdBuf, "login", sizeof("login"));

		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->closeSocket();
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "���»�");
			strcpy(login.password, "123456");
			client->sendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			Loginout loginout;
			strcpy(loginout.userName, "���»�");
			client->sendData(&loginout);
		}
		else
		{
			printf("�����������\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.connecttion("127.0.0.1", 4567);

	//EasyTcpClient client2;
	//client2.connecttion("127.0.0.1", 4567);

	// �����߳�
	std::thread t1(cmdThread, &client);
	t1.detach(); // �����̷߳���

	//// �����߳�
	//std::thread t2(cmdThread, &client2);
	//t2.detach(); // �����̷߳���

	// ѭ��������������
	while (client.isRun())
	{
		client.onRun();
		//client2.onRun();
		//printf("����ʱ�䴦������ҵ��\n");
	}

	client.closeSocket();
	//client2.closeSocket();

	printf("���˳�");
	getchar();
	return 0;
}
