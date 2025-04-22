#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, max_sd, activity, i, valread;
    struct sockaddr_in address;
    int opt = 1;
    int client_sockets[MAX_CLIENTS] = {0}; // Массив для хранения сокетов клиентов
    int client_ids[MAX_CLIENTS] = {0};    // Массив для хранения ID клиентов
    fd_set readfds;
    char buffer[BUFFER_SIZE] = {0};

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка опций сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(0); // Порт выбирается автоматически

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Получение номера порта
    socklen_t len = sizeof(address);
    getsockname(server_fd, (struct sockaddr *)&address, &len);
    printf("Server is running on port: %d\n", ntohs(address.sin_port));

    // Прослушивание входящих подключений
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    puts("Waiting for connections...");

    int next_client_id = 1; // Счетчик для назначения уникальных ID клиентам

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Добавление клиентских сокетов в набор
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        // Ожидание активности на одном из сокетов
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
        }

        // Если активность на серверном сокете
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&len)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            // Назначаем новый ID клиенту
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    client_ids[i] = next_client_id++;
                    break;
                }
            }

            printf("New connection, client ID: %d, ip: %s, port: %d\n",
                   client_ids[i], inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        }

        // Проверка активности клиентских сокетов
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds)) {
                if ((valread = read(client_sockets[i], buffer, BUFFER_SIZE)) == 0) {
                    // Клиент отключился
                    getpeername(client_sockets[i], (struct sockaddr *)&address, (socklen_t *)&len);
                    printf("Client ID %d disconnected, ip: %s, port: %d\n",
                           client_ids[i], inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                    client_ids[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("Message from client ID %d: %s", client_ids[i], buffer);
                }
            }
        }
    }

    return 0;
}