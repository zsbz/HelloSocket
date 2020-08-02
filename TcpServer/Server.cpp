#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	// ����Windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	// 1.����socket
	SOCKET _serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 2.�󶨽��տͻ������ӵĶ˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr(127.0.0.1);
	if (SOCKET_ERROR == bind(_serverSock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("���󣬰�����˿�ʧ��\n");
	}
	else
	{
		printf("������˿ڳɹ�\n");
	}
	// 3 listen ��������˿�
	if (SOCKET_ERROR == listen(_serverSock, 5))// ���5������
	{
		printf("���󣬼����˿�ʧ��\n");
	}
	else
	{
		printf("��������˿ڳɹ�\n");
	}

	// 4 accept �ȴ����տͻ�������
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _clientSock = INVALID_SOCKET;

	_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _clientSock)
	{
		printf("���󣬽��յ���Ч�Ŀͻ���socket\n");
	}
	printf("�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));

	char _recvBuf[128] = {};
	while (true)
	{
		// 5.���տͻ�������
		int nLen = recv(_clientSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("�ͻ����˳����������");
			break;
		}
		printf("�յ����%s\n", _recvBuf);
		// 6.�������󣬸��ݲ�ͬ�����Ͳ�ͬ����
		if (0 == strcmp(_recvBuf, "getName"))
		{
			char msgBuf[] = "���»�";
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

	// 7.�ر��׽���
	closesocket(_serverSock);

	// ���Windows socket 2.x����
	WSACleanup();
	printf("���˳�");
	getchar();
	return 0;

}
