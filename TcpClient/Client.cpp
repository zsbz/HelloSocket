#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
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

// ��ͷ
struct DataHeader
{
	short dataLength; // ����
	short cmd;		  // ��������
};

// ��¼ DataPackage
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
			Login login;
			strcpy(login.userName, "���»�");
			strcpy(login.password, "123456");

			send(_sock, (const char *)&login, sizeof(Login), 0);

			// ���շ�������������
			LoginResult loginResult = {};
			recv(_sock, (char *)&loginResult, sizeof(loginResult), 0);
			printf("��¼�����%d \n", loginResult.result);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			// 5.�������������������
			Loginout loginout;
			strcpy(loginout.userName, "���»�");
			send(_sock, (const char *)&loginout, sizeof(Loginout), 0);

			// ���շ�������������
			LoginoutResult loginoutResult = {};
			recv(_sock, (char *)&loginoutResult, sizeof(loginoutResult), 0);
			printf("�ǳ������%d \n", loginoutResult.result);
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
