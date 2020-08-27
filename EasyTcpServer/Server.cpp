#include "EasyTcpServer.hpp"

int main()
{
	EasyTcpServer server;
	server.initSocket();
	server.bindAddr(nullptr, 4567);
	server.listenSocket(5);

	while (server.isRun())
	{
		server.onRun();
	}
	server.closeSocket();
	
	printf("ÒÑÍË³ö");
	getchar();
	return 0;
}