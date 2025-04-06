#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 5
#define BUFFER_SIZE 1024

void signal_chld(int sig) {
    (void)sig; 
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int socket_server, socket_client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка: не удалось создать сокет");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(0); 

    if (bind(socket_server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка: не удалось привязать сокет");
        close(socket_server);
        exit(1);
    }

    socklen_t addr_len = sizeof(server_addr);
    if (getsockname(socket_server, (struct sockaddr*)&server_addr, &addr_len) == -1) {
        perror("Ошибка: не удалось получить имя сокета");
        close(socket_server);
        exit(1);
    }

    printf("Сервер запущен на порту: %d\n", ntohs(server_addr.sin_port));

    if (listen(socket_server, BACKLOG) == -1) {
        perror("Ошибка: не удалось запустить прослушивание");
        close(socket_server);
        exit(1);
    }

    signal(SIGCHLD, signal_chld);

    while (1) {
        socket_client = accept(socket_server, (struct sockaddr*)&client_addr, &client_len);
        if (socket_client == -1) {
            perror("Ошибка: не удалось принять соединение");
            continue;
        }

        if (fork() == 0) {
            char buffer[BUFFER_SIZE];

            close(socket_server);

            while (1) {
                memset(buffer, 0, BUFFER_SIZE);

                int received = recv(socket_client, buffer, BUFFER_SIZE, 0);
                if (received <= 0) break;

                printf("Получено от (IP: %s, Порт: %d) - %s\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
            }

            close(socket_client);
            exit(0);
        }

        close(socket_client);
    }

    close(socket_server);
    return 0;
}