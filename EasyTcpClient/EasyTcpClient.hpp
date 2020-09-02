#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#define WIN32_LEAN_AND_MEAN // ����Windows��WinSock2�궨���ͻ������һ�ֽ���취�ǰ�WinSock2������Windowsǰ
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
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

#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpClient()
	{
		closeSocket();
	}

	// ��ʼ��socket
	void initSocket()
	{
#ifdef _WIN32
		// ����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		// ֮ǰ�Ѿ���ʼ�����ر�ԭ����
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվɵ�socket\n", _sock);
			closeSocket();
		}

		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock)
		{
			printf("����socketʧ��\n");
		}
		else
		{
			printf("����socket�ɹ�\n");
		}
	}

	// ���ӷ�����
	int connecttion(char *ip, short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			initSocket();
		}

		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("����socketʧ��\n");
		}
		else
		{
			printf("����socket�ɹ�\n");
		}
		return ret;
	}

	// �ر�socket
	void closeSocket()
	{
		// �ر�Win Sock2.x����
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			// ���Windows socket 2.x����
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	// ���ջ�������С��Ԫ��С
#define RECV_BUFF_SIZE 10240
	// ���ջ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	// �ڶ�����������Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	// ��Ϣ����������β��λ��
	int _lastPos = 0;

	// ��������  ����ճ�� ��ְ�
	int RecvData(SOCKET _sock)
	{
		// ��������  ϵͳsocket��һ��Ĭ�ϻ�����������ʹ��RECV_BUFF_SIZE��ÿ�ΰ�ϵͳ������������ȫ��ȡ������������Ϣ����
		int nLen = recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			printf("��������Ͽ����ӣ��������\n");
			return -1;
		}

		// ����ȡ�����ݿ�������Ϣ������β��
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		// ��Ϣ������������β��λ�ú���
		_lastPos += nLen;

		// �ж���Ϣ�����������ݳ��ȴ��ڵ�����ϢͷDataHeader���ȣ�������Ϣͷ���ȾͿ���ת����Ϣͷ,Ȼ���ٴ�ѭ��
		while (_lastPos >= sizeof(DataHeader))
		{
			// ת����Ϣͷ���Ϳ��Ը�����Ϣͷ֪����Ϣ�ĳ���
			DataHeader *header = (DataHeader *)_szMsgBuf;
			// �ж���Ϣ��������ȫ�����ݴ��ڵ�����Ϣ���ȣ����ھͱ�ʾ�ð��Լ�ȫ�����յ���
			if (_lastPos >= header->dataLength)
			{
				// ʣ��δ������Ϣ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				// ����������Ϣ
				onNetMsg(header);
				// ����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// ��Ϣ������������β��λ��ǰ��
				_lastPos = nSize;
			}
			else
			{
				// ��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}

		return 0;
	}

	// ��Ӧ��������  �������麯����������Ҫ�����������͵����ݣ������ѯ��Ʒ�ȿ��Լ̳и��࣬Ȼ��ʵ������麯��
	virtual void onNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *loginRes = (LoginResult*)header;
			printf("<scoket=%d>�յ���������Ϣ����¼���, ���ݳ��ȣ�%d\n", _sock,loginRes->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginoutResult *loginoutRes = (LoginoutResult*)header;
			printf("<scoket=%d>�յ���������Ϣ���ǳ����, ���ݳ��ȣ�%d\n", _sock, loginoutRes->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{	
			NewUserJoin *userJoin = (NewUserJoin*)header;
			printf("<scoket=%d>�յ���������Ϣ�����û�����, ���ݳ��ȣ�%d\n", _sock, userJoin->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			printf("<scoket=%d>�յ�������������Ϣ�����ݳ��ȣ�%d\n", _sock, header->dataLength);
		}
		break;
		default:
		{
			printf("<scoket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d\n", _sock, header->dataLength);
		}
		break;
		}
	}

	// ��������
	bool onRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);

			timeval timeva = { 1, 0 };
			int res = select(_sock, &fdReads, 0, 0, &timeva);
			if (res < 0)
			{
				printf("select�������1\n");
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// ��������
	int sendData(DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
};


#endif // !_EasyTcpClient_hpp_
