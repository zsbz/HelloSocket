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
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("����socketʧ��\n");
	}
	else
	{
		printf("����socket�ɹ�\n");
	}
	// 2.���ӷ�����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		printf("����socketʧ��\n");
	}
	else
	{
		printf("����socket�ɹ�\n");
	}

	// ѭ��������������
	while (true)
	{
		// 3.������������
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);
		// 4.������������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			break;
		}
		else
		{
			// 5.�������������������
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		// 6.���շ�������Ϣ
		char recvBuf[256] = {};
		int nlen = recv(_sock, recvBuf, 256, 0);
		if (nlen > 0)
		{
			printf("���յ������ݣ�%s \n", recvBuf);
		}
	}


	// 7.�ر��׽���
	closesocket(_sock);

	// ���Windows socket 2.x����
	WSACleanup();
	printf("���˳�");
	getchar();
	return 0;
}
