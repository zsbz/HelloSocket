#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
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

// ��ͷ
struct DataHeader
{
	short dataLength; // ����
	short cmd;		  // ��������
};

// ��¼ DataPackage
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			// 5.�������������������
			Login login = { "���»�", "123456" };
			DataHeader header = { sizeof(login), CMD_LOGIN};

			send(_sock, (const char *)&header, sizeof(header), 0);
			send(_sock, (const char *)&login, sizeof(Login), 0);

			// ���շ�������������
			DataHeader resultHeader = {};
			LoginResult loginResult = {};
			recv(_sock, (char *)&resultHeader, sizeof(resultHeader), 0);
			recv(_sock, (char *)&loginResult, sizeof(loginResult), 0);
			printf("LoginResult��%d \n", loginResult.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			// 5.�������������������
			Loginout loginout = { "���»�"};
			DataHeader header = { sizeof(loginout), CMD_LOGIN_OUT};
			send(_sock, (const char *)&header, sizeof(DataHeader), 0);
			send(_sock, (const char *)&loginout, sizeof(Loginout), 0);

			// ���շ�������������
			DataHeader resultHeader = {};
			LoginoutResult loginoutResult = {};
			recv(_sock, (char *)&resultHeader, sizeof(resultHeader), 0);
			recv(_sock, (char *)&loginoutResult, sizeof(loginoutResult), 0);
			printf("LoginoutResult��%s \n", loginoutResult.userName);
		} 
		else
		{
			printf("��֧�ֵ����\n");
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
