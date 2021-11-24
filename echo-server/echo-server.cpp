#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux
#ifdef WIN32
#include <winsock2.h>
#include "../mingw_net.h"
#endif // WIN32
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#ifdef WIN32
void perror(const char* msg) { fprintf(stderr, "%s %ld\n", msg, GetLastError()); }
#endif // WIN32

void usage() {
	cout << "syntax : echo-server <port> [-e[-b]]\n";
	cout << "sample : echo-server 1234\n";
}

struct Param {
	bool echo{ false };
	bool bc{ false };
	uint16_t port{ 0 };

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				cout << "echo mode\n";
				continue;
			}
			if (strcmp(argv[i], "-b") == 0) {
				bc = true;
				cout << "broadcast mode\n";
				continue;
			}
			port = stoi(argv[i++]);
		}
		return port != 0;
	}
} param;
static const int BUFSIZE = 65536;
char buf[BUFSIZE];
vector<int> total_sds;


void recvThread(int sd) {
	cout << "connected\n";

	while (true) {
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res;
			perror(" ");
			break;
		}
		buf[res] = '\0';
		cout << buf;
		cout.flush();
		if (param.echo) {
			res = send(sd, buf, res, 0);
			if (res == 0 || res == -1) {
				cerr << "send return " << res;
				perror(" ");
				break;
			}
		}
	}
	cout << "disconnected\n";
	close(sd);
}
// sending thread impl. if thread invokes sendthread, it will send to all clients under broadcast mode

void sendThread() {
	cout << "send thread started\n";

	while (true) {
		// sleep until buf is filled.
		while (buf[0] == '\0') {
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}

		cout << "SENDING" << buf << endl;


		if (param.bc) {
			for (int sd : total_sds) {
				if (send(sd, buf, strlen(buf), 0) == -1) {
					perror("send");
					continue;
				}
			}
		}
		else {
			if (send(0, buf, strlen(buf), 0) == -1) {
				perror("send");
				continue;
			}
		}
		buf[0] = '\0';
	}

}



int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

#ifdef WIN32
	WSAData wsaData;
	WSAStartup(0x0202, &wsaData);
#endif // WIN32

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int res;
#ifdef __linux__
	int optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}
#endif // __linux

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr*)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}

	thread* send_th = new thread(sendThread);
	send_th->detach();

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(sd, (struct sockaddr*)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}
		thread* t = new thread(recvThread, cli_sd);
		t->detach();
		total_sds.push_back(cli_sd);
	}
	close(sd);
}
