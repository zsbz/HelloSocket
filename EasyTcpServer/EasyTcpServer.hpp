#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SICKET)(~0)
	#define SOCKET_ERROR	(-1)
#endif

#include <vector>
#include <stdio.h>

#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _serverSock;
	std::vector<SOCKET> g_clients;

public:
	EasyTcpServer()
	{
		_serverSock = INVALID_SOCKET;
	}

	virtual ~EasyTcpServer()
	{
		closeSocket();
	}

	// ��ʼ��Socket
	SOCKET initSocket()
	{
#ifdef _WIN32
		// ����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		// ֮ǰ�Ѿ���ʼ�����ر�ԭ����
		if (INVALID_SOCKET != _serverSock)
		{
			printf("<socket=%d>�رվɵ�socket\n", _serverSock);
			closeSocket();
		}

		_serverSock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _serverSock)
		{
			printf("����socketʧ��\n");
		}
		else
		{
			printf("����socket�ɹ�\n");
		}

		return _serverSock;
	}

	// �󶨶˿ں�
	int bindAddr(const char *ip, unsigned short port)
	{
		if (INVALID_SOCKET == _serverSock)
		{
			initSocket();
		}

		// 2.�󶨽��տͻ������ӵĶ˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); // host to net unsigned short

#ifdef _WIN32
		// ���ip��Ϊ�գ���ʹ�ô����ip
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr(127.0.0.1);
		}
#else
		// ���ip��Ϊ�գ���ʹ�ô����ip
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY; // inet_addr(127.0.0.1);
		}
#endif
		int res = bind(_serverSock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == res)
		{
			printf("���󣬰�����˿�<%d>ʧ��\n", port);
		}
		else
		{
			printf("������˿�<%d>�ɹ�\n", port);
		}

		return res;
	}
	
	// �����˿ں�
	int listenSocket(int n)
	{
		int res = listen(_serverSock, n); // ���n������
		if (SOCKET_ERROR == res)
		{
			printf("Socket=<%d>���󣬼����˿�ʧ��\n", _serverSock);
		}
		else
		{
			printf("Socket=<%d>��������˿ڳɹ�\n", _serverSock);
		}
		return res;
	}

	// ���ܿͻ�������
	SOCKET acceptSocket()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _clientSock = INVALID_SOCKET;

		_clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _clientSock)
		{
			printf("Socket=<%d>���󣬽��յ���Ч�Ŀͻ���socket\n", _serverSock);
		}
		else
		{
			NewUserJoin userJoin;
			sendDataToAll(&userJoin);

			// �����¼���Ŀͻ���
			g_clients.push_back(_clientSock);
			printf("Socket=<%d>�¿ͻ��˼��룺socket = %d, IP = %s \n", _serverSock, (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _clientSock;
	}

	// �ر�Socket
	void closeSocket()
	{
		// �ر�Win Sock2.x����
		if (_serverSock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (size_t n = g_clients.size() - 1; n >= 0; --n)
			{
				closesocket(g_clients[n]);
			}
			closesocket(_serverSock);
			// ���Windows socket 2.x����
			WSACleanup();
#else
			for (size_t n = g_clients.size() - 1; n >= 0; --n)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}
	}

	// ����������Ϣ
	bool onRun()
	{
		if (isRun())
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
				closeSocket();
				return false;
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
					// ���͸�ȫ���ͻ���
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
				if (-1 == RecvData(fdRead.fd_array[n]))
				{
					// ָ����ʼ����λ��
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}

			return true;
		}
		return false;
	}

	// �Ƿ�����
	bool isRun()
	{
		return _serverSock != INVALID_SOCKET;
	}

	// �������� ����ճ�� ��ְ�
	int RecvData(SOCKET _clientSock)
	{
		// ������
		char szRecv[10240] = {};

		// 5.���տͻ�������		�Ƚ���ͷ��ͨ��ͷ���жϽ��յ�ʲô����
		int nLen = recv(_clientSock, szRecv, sizeof(DataHeader), 0);
		DataHeader *header = (DataHeader *)szRecv;

		if (nLen <= 0)
		{
			printf("�ͻ���<Socket=%d>�˳����������\n", _clientSock);
			return -1;
		}

		recv(_clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		onNetMsg(_clientSock, header);
		return 0;
	}

	// ��Ӧ������Ϣ
	virtual void onNetMsg(SOCKET _clientSock, DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login*)header;
			printf("�յ��ͻ���<Socket=%d>�����¼����, ���ݳ��ȣ�%d, �û�����%s ���룺%s\n", _clientSock, login->dataLength, login->userName, login->password);
			// �ж��û���������ȷ

			// ���ص�¼���
			LoginResult result;
			sendData(_clientSock, &result);
		}
		break;
		case CMD_LOGINOUT:
		{
			Loginout *loginout = (Loginout *)header;
			printf("�յ��ͻ���<Socket=%d>����ǳ�����, ���ݳ��ȣ�%d, �û�����%s\n", _clientSock, loginout->dataLength, loginout->userName);

			// ���صǳ����
			LoginoutResult result;
			sendData(_clientSock, &result);
		}
		break;
		default:
		{
			DataHeader tempHeader = { 0, CMD_ERROR };
			sendData(_clientSock, &tempHeader);
		}
		break;
		}
	}
	// ���͸�ָ��socket
	int sendData(SOCKET _clientSock, DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	// ���͸�ȫ���ͻ���socket
	void sendDataToAll(DataHeader *header)
	{
		if (isRun() && header)
		{
			// ���͸�ȫ���ͻ���
			for (int n = (int)g_clients.size() - 1; n >= 0; --n)
			{
				sendData(g_clients[n], header);
			}
		}
	}
};


#endif // !_EasyTcpServer_hpp_