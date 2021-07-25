#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#pragma warning(disable : 4996)

#include <array>
#include <deque>
#include <string>
#include <ctime>

#define PRINTF ScriptDragon::PrintNote
#define MSGBOX ScriptDragon::Debug::ShowMessageBox
#define IDENT (108I8)
class network
{
public:
	static void run(const char *ip, unsigned short port, const char *name);
	static std::deque<std::string> send_data;

private:
	static char receive_buffer[1024];
	static size_t bytes_to_send;
	static clock_t last_send;
	static clock_t last_receive;
	static void startup(void *socket_);
	static void on_error();
	static void connect_(void *socket_, void *addr, const char *name);
	static void non_blocking(void *socket_);
	network(const network&) {}
	network& operator=(const network&) {}
};