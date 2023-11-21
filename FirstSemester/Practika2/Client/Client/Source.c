#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <locale.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
	setlocale(0, "Ru");
	WSADATA ws_data; //Необходимо для установки версии WinSock и последующего использования скоетов
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0) {
		printf("Ошибка инициализации WinSock. Код ошибки: %d\n", WSAGetLastError());
		return EXIT_FAILURE;
	}
	else
		printf("WinSock инициализирован верно\n");

	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Создаем сокет IPv4, на основе протокола TCP, 
	if (serv_sock == INVALID_SOCKET) {
		printf("Ошибка создания сокета. Код ошибки: %d\n", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else
		printf("Сокет инициализирован верно\n");

	in_addr ip_to_num; //Создаем структуру адреса
	if (inet_pton(AF_INET, "127.0.0.1", &ip_to_num) <= 0) {
		perror("Ошибка при преобразовании IP-адреса");
		return EXIT_FAILURE;
	}
	sockaddr_in serv_addr; //Создаем структуру адреса сокета
	memset(&serv_addr, 0, sizeof(serv_addr));	//Обнуляем все байты структуры
	serv_addr.sin_family = AF_INET; //IPv4
	serv_addr.sin_port = htons(6379); //Порт 6379
	serv_addr.sin_addr = ip_to_num; //127.0.0.1
	if (connect(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		printf("Ошибка подключения к сокету. Код ошибки: %d\n", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	char buffer[200];
	do {
		printf("Введите команду: ");
		fgets(buffer, sizeof(buffer), stdin);
		send(serv_sock, buffer, sizeof(buffer), 0);

		recv(serv_sock, buffer, sizeof(buffer), 0);
		printf("%s\n", buffer);
	} while (GetKeyState(VK_ESCAPE) >= 0);
	closesocket(serv_sock);
	WSACleanup();
	return EXIT_SUCCESS;
}