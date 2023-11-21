#include "Header.h"
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

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
	if (bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { //Биндим сокет по указанной структуре, то есть при запуске сервер будет считывать 
		printf("Ошибка привязки. Код ошибки: %d", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else
		printf("Сокет успешно привязан\n");

	if (listen(serv_sock, 5) != 0) { //Прослушиваем данный порт
		printf("Ошибка прослушивания. Код ошибки: %d\n", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else {
		printf("Сервер ожидает подключений...\n");
	}
	while (1) {
		sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		SOCKET client_sock = accept(serv_sock, (sockaddr*)&client_addr, &client_addr_len);
		
		if (client_sock == INVALID_SOCKET) {
			printf("Ошибка при принятии подключения. Код ошибки: %d\n", WSAGetLastError());
			closesocket(serv_sock);
			WSACleanup();
			return EXIT_FAILURE;
		}
		else {
			printf("Подключение приянто\n");
			DWORD dw_thread_id;
			HANDLE h_thread;
			ThreadArgs thread_args;
			thread_args.socket = client_sock;
			h_thread = CreateThread(NULL, 0, CommunicationClient, (LPVOID)&thread_args, 0, &dw_thread_id); // создание потока
			if (h_thread == NULL) {
				printf("Не удалось создать поток\n");
				return EXIT_FAILURE;
			}
			CloseHandle(h_thread);
		}
	}
}

DWORD WINAPI CommunicationClient(LPVOID lpParam) {
	char buffer[200];
	char* point_buffer[8];
	ThreadArgs* args = (ThreadArgs*)lpParam;
	SOCKET client_sock = args->socket;
	while (recv(client_sock, buffer, sizeof(buffer), 0) > 0) {
		char* str = strtok (buffer," ");
		int i = 1;
		while (str != NULL) {
			point_buffer[i] = (char*)malloc(sizeof(str));
			strcpy(point_buffer[i], str);
			str = strtok (NULL, " ");
			i++;
		}
		char ans_to_client[50];
		memset(ans_to_client, 0, 50);
		int isErc = Work(i, point_buffer, ans_to_client);
		if ((isErc == EXIT_SUCCESS) && (ans_to_client[0] == 0)) {
			char callback[] = "Успешно!";
			send(client_sock, callback, sizeof(callback), 0);
		}
		else if ((isErc == EXIT_SUCCESS) && (ans_to_client[0] != 0)){
			send(client_sock, ans_to_client, sizeof(ans_to_client), 0);
		}
		else {
			char callback[] = "Произошла ошибка";
			send(client_sock, callback, sizeof(callback), 0);
		}
	}
	return EXIT_SUCCESS;
}