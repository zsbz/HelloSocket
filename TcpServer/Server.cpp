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
		DataHeader header = {};
		// 5.���տͻ�������
		int nLen = recv(_clientSock, (char *)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("�ͻ����˳����������");
			break;
		}
		printf("�յ����%d, ���ݳ��ȣ�%d\n", header.cmd, header.dataLength);

		switch (header.cmd)
		{
		case CMD_LOGIN:
		{
			// ���յ�¼��Ϣ
			Login login = {};
			recv(_clientSock, (char *)&login, sizeof(Login), 0);
			// �ж��û���������ȷ

			// ���ص�¼���  �ȷ�ͷ
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			LoginResult result = { 1 };
			send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGIN_OUT:
		{
			// ���յǳ���Ϣ
			Loginout loginout = {};
			recv(_clientSock, (char *)&loginout, sizeof(Loginout), 0);

			// ���صǳ����  �ȷ�ͷ
			LoginoutResult result = { "�ɹ�" };
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			send(_clientSock, (const char *)&result, sizeof(LoginoutResult), 0);
		}
		break;
		default:
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
			break;
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
