#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>
#include <set>

using namespace std;

set <int> clnt_list;


void usage() {
	cout << "syntax : echo-server <port> [-e[-b]] \n";
    cout << "sample : echo-server 1234 -e -b \n";
}

struct Param {
	bool echo{false};
    bool bcast{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
            if(strncmp(argv[i], "-e", 2)==0){
                echo = true;
                continue;
            }
            else if(strncmp(argv[i], "-b", 2)==0){
                bcast = true;
                continue;
            }
			port = stoi(argv[i]);
		}

        if (!echo && bcast){
            usage();
            exit(1);
        }
        if (echo) {
            if (bcast) {
                cout << "broadcast mode server!\n";
            } else {
                cout << "echo mode server!\n";
            }
        }
		return port != 0;
	}
} param;

void recvThread(int sd) {
    int num = sd -3;
	cout << "client number " << num << " connected\n";
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	while (true) {
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res;
			perror(" ");
			break;
		}
		buf[res] = '\0';
		cout << "message from client number " << num << " : " << buf;
		cout.flush();

        if (param.bcast) {
            for(auto i:clnt_list) {
                res = send(i, buf, res, 0);
                if (res == 0 || res == -1) {
                    cerr << "send return " << res;
                    perror(" ");
                    break;
                }
            }
        }
        else if (param.echo) {
			res = send(sd, buf, res, 0);
			if (res == 0 || res == -1) {
				cerr << "send return " << res;
				perror(" ");
				break;
			}
		}
	}
    clnt_list.erase(sd);
	cout << "client number "  << num << " disconnected\n";
	close(sd);
}

int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int res;
    int optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}

        clnt_list.insert(cli_sd);

		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
