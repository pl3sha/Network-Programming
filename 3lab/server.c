#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Глобальные переменные
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *shared_file;

// Функция для обработки клиента
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Получаем адрес клиента
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    if (getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len) == -1) {
        perror("Getpeername failed");
        close(client_socket);
        free(arg);
        pthread_exit(NULL);
    }

    char client_info[INET_ADDRSTRLEN + 10]; // Буфер для хранения информации о клиенте
    snprintf(client_info, sizeof(client_info), "Client [%s:%d]: ",
             inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        // Блокируем доступ к файлу для записи
        pthread_mutex_lock(&file_mutex);

        // Записываем данные в файл с указанием клиента
        fprintf(shared_file, "%s%s", client_info, buffer);
        fflush(shared_file);

        // Разблокируем доступ к файлу
        pthread_mutex_unlock(&file_mutex);

        // Выводим данные на экран
        printf("%s%s", client_info, buffer);
    }

    close(client_socket);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Открываем общий файл для записи
    shared_file = fopen("data.txt", "a");
    if (!shared_file) {
        perror("Failed to open shared file");
        exit(EXIT_FAILURE);
    }

    // Создаем сокет
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0; // Нулевой порт означает, что система выберет свободный порт

    // Привязываем сокет к адресу
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Получаем номер порта, назначенный системой
    socklen_t addr_len = sizeof(server_addr);
    if (getsockname(server_socket, (struct sockaddr *)&server_addr, &addr_len) == -1) {
        perror("Getsockname failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port: %d\n", ntohs(server_addr.sin_port));

    // Начинаем прослушивание
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // Принимаем подключение клиента
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Выделяем память для сокета клиента
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        // Создаем новый поток для обработки клиента
        if (pthread_create(&thread_id, NULL, handle_client, new_sock) != 0) {
            perror("Thread creation failed");
            free(new_sock);
            close(client_socket);
        }

        // Отсоединяем поток, чтобы освободить ресурсы
        pthread_detach(thread_id);
    }

    fclose(shared_file);
    close(server_socket);
    return 0;
}