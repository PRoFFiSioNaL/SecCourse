#include "Header.h"
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

int main() {
	setlocale(0, "Ru");
	WSADATA ws_data; //���������� ��� ��������� ������ WinSock � ������������ ������������� �������
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0) {
		printf("������ ������������� WinSock. ��� ������: %d\n", WSAGetLastError());
		return EXIT_FAILURE;
	}
	else
		printf("WinSock ��������������� �����\n");

	SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //������� ����� IPv4, �� ������ ��������� TCP, 
	if (serv_sock == INVALID_SOCKET) {
		printf("������ �������� ������. ��� ������: %d\n", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else
		printf("����� ��������������� �����\n");

	in_addr ip_to_num; //������� ��������� ������
	if (inet_pton(AF_INET, "127.0.0.1", &ip_to_num) <= 0) {
		perror("������ ��� �������������� IP-������");
		return EXIT_FAILURE;
	}
	sockaddr_in serv_addr; //������� ��������� ������ ������
	memset(&serv_addr, 0, sizeof(serv_addr));	//�������� ��� ����� ���������
	serv_addr.sin_family = AF_INET; //IPv4
	serv_addr.sin_port = htons(6379); //���� 6379
	if (bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { //������ ����� �� ��������� ���������, �� ���� ��� ������� ������ ����� ��������� 
		printf("������ ��������. ��� ������: %d", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else
		printf("����� ������� ��������\n");

	if (listen(serv_sock, 5) != 0) { //������������ ������ ����
		printf("������ �������������. ��� ������: %d\n", WSAGetLastError());
		closesocket(serv_sock);
		WSACleanup();
		return EXIT_FAILURE;
	}
	else {
		printf("������ ������� �����������...\n");
	}
	while (1) {
		sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		SOCKET client_sock = accept(serv_sock, (sockaddr*)&client_addr, &client_addr_len);
		
		if (client_sock == INVALID_SOCKET) {
			printf("������ ��� �������� �����������. ��� ������: %d\n", WSAGetLastError());
			closesocket(serv_sock);
			WSACleanup();
			return EXIT_FAILURE;
		}
		else {
			printf("����������� �������\n");
			DWORD dw_thread_id;
			HANDLE h_thread;
			ThreadArgs thread_args;
			thread_args.socket = client_sock;
			h_thread = CreateThread(NULL, 0, CommunicationClient, (LPVOID)&thread_args, 0, &dw_thread_id); // �������� ������
			if (h_thread == NULL) {
				printf("�� ������� ������� �����\n");
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
			char callback[] = "�������!";
			send(client_sock, callback, sizeof(callback), 0);
		}
		else if ((isErc == EXIT_SUCCESS) && (ans_to_client[0] != 0)){
			send(client_sock, ans_to_client, sizeof(ans_to_client), 0);
		}
		else {
			char callback[] = "��������� ������";
			send(client_sock, callback, sizeof(callback), 0);
		}
	}
	return EXIT_SUCCESS;
}