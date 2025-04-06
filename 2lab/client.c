#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Ошибка: неверное количество аргументов\n");
        fprintf(stderr, "Использование: %s <IP> <порт>\n", argv[0]);
        return -1;
    }

    int socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_client == -1) {
        perror("Ошибка: не удалось создать сокет");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Ошибка: некорректный IP-адрес");
        close(socket_client);
        return -1;
    }

    if (connect(socket_client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка: не удалось подключиться к серверу");
        close(socket_client);
        return -1;
    }

    int number, count_iter;

    printf("Введите число для отправки: ");
    scanf("%d", &number);
    printf("Введите количество итераций: ");
    scanf("%d", &count_iter);

    for (int i = 1; i <= count_iter; i++) {
        char message[32];
        snprintf(message, sizeof(message), "%d", number);

        if (send(socket_client, message, strlen(message), 0) == -1) {
            perror("Ошибка: не удалось отправить данные");
            close(socket_client);
            return -1;
        }

        printf("%d) Отправлено: %s\n", i, message);

        sleep(i); 
    }

    close(socket_client);
    return 0;
}