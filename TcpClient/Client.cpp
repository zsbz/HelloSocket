#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
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
	// ������
	char szRecv[1024] = {};

	// 5.���տͻ�������		�Ƚ���ͷ��ͨ��ͷ���жϽ��յ�ʲô����
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader *)szRecv;
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ��������\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult *loginRes = (LoginResult*)szRecv;
		printf("�յ���������Ϣ����¼���, ���ݳ��ȣ�%d\n", loginRes->dataLength);
	}
	break;
	case CMD_LOGINOUT_RESULT:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult *loginoutRes = (LoginoutResult*)szRecv;
		printf("�յ���������Ϣ���ǳ����, ���ݳ��ȣ�%d\n", loginoutRes->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin *userJoin = (NewUserJoin*)szRecv;
		printf("�յ���������Ϣ�����û�����, ���ݳ��ȣ�%d\n", userJoin->dataLength);
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
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "���»�");
			strcpy(login.password, "123456");
			send(_sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "loginout"))
		{
			Loginout loginout;
			strcpy(loginout.userName, "���»�");
			send(_sock, (const char*)&loginout, sizeof(Loginout), 0);
		}
		else
		{
			printf("�����������\n");
		}
	}
}

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

	// �����߳�
	std::thread t1(cmdThread, _sock);
	t1.detach(); // �����̷߳���
	g_bRun = true;

	// ѭ��������������
	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		timeval timeva = { 1, 0 };
		int res = select(_sock, &fdReads, 0, 0, &timeva);
		if (res < 0)
		{
			printf("select�������1\n");
			break;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads); 
			if (-1 == processor(_sock))
			{
				printf("select�������2\n");
				break;
			}
		}

		//printf("����ʱ�䴦������ҵ��\n");
	}

	// 7.�ر��׽���
	closesocket(_sock);

	// ���Windows socket 2.x����
	WSACleanup();
	printf("���˳�");
	getchar();
	return 0;
}
