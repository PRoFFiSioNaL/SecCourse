#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string.h>
#include <locale.h>
#include <fcntl.h>

typedef struct ThreadArgs {
	SOCKET socket;
} ThreadArgs;

typedef struct Set { //���������
	char value[30];
} Set;

typedef struct List { //���� � �������
	char value[30];
	struct List* next;
} List;

typedef struct HashTable { //���-������� � ������� �������� ���������
	char key[15];
	char value[30];
	char is_exist;
} HashTable;

void PushToHeadList(List** node, List** tail, char* value); //�������� � ������ ������
void PushToTailList(List** node, List** tail, char* value); //�������� � ����� ������
char* PopFromHead(List** node); //������� �� ������ ������
int WriteListToFile(char* name, char* value); //������ ����� � ����
int WriteHashToFile(char* key, char* value, char is_exist);
int DeleteStrFromFile(char* str, char* path); //�������� ������ �� �����
int ReplaceStrInFile(char* str, char* new_str, char* path); //������ ������ � �����
void �oncatenationStr(char* name, char* value, char* str); //������������ �����
void �oncatenationStrForHash(char* key, char* value, char is_exist, char* str); //������������ ����� ��� ���-�������
void SetAdd(Set* array, char* name, char* value, char* callback);
void SetRem(Set* array, char* name, char* value, char* path, char* callback);
void SetIsMember(Set* array, char* value, char* callback);
void SetReadFromFile(Set** array, char* main_name); //������ ��������� �� �����(���������� ���������)
void StackPush(List* node, List* tail, char* name, char* value);
void StackPop(List* node, char* name, char* path, char* callback);
void StackReadFromFile(List** node, List** tail, char* name); //������ �����
void QueuePush(List* node, List* tail, char* name, char* value);
void QueuePop(List* node, char* name, char* path, char* callback);
void QueueReadFromFile(List** node, List** tail, char* name); //������ �������
void HashSet(HashTable* row, char* key, char* value, char* path);
void HashDelete(HashTable* row, char* key, char* path);
void HashGet(HashTable* row, char* key, char* path, char* callback);
void HashReadFromFile(HashTable** row); //������ ���-�������
int FuncHash(char* key); //������ ���-�������
int FuncHashSec(char ch); //������ ���-�������
int Work(int argc, char* argv[], char* callback);
DWORD WINAPI CommunicationClient(LPVOID lpParam);