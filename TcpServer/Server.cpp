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
		// ������
		char szRecv[1024] = {};

		// 5.���տͻ�������		�Ƚ���ͷ��ͨ��ͷ���жϽ��յ�ʲô����
		int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;
		if (nLen <= 0)
		{
			printf("�ͻ����˳����������");
			break;
		}

		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			// ���յ�¼��Ϣ		�����Ѿ����յ���ͷ����Ҫ����ƫ�ƣ���ȥͷ�ĳ���
			recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login *login = (Login*)szRecv;
			printf("�յ������¼����, ���ݳ��ȣ�%d, �û�����%s ���룺%s\n", login->dataLength, login->userName, login->password);
			// �ж��û���������ȷ

			// ���ص�¼���
			LoginResult result;
			send(_clientSock, (const char *)&result, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			// ���յǳ���Ϣ
			recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Loginout *loginout = (Loginout *)szRecv;
			printf("�յ�����ǳ�����, ���ݳ��ȣ�%d, �û�����%s\n", loginout->dataLength, loginout->userName);

			// ���صǳ����
			LoginoutResult result;
			send(_clientSock, (const char *)&result, sizeof(LoginoutResult), 0);
		}
		break;
		default:
		{
			DataHeader header = { 0, CMD_ERROR };
			send(_clientSock, (const char *)&header, sizeof(DataHeader), 0);
		}
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
