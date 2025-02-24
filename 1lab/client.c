#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <server_port> <iterations>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int iterations = atoi(argv[3]);

    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return EXIT_FAILURE;
    }

    for (int i = 1; i <= iterations; i++) {
        snprintf(buffer, BUF_SIZE, "%d", i);
        printf("Sending to server: %s\n", buffer);

        if (sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Sendto failed");
            close(sockfd);
            return EXIT_FAILURE;
        }

        socklen_t len = sizeof(serv_addr);
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&serv_addr, &len);
        if (bytes_received < 0) {
            perror("Recvfrom failed");
            continue;
        }

        buffer[bytes_received] = '\0';
        printf("Received from server: %s\n", buffer);

        sleep(i);
    }

    close(sockfd);
    return 0;
}