#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addr_len = sizeof(cli_addr);
    char buffer[BUF_SIZE];
    int port;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0; 

    if (bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sockfd, (struct sockaddr *)&serv_addr, &len) == -1) {
        perror("Getsockname failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    port = ntohs(serv_addr.sin_port);
    printf("Server started on port: %d\n", port);

    while (1) {
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &addr_len);
        if (bytes_received < 0) {
            perror("Recvfrom failed");
            continue;
        }

        buffer[bytes_received] = '\0';
        int number = atoi(buffer);

        printf("Received from client: %s\n", buffer);
        printf("Client IP: %s, Port: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        int result = number * 2;
        snprintf(buffer, BUF_SIZE, "%d", result);

        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&cli_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
