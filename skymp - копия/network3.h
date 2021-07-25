#pragma once
#include <ctime>
#include <thread>
#include "skyplug.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h> 
#pragma comment( lib, "wsock32.lib" )
#include <windows.h>
#pragma warning(disable : 4996)

#define PRINTF ScriptDragon::PrintNote
#define MSGBOX ScriptDragon::Debug::ShowMessageBox
#define IDENT 108i8

namespace network
{
	player_t *_player = nullptr;

	/*void deserialize_all(char *buf, const char *const end)
	{
		char *pos = nullptr;
		for (auto i = buf; i != end; ++i)
		{
			if (*i == '|')
			{
				*i = 0;
				pos = ++i;
				break;
			}
		}
		if (!pos)
			return;
		static packet_t tmp;
		tmp.deserialize(buf);
		if (tmp.id >= 0 && tmp.id < 1000)
		{
			other_players[tmp.id].deserialize(buf);
			if (!other_players[tmp.id].is_active)
			{
				other_players[tmp.id].is_active = true;
			}
		}
		PRINTF("Synced ID%d", tmp.id);

		network::deserialize_all(pos, end);
	}*/

	void sync(SOCKET udp_sock, sockaddr_in dest_addr)
	{
		char buf[128];
		sockaddr from;
		int fromlen = sizeof from, size = SOCKET_ERROR;
		for (;;)
		{
			auto msg = my_data.serialize();
			sendto(udp_sock, msg, strlen(msg), 0,
				(sockaddr *)&dest_addr, sizeof dest_addr);
			if ((size = recvfrom(udp_sock, buf, sizeof buf, 0, &from, &fromlen)) > 0)
			{
				buf[size] = NULL;

				size_t index = 0;
				auto id_ = std::stoul(buf, &index);
				if (id_ >= 0 && id_ < 1000)
				{
					other_players[id_].deserialize(&buf[1 + index]);
					if (!other_players[id_].is_active)
					{
						other_players[id_].is_active = true;
						PRINTF("add player");
						skyplug::id_to_add = id_;
						//while (skyplug::id_to_add == id_) { Sleep(1); }
					}
				}
			}
			Sleep(20);
		}
	}

	void connect(SOCKET tcp_sock, const char *ip, unsigned short port, const char* name)
	{
		sockaddr_in dest_addr;
		const hostent *hst;
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(port);
		if (inet_addr(ip) != INADDR_NONE)
			dest_addr.sin_addr.s_addr = inet_addr(ip);
		else if (hst = gethostbyname(ip))
			((unsigned long *)&dest_addr.sin_addr)[0] = ((unsigned long **)hst->h_addr_list)[0][0];

		if (WSAGetLastError())
			return (void)PRINTF("Неизвестная ошибка [%d]", WSAGetLastError());

		PRINTF("Подключение ...");
	connecting:
		while (connect(tcp_sock, (sockaddr *)&dest_addr, sizeof dest_addr))
		{
			PRINTF("Сервер не отвечает, повторная попытка подключения ...");
			Sleep(3000);
		}

		int recvsize = -2;
		const size_t max_name(25);
		char buff[1024]{ 0 };
		do
		{
			if (recvsize != -2)
			{
				
				if (!strcmp("timeout", buff))
					return (void)MSGBOX("Сервер закрыл соединение");
				else if (!strcmp("fail", buff))
					return skyplug::quit_with_msg("К серверу уже подключен игрок с этого IP-адреса");
				else if (!strcmp("no_slots", buff))
				{
					PRINTF("Нет свободных мест, повторная попытка подключения ...");
					goto connecting;
				}
				else if(!memcmp("OK", buff, 2))
				{
					const auto _port = std::stoul(&buff[2]);
					dest_addr.sin_port = htons(_port);
					const SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, NULL);
					static u_long non_blocking = true;
					ioctlsocket(udp_sock, FIONBIO, &non_blocking);
					std::thread(network::sync, udp_sock, dest_addr).detach();
				}
				else if (!memcmp("ID", buff, 2))
				{
					const auto _id = std::stoul(&buff[2]);
					PRINTF("Добро пожаловать, %s[%d]", name, my_id = _id);
					connected_to_server = true;
				}
			}

			strcpy(&buff[3], name);
			buff[2] = strlen(name) + 1;
			buff[0] = 
			buff[1] = IDENT;
			send(tcp_sock, buff, max_name, 0);

		} while (recvsize = recv(tcp_sock, buff, sizeof buff, NULL) != SOCKET_ERROR);
		//PRINTF("Соединение было разорвано [%d]", WSAGetLastError());
		MSGBOX("Неизвестная ошибка привела к разрыву соединения");
	}

	void run(const char* ip, unsigned short port, const char* name)
	{
		WSAData data;
		WSAStartup(0x202, &data);

		const SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, NULL);
		std::thread(network::connect, tcp_sock, ip, port, name).detach();
	}
}