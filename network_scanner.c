#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define TIMEOUT 1

int ports[] = {80, 22, 109, 9100, 111}; 
int num_ports = sizeof(ports) / sizeof(ports[0]);

const char* ip_addresses[] = {
    "172.20.0.35",
    "172.20.0.60",
    "172.20.0.56"
};
int num_ips = sizeof(ip_addresses) / sizeof(ip_addresses[0]);

void scan_ip(const char* ip) {
    for (int i = 0; i < num_ports; i++) {
        int sockfd;
        struct sockaddr_in target;
        struct timeval timeout;
        fd_set fdset;

        printf("Scanning IP: %s, Port: %d\n", ip, ports[i]);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Socket creation failed");
            continue;
        }

        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        target.sin_family = AF_INET;
        target.sin_port = htons(ports[i]);
        inet_pton(AF_INET, ip, &target.sin_addr);

        connect(sockfd, (struct sockaddr *)&target, sizeof(target));

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);

        if (select(sockfd + 1, NULL, &fdset, NULL, &timeout) > 0) {
            int so_error;
            socklen_t len = sizeof so_error;

            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

            if (so_error == 0) {
                printf("Port %d open on %s\n", ports[i], ip);
            } else {
                printf("Port %d closed on %s\n", ports[i], ip);
            }
        } else {
            printf("No response from %s, Port %d\n", ip, ports[i]);
        }

        close(sockfd);
    }
}

int main() {
    for (int i = 0; i < num_ips; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            scan_ip(ip_addresses[i]);
            exit(0);
        } else if (pid > 0) {

        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    while (wait(NULL) > 0);

    return 0;
}

