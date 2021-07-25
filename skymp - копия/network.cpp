/*#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> 
#pragma comment( lib, "wsock32.lib" )
#include <windows.h>

#include "network.h"

#include "skyplug.h"

std::deque<std::string> network::send_data;
char network::receive_buffer[1024];

size_t network::bytes_to_send = 0;
clock_t network::last_send = 0,
	network::last_receive = 0;


void network::connect_(void *socket__, void *addr, const char *name_)
{
	printf("start_connect\n");
	const SOCKET& socket_ = *(SOCKET *)socket__;
	PRINTF("Подключение ...\n");
	Sleep(500);
	while (connect(socket_, (sockaddr *)addr, sizeof(sockaddr)))
	{
		PRINTF("Не удалось подключиться\n");
		PRINTF("Повторная попытка ...\n");
		Sleep(3000);
	}
	//network::non_blocking(socket__);
	PRINTF("Сервер SkyMP приветствует Вас\n");
	connected_to_server = true;

	char tmp[128]{ 0 };
	clock_t ping_tmp = 0;
	bool received_once = false, sent_once = false;
	while (1)
	{
		Sleep(10);

		clock_t time_on_iteration_start = clock();
		int bytes_received = 0;

		clock_t const max_clock = clock() + 10;
		///while(clock() < max_clock)
		///{
			// принятие пакета
			bytes_received = recv(socket_, &receive_buffer[0], 1024, 0);
			if (bytes_received > 0)
			{
				if (ping_tmp != 0)
					ping_tmp = 0;
				//использование принятого пакета

				if (receive_buffer[0] == 'd' &&
					receive_buffer[1] == 'i' &&
					receive_buffer[2] == 's' &&
					receive_buffer[3] == 'c' &&
					receive_buffer[4] == 'o' &&
					receive_buffer[5] == 'n' &&
					receive_buffer[6] == 'n')
				{
					MSGBOX("Сервер разорвал соединение\n");
					return;
				}

				received_once = true;
			}
			///else
			///	break;
		///}

		PRINTF("nooo");
		// создание ответного пакета

		std::string str;
		if (sent_once)
		{
		}
		//else
		{
			PRINTF("hopa popa");
			std::string name_str(name_);
			int sz = sprintf(tmp, "%c%c%c%s", IDENT, IDENT, name_str.length() + 1, name_str.data());
			str = tmp;
			str[sz] = 0;
			send(socket_, str.data(), str.size(), 0);
			ping_tmp = clock();
			sent_once = 1;
		}
		//printf("delay = '%d'\n", clock() - time_on_iteration_start);
	}
}

sockaddr_in make_sockaddr(const char* ip, unsigned short port)
{
	HOSTENT *hst;
	sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);

	auto addr = inet_addr(ip);
	if (addr)
		dest_addr.sin_addr.s_addr = addr;
	else if (hst = gethostbyname(ip))
		dest_addr.sin_addr.s_addr = ((unsigned long **)hst->h_addr_list)[0][0];
	else
		throw 1;
	return dest_addr;
}

void network::run(const char* ip, unsigned short port, const char *name)
{
	try
	{
		SOCKET socket_;
		startup(&socket_);
		sockaddr_in remote_endpoint = make_sockaddr(ip, port);
		network::connect_(&socket_, &remote_endpoint, name);
		network::non_blocking(&socket_);
		return;
	}
	catch (std::exception& e)
	{
		PRINTF("%s\n", e.what());
	}
	catch (...)
	{
		network::on_error();
	}
}

void network::startup(void *socket_)
{
	WSAData data;
	if (WSAStartup(0x202, &data) != NO_ERROR)
		throw 1;

	*(SOCKET *)socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (*(SOCKET *)socket_ == INVALID_SOCKET)
		throw 1;
}

void network::non_blocking(void *socket_)
{
	DWORD nonBlocking = 1;
	if (ioctlsocket(*(SOCKET *)socket_, FIONBIO, &nonBlocking) != 0)
	throw 1;
}

void network::on_error()
{
	PRINTF("Failed to start the client: Error %d\n", WSAGetLastError());
	WSACleanup();
}*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> 
#pragma comment( lib, "wsock32.lib" )
#include <windows.h>

#include "network.h"

#include "skyplug.h"

std::deque<std::string> network::send_data;
char network::receive_buffer[1024];

size_t network::bytes_to_send = 0;
clock_t network::last_send = 0,
network::last_receive = 0;


void network::connect_(void *socket__, void *addr, const char *name_)
{
	printf("start_connect\n");
	const SOCKET& socket_ = *(SOCKET *)socket__;
	PRINTF("Подключение ...\n");
	Sleep(500);
	while (connect(socket_, (sockaddr *)addr, sizeof(sockaddr)))
	{
		PRINTF("Не удалось подключиться\n");
		PRINTF("Повторная попытка ...\n");
		Sleep(3000);
	}

	char tmp[128]{ 0 };
	clock_t ping_tmp = 0;
	while (1)
	{
		clock_t time_on_iteration_start = clock();
		int bytes_received = 0;

		clock_t const max_clock = clock() + 10
			;
		while (clock() < max_clock)
		{
			// принятие пакета
			//if ((bytes_received = recv(socket_, &receive_buffer[0], 1024, 0)) <= 0)
			//	break;
			bytes_received = recv(socket_, &receive_buffer[0], 1024, 0);
			if (bytes_received > 0)
			{
				if (ping_tmp != 0)
				{
					//printf("PING = %d\n", clock() - ping_tmp);
					ping_tmp = 0;
				}
				//использование принятого пакета
				receive_buffer[14] = 0; // на всякий пожарный
										///printf(receive_buffer);
										///printf("\n");
			}
			else
				break;
		}

		// создание ответного пакета
		std::string str;
		std::string name_str(name_);
		int sz = sprintf(tmp, "%c%c%c%s", IDENT, IDENT, name_str.length() + 1, name_str.data());
		str = tmp;
		str[sz] = 0;
		(send(socket_, str.data(), str.size(), 0));
		//printf("sent %s\n", str.data());
		ping_tmp = clock();

		printf("delay = '%d'\n", clock() - time_on_iteration_start);
	}
}

sockaddr_in make_sockaddr(const char* ip, unsigned short port)
{
	HOSTENT *hst;
	sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);

	auto addr = inet_addr(ip);
	if (addr)
		dest_addr.sin_addr.s_addr = addr;
	else if (hst = gethostbyname(ip))
		dest_addr.sin_addr.s_addr = ((unsigned long **)hst->h_addr_list)[0][0];
	else
		throw 1;
	return dest_addr;
}

void network::run(const char* ip, unsigned short port, const char *name)
{
	try
	{
		SOCKET socket_;
		startup(&socket_);
		sockaddr_in remote_endpoint = make_sockaddr(ip, port);
		network::connect_(&socket_, &remote_endpoint, name);
		return;
	}
	catch (std::exception& e)
	{
		PRINTF("%s\n", e.what());
	}
	catch (...)
	{
		network::on_error();
	}
}

void network::startup(void *socket_)
{
	WSAData data;
	if (WSAStartup(0x202, &data) != NO_ERROR)
		throw 1;

	*(SOCKET *)socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (*(SOCKET *)socket_ == INVALID_SOCKET)
		throw 1;
}

void network::non_blocking(void *socket_)
{
	DWORD nonBlocking = 1;
	if (ioctlsocket(*(SOCKET *)socket_, FIONBIO, &nonBlocking) != 0)
		throw 1;
}

void network::on_error()
{
	PRINTF("Failed to start the client: Error %d\n", WSAGetLastError());
	WSACleanup();
}