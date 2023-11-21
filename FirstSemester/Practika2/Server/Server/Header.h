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

typedef struct Set { //Множество
	char value[30];
} Set;

typedef struct List { //Стек и очередь
	char value[30];
	struct List* next;
} List;

typedef struct HashTable { //Хеш-таблица с методом открытых адресаций
	char key[15];
	char value[30];
	char is_exist;
} HashTable;

void PushToHeadList(List** node, List** tail, char* value); //Добавить в начало списка
void PushToTailList(List** node, List** tail, char* value); //Добавить в конец списка
char* PopFromHead(List** node); //Удалить из начала списка
int WriteListToFile(char* name, char* value); //Запись листа в файл
int WriteHashToFile(char* key, char* value, char is_exist);
int DeleteStrFromFile(char* str, char* path); //Удаление строки из файла
int ReplaceStrInFile(char* str, char* new_str, char* path); //Замена строки в файле
void СoncatenationStr(char* name, char* value, char* str); //Конкатенация строк
void СoncatenationStrForHash(char* key, char* value, char is_exist, char* str); //Конкатенация строк для хеш-таблицы
void SetAdd(Set* array, char* name, char* value, char* callback);
void SetRem(Set* array, char* name, char* value, char* path, char* callback);
void SetIsMember(Set* array, char* value, char* callback);
void SetReadFromFile(Set** array, char* main_name); //Чтение множества из файла(заполнение структуры)
void StackPush(List* node, List* tail, char* name, char* value);
void StackPop(List* node, char* name, char* path, char* callback);
void StackReadFromFile(List** node, List** tail, char* name); //Чтение стека
void QueuePush(List* node, List* tail, char* name, char* value);
void QueuePop(List* node, char* name, char* path, char* callback);
void QueueReadFromFile(List** node, List** tail, char* name); //Чтение очереди
void HashSet(HashTable* row, char* key, char* value, char* path);
void HashDelete(HashTable* row, char* key, char* path);
void HashGet(HashTable* row, char* key, char* path, char* callback);
void HashReadFromFile(HashTable** row); //Чтение хеш-таблицы
int FuncHash(char* key); //Первая хеш-функция
int FuncHashSec(char ch); //Вторая хеш-функция
int Work(int argc, char* argv[], char* callback);
DWORD WINAPI CommunicationClient(LPVOID lpParam);