#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#define WIN32_LEAN_AND_MEAN // 避免Windows和WinSock2宏定义冲突，还有一种解决办法是把WinSock2导入在Windows前
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

	// 初始化socket
	void initSocket()
	{
#ifdef _WIN32
		// 启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

		// 之前已经初始化，关闭原来的
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧的socket\n", _sock);
			closeSocket();
		}

		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock)
		{
			printf("建立socket失败\n");
		}
		else
		{
			printf("建立socket成功\n");
		}
	}

	// 连接服务器
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
			printf("连接socket失败\n");
		}
		else
		{
			printf("连接socket成功\n");
		}
		return ret;
	}

	// 关闭socket
	void closeSocket()
	{
		// 关闭Win Sock2.x环境
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			// 清除Windows socket 2.x环境
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	// 接收缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
	// 接收缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	// 第二缓冲区，消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	// 消息缓冲区数据尾部位置
	int _lastPos = 0;

	// 接收数据  处理粘包 拆分包
	int RecvData(SOCKET _sock)
	{
		// 接收数据  系统socket有一个默认缓冲区，这里使用RECV_BUFF_SIZE，每次把系统缓冲区的数据全部取出来，避免消息堵塞
		int nLen = recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			printf("与服务器断开连接，任务结束\n");
			return -1;
		}

		// 将收取的数据拷贝到消息缓冲区尾部
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		// 信息缓冲区的数据尾部位置后移
		_lastPos += nLen;

		// 判断消息缓冲区的数据长度大于等于消息头DataHeader长度，大于消息头长度就可以转成消息头,然后再次循环
		while (_lastPos >= sizeof(DataHeader))
		{
			// 转成消息头，就可以根据消息头知道消息的长度
			DataHeader *header = (DataHeader *)_szMsgBuf;
			// 判断消息缓冲区的全部数据大于等于信息长度，大于就表示该包以及全部接收到了
			if (_lastPos >= header->dataLength)
			{
				// 剩余未处理消息缓冲区数据的长度
				int nSize = _lastPos - header->dataLength;
				// 处理网络消息
				onNetMsg(header);
				// 将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				// 消息缓冲区的数据尾部位置前移
				_lastPos = nSize;
			}
			else
			{
				// 消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}

		return 0;
	}

	// 响应网络数据  这里用虚函数，假如需要处理其他类型的数据，比如查询商品等可以继承该类，然后实现这个虚函数
	virtual void onNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *loginRes = (LoginResult*)header;
			printf("<scoket=%d>收到服务器消息：登录结果, 数据长度：%d\n", _sock,loginRes->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginoutResult *loginoutRes = (LoginoutResult*)header;
			printf("<scoket=%d>收到服务器消息：登出结果, 数据长度：%d\n", _sock, loginoutRes->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{	
			NewUserJoin *userJoin = (NewUserJoin*)header;
			printf("<scoket=%d>收到服务器消息：新用户加入, 数据长度：%d\n", _sock, userJoin->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			printf("<scoket=%d>收到服务器错误消息，数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		default:
		{
			printf("<scoket=%d>收到未定义消息，数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		}
	}

	// 处理数据
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
				printf("select任务结束1\n");
				return false;
			}

			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// 发送数据
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
