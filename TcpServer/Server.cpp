#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>

#include <vector>

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

// �������ӵĿͻ���socket
std::vector<SOCKET> g_clients;

int processor(SOCKET _clientSock)
{
	// ������
	char szRecv[1024] = {};

	// 5.���տͻ�������		�Ƚ���ͷ��ͨ��ͷ���жϽ��յ�ʲô����
	int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader *)szRecv;

	if (nLen <= 0)
	{
		printf("�ͻ���<Socket=%d>�˳����������\n", _clientSock);
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		// ���յ�¼��Ϣ		�����Ѿ����յ���ͷ����Ҫ����ƫ�ƣ���ȥͷ�ĳ���
		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login *login = (Login*)szRecv;
		printf("�յ��ͻ���<Socket=%d>�����¼����, ���ݳ��ȣ�%d, �û�����%s ���룺%s\n", _clientSock, login->dataLength, login->userName, login->password);
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
		printf("�յ��ͻ���<Socket=%d>����ǳ�����, ���ݳ��ȣ�%d, �û�����%s\n", _clientSock, loginout->dataLength, loginout->userName);

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

	while (true)
	{
		fd_set fdRead; // �����ɶ���������(socket)����
		fd_set fdWrite; // ��д��������(socket)����
		fd_set fdExcp; // ���쳣��������(socket)����

		// FD_ZERO ��ռ���
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcp);

		FD_SET(_serverSock, &fdRead); // ��������(socket)����fdRead����
		FD_SET(_serverSock, &fdWrite);
		FD_SET(_serverSock, &fdExcp);

		for (int n = (int)g_clients.size() - 1; n >= 0; --n)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		timeval timeva = { 1, 0 }; // {�룬΢��}

		// select�������ú󣬰ѿɶ���socket����fdRead���ѿ�д��socket����fdWrite
		// nfds��һ������ֵ����ָfd_set������������������socket���ķ�Χ�����������ļ����������ֵ+1��Windows��û�����壬����д0
		// ���һ������timeout��select()���ȴ�ʱ�䣬������������ΪNULL����timeout����Ϊ��ָ�룬��select��һֱ��������һ��������������������socket�仯�ˣ��ɶ����߿�д�������쳣��
		int res = select(_serverSock + 1, &fdRead, &fdWrite, &fdExcp, &timeva); // timevaΪ{0,0}��ʾ��������
		if (res < 0)
		{
			printf("select�������\n");
			break;
		}

		// �ж�������(socket)���ڿɶ���socket�����У��Ƿ���������Ҫ���գ�
		// �ڵ���select()��������FD_ISSET�����fd�Ƿ���set�����У�����⵽fd��set���򷵻��棬���򣬷��ؼ٣�0��
		if (FD_ISSET(_serverSock, &fdRead))
		{
			// �������ݣ�Ȼ��ӿɶ��������Ƴ�
			FD_CLR(_serverSock, &fdRead);
			// 4 accept �ȴ����տͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _clientSock = INVALID_SOCKET;

			_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _clientSock)
			{
				printf("���󣬽��յ���Ч�Ŀͻ���socket\n");
			}
			else
			{
				for (int n = (int)g_clients.size() - 1; n >= 0; --n)
				{
					NewUserJoin userJoin;
					send(g_clients[n], (const char *)&userJoin, sizeof(NewUserJoin), 0);
				}
				// �����¼���Ŀͻ���
				g_clients.push_back(_clientSock);
				printf("�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
			}
		}

		// ����ɶ��Ŀͻ�������
		for (size_t n = 0; n < fdRead.fd_count; ++n)
		{
			// �������-1����ʾ�ͻ����˳����Ӵ���ͻ��˵ļ������Ƴ�
			if (-1 == processor(fdRead.fd_array[n]))
			{
				// ָ����ʼ����λ��
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}

		printf("����ʱ�䴦������ҵ��\n");
	}

	// 7.�ر������׽���
	for (size_t n = g_clients.size() - 1; n >= 0; --n)
	{
		closesocket(g_clients[n]);
	}

	// ���Windows socket 2.x����
	WSACleanup();
	printf("���˳�");
	getchar();
	return 0;
}