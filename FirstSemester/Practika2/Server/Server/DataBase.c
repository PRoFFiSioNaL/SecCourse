#include "Header.h"
#pragma warning(disable : 4996)

enum Command {
	SADD,
	SREM,
	SISMEMBER,
	SPUSH,
	SPOP,
	QPUSH,
	QPOP,
	HSET,
	HDEL,
	HGET,
	EROR
};

FILE* fp;
int Work(int argc, char* argv[], char* callback) {
	Set* array = NULL;
	List* node = NULL;
	List* tail = NULL;
	HashTable* row = NULL;
	char order[12];
	char name[20];
	char key[15];
	char value[30];
	if (argc < 6) { //Проверка на кол-во аргументов
		puts("Ошибка аргументов");
		return EXIT_FAILURE;
	}
	if (strcmp(argv[1], "--file") != 0) {
		perror("Ошибка флагов");
		return EXIT_FAILURE;
	}
	if ((fopen_s(&fp, argv[2], "a+")) != NULL) {
		perror("Ошибка открытия файла");
		return EXIT_FAILURE;
	}
	if (strcmp(argv[3], "--query") != 0) {
		perror("Ошибка флагов 2");
		return EXIT_FAILURE;
	}
	
	for (int i = 0; i < strlen(argv[4]); ++i) {
		order[i] = *(argv[4] + i + 1);
	}
	if (argc == 6) { //Парс входных данных
		for (int i = 0; i <= strlen(argv[5]) - 1; ++i) {
			name[i] = *(argv[5] + i);
		}
		name[strlen(argv[5]) - 2] = '\0';
	}
	else if (argc == 7) {
		for (int i = 0; i <= strlen(argv[5]); ++i) {
			name[i] = *(argv[5] + i);
		}
		for (int i = 0; i < strlen(argv[6]) - 1; ++i) {
			value[i] = *(argv[6] + i);
		}
		value[strlen(argv[6]) - 2] = '\0';
	}
	else if (argc == 8) {
		for (int i = 0; i <= strlen(argv[5]); ++i) {
			name[i] = *(argv[5] + i);
		}
		for (int i = 0; i <= strlen(argv[6]); ++i) {
			key[i] = *(argv[6] + i);
		}
		for (int i = 0; i < strlen(argv[7]) - 1; ++i) {
			value[i] = *(argv[7] + i);
		}
		value[strlen(argv[7]) - 2] = '\0';
	}
	enum Command command = EROR;
	if (strcmp(order, "SADD") == 0) command = SADD;
	else if (strcmp(order, "SREM") == 0) command = SREM;
	else if (strcmp(order, "SISMEMBER") == 0) command = SISMEMBER;
	else if (strcmp(order, "QPUSH") == 0) command = QPUSH;
	else if (strcmp(order, "QPOP") == 0) command = QPOP;
	else if (strcmp(order, "SPUSH") == 0) command = SPUSH;
	else if (strcmp(order, "SPOP") == 0) command = SPOP;
	else if (strcmp(order, "HSET") == 0) command = HSET;
	else if (strcmp(order, "HDEL") == 0) command = HDEL;
	else if (strcmp(order, "HGET") == 0) command = HGET;
	switch (command)
	{
	case SADD:
		SetReadFromFile(&array, name);
		SetAdd(array, name, value, callback);
		break;
	case SREM:
		SetReadFromFile(&array, name);
		SetRem(array, name, value, argv[2], callback);
		break;
	case SISMEMBER:
		SetReadFromFile(&array, name);
		SetIsMember(array, value, callback);
		break;
	case SPUSH:
		StackReadFromFile(&node, &tail, name);
		StackPush(node, tail, name, value);
		break;
	case SPOP:
		StackReadFromFile(&node, &tail, name);
		StackPop(node, name, argv[2], callback);
		break;
	case QPUSH:
		QueueReadFromFile(&node, &tail, name);
		QueuePush(node, tail, name, value);
		break;
	case QPOP:
		QueueReadFromFile(&node, &tail, name);
		QueuePop(node, name, argv[2], callback);
		break;
	case HSET:
		HashReadFromFile(&row);
		HashSet(row, key, value, argv[2]);
		break;
	case HDEL:
		HashReadFromFile(&row);
		HashDelete(row, value, argv[2]);
		break;
	case HGET:
		HashReadFromFile(&row);
		HashGet(row, value, argv[2], callback);
		break;
	default:
		fclose(fp);
		return EXIT_FAILURE;
	}
	fclose(fp);
	return EXIT_SUCCESS;
}

void PushToHeadList(List** node, List** tail, char* value) {
	List* tmp = (List*)malloc(sizeof(List));
	strcpy_s(tmp->value, 30 ,value);
	if (*node == NULL) {
		*tail = tmp;
		*node = tmp;
		tmp->next = NULL;
	}
	else {
		tmp->next = *node;
		*node = tmp;
	}
	
}

void PushToTailList(List** node, List** tail, char* value) {
	List* tmp = (List*)malloc(sizeof(List));
	strcpy_s(tmp->value, 30 ,value);
	tmp->next = NULL;
	if (*node == NULL) {
		*tail = tmp;
		*node = tmp;
	}
	else {
		(*tail)->next = tmp;
		(*tail) = tmp;
	}
}

char* PopFromHead(List** node) {
	if (*node == NULL) {
		char value[30] = "Список пуст";
		return value;
	}
	List* tmp = *node;
	char value[30];
	strcpy_s(value, 30, tmp->value);
	*node = tmp->next;
	free(tmp);
	return value;
}

int WriteListToFile(char* name, char* value) {
	char str[50];
	strcat_s(value, 30, "\n");
	СoncatenationStr(name, value, str);
	if (fwrite(str, 1, strlen(str), fp) != strlen(str)) {
		perror("Ошибка записи в файл");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int WriteHashToFile(char* key, char* value, char is_exist) {
	char str[50];
	СoncatenationStr(key, value, str);
	str[strlen(str) + 3] = '\0';
	str[strlen(str) + 2] = '\n';
	str[strlen(str) + 1] = is_exist;
	str[strlen(str)] = ' ';
	if (fwrite(str, 1, strlen(str), fp) != strlen(str)) {
		perror("Ошибка записи в файл");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int DeleteStrFromFile(char* str, char* path) {
	char buff[50];
	char change_buff[50];
	char is_del = '0';
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	char* data = (char*)calloc(size, sizeof(char));
	fseek(fp, 0, SEEK_SET);
	while (fgets(buff, 50, fp)) {
		strcpy_s(change_buff, 50, buff);
		change_buff[strlen(change_buff) - 1] = '\0';
		if ((strcmp(change_buff, str) != 0) || (is_del == '1')) {
			strcat_s(data, size, buff);
		}
		else is_del = '1';
	}
	fclose(fp);
	if ((fopen_s(&fp, path, "w")) != NULL) {
		perror("Ошибка открытия файла");
		return EXIT_FAILURE;
	}
	fputs (data, fp);
	free(data);
	return EXIT_SUCCESS;
}

int ReplaceStrInFile(char* str, char* new_str, char* path) {
	char buff[50];
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp) + 50;
	char* data = (char*)calloc(size, sizeof(char));
	fseek(fp, 0, SEEK_SET);
	while (fgets(buff, 50, fp)) {
		if (strcmp(buff, str) != 0) {
			strcat_s(data, size, buff);
		}
		else {
			strcat_s(data, size, new_str);
		}
	}
	fclose(fp);
	if ((fopen_s(&fp, path, "w")) != NULL) {
		perror("Ошибка открытия файла");
		return EXIT_FAILURE;
	}
	fputs (data, fp);
	free(data);
	return EXIT_SUCCESS;
}

void СoncatenationStr(char* name, char* value, char* str) {
	strcpy_s(str, 50, name);
	str[strlen(str) + 1] = '\0';
	str[strlen(str)] = ' ';
	strcat_s(str, 50, value);
}

void СoncatenationStrForHash(char* key, char* value, char is_exist, char* str) {
	СoncatenationStr(key, value, str);
	str[strlen(str) + 3] = '\0';
	str[strlen(str) + 2] = '\n';
	str[strlen(str) + 1] = is_exist;
	str[strlen(str)] = ' ';
}

void SetAdd(Set* array, char* name, char* value, char* callback) {
	int index = FuncHash(value);
	char is_coincidence = '0';
	while (array[index].value[0] != '\0') {
		if (strcmp(array[index].value, value) == 0) {
			is_coincidence = '1';
			break;
		}
		index = (index + FuncHashSec(value[0])) % 100;
	}
	if (is_coincidence == '0') WriteListToFile(name, value);
	else strcpy(callback, "Данный элемент уже есть в множестве");
}

void SetRem(Set* array, char* name, char* value, char* path, char* callback) {
	char str[50];
	int index = FuncHash(value);
	char is_coincidence = '0';
	while (array[index].value[0] != '\0') {
		if (strcmp(array[index].value, value) == 0) {
			is_coincidence = '1';
			СoncatenationStr(name, value, str);
			break;
		}
		index = (index + FuncHashSec(value[0])) % 100;
	}
	if (is_coincidence == '1') DeleteStrFromFile(str, path);
	else strcpy(callback, "Такого элемента нет в множестве");
}

void SetIsMember(Set* array, char* value, char* callback) {
	int index = FuncHash(value);
	char is_coincidence = '0';
	while (array[index].value[0] != '\0') {
		if (strcmp(array[index].value, value) == 0) {
			is_coincidence = '1';
			break;
		}
		index = (index + FuncHashSec(value[0])) % 100;
	}
	if (is_coincidence == '1') strcpy(callback, "Такой элемент есть в множестве");
	else strcpy(callback, "Такого элемента нет в множестве");
}

void SetReadFromFile(Set** array, char* main_name) {
	*array = (Set*)calloc(100, sizeof(Set));
	char name[20];
	char value[30];
	while (!feof(fp)) {
		fscanf_s(fp, "%s %s\n", name, (unsigned)sizeof(name), value, (unsigned)sizeof(value));
		if (strcmp(main_name, name) == 0) {
			int index = FuncHash(value);
			while ((*array)[index].value[0] != '\0') {
				index = (index + FuncHashSec(value[0])) % 100;
			}
			strcpy_s((*array)[index].value, 30, value);
		}
	}
}

void StackPush(List* node, List* tail, char* name, char* value) {
	PushToHeadList(&node, &tail, value); //Добавляю в голову - становится первым элементом
	WriteListToFile(name, value);
}

void StackPop(List* node, char* name, char* path, char* callback) {
	char value[30];
	char str[50];
	strcpy_s(value, 30, PopFromHead(&node)); //Удаляю голову - первый элемент
	strcpy(callback, value);
	СoncatenationStr(name, value, str);
	DeleteStrFromFile(str, path);
}

void StackReadFromFile(List** node, List** tail, char* name) {
	char text_name[20];
	char text_value[30];
	while (!feof(fp)) {
		fscanf_s(fp, "%s %s\n", text_name, (unsigned)sizeof(text_name), text_value, (unsigned)sizeof(text_value));
		if (strcmp(text_name, name) == 0) {
			PushToHeadList(node, tail, text_value); //Первый элмент в файле - хвост
		} 
	}
}

void QueuePush(List* node, List* tail, char* name, char* value) {
	PushToTailList(&node, &tail, value); //Ложу в хвост - становится последним элементом
	WriteListToFile(name, value);
}

void QueuePop(List* node, char* name, char* path, char* callback) {
	char value[30];
	char str[50];
	strcpy_s(value, 30, PopFromHead(&node)); //Удаляю голову - первый элемент
	strcpy(callback, value);
	СoncatenationStr(name, value, str);
	DeleteStrFromFile(str, path);
}

void QueueReadFromFile(List** node, List** tail, char* name) {
	char text_name[20];
	char text_value[30];
	while (!feof(fp)) {
		fscanf_s(fp, "%s %s\n", text_name, (unsigned)sizeof(text_name), text_value, (unsigned)sizeof(text_value));
		if (strcmp(text_name, name) == 0) {
			PushToTailList(node, tail, text_value); //Первый элемент в файле - голова
		} 
	}
}

void HashSet(HashTable* row, char* key, char* value, char* path) {
	int index = FuncHash(key);
	char is_coincidence = '0';
	char is_del = '0';
	int index_del;
	char str[50];
	char new_str[50];
	while (row[index].value[0] != '\0') {
		if (row[index].is_exist == '0') {
			if (is_del == '0') {
				index_del = index;
				is_del = '1';
			}
			else break;
		}
		if (strcmp(row[index].key, key) == 0) {
			is_coincidence = '1';
			break;
		}
		index = (index + FuncHashSec(key[0])) % 100;
	}
	if (is_coincidence == '1') {
		СoncatenationStrForHash(row[index].key, row[index].value, row[index].is_exist, str);
		strcpy_s(row[index].key, 15, key);
		strcpy_s(row[index].value, 30, value);
		row[index].is_exist = '1';
		СoncatenationStrForHash(row[index].key, row[index].value, row[index].is_exist, new_str);
		ReplaceStrInFile(str, new_str, path);
	}
	else if (is_del == '1') {
		СoncatenationStrForHash(row[index_del].key, row[index_del].value, row[index_del].is_exist, str);
		strcpy_s(row[index_del].key, 15, key);
		strcpy_s(row[index_del].value, 30, value);
		row[index_del].is_exist = '1';
		СoncatenationStrForHash(row[index_del].key, row[index_del].value, row[index_del].is_exist, new_str);
		ReplaceStrInFile(str, new_str, path);
	}
	else {
		СoncatenationStrForHash(row[index].key, row[index].value, row[index].is_exist, str);
		strcpy_s(row[index].key, 15, key);
		strcpy_s(row[index].value, 30, value);
		row[index].is_exist = '1';
		СoncatenationStrForHash(row[index].key, row[index].value, row[index].is_exist, new_str);
		WriteHashToFile(row[index].key, row[index].value, row[index].is_exist);	
	}
}

void HashDelete(HashTable* row, char* key, char* path) {
	int index = FuncHash(key);
	char str[50];
	char new_str[50];
	char is_coincidence = '0';
	while (row[index].value[0] != '\0') {
		if (strcmp(row[index].key, key) == 0) {
			is_coincidence = '1';
			break;
		}
		index = (index + FuncHashSec(key[0])) % 100;
	}
	if (is_coincidence == '1') {
		СoncatenationStrForHash(row[index].key, row[index].value, '1', str);
		СoncatenationStrForHash(row[index].key, row[index].value, '0', new_str);
		ReplaceStrInFile(str, new_str, path);
	}
}

void HashGet(HashTable* row, char* key, char* path, char* callback) {
	int index = FuncHash(key);
	char is_coincidence = '0';
	while (row[index].value[0] != '\0') {
		if ((strcmp(row[index].key, key) == 0) && (row[index].is_exist == '1')) {
			is_coincidence = '1';
			break;
		}
		index = (index + FuncHashSec(key[0])) % 100;
	}
	if (is_coincidence == '1') strcpy(callback, row[index].value);
	else strcpy(callback, "Значение по такому ключу не найдено");
}

void HashReadFromFile(HashTable** row) {
	*row = (HashTable*)calloc(100, sizeof(HashTable));
	char key[15];
	char value[30];
	char is_exist = '\0';
	while (!feof(fp)) {
		fscanf_s(fp, "%s %s %c\n", key, (unsigned)sizeof(key), value, (unsigned)sizeof(value), &is_exist, (unsigned)sizeof(is_exist));
		int index = FuncHash(key);
		while ((*row)[index].value[0] != '\0') {
			index = (index + FuncHashSec(key[0])) % 100;
		}
		strcpy_s((*row)[index].key, 15, key);
		strcpy_s((*row)[index].value, 30, value);
		(*row)[index].is_exist = is_exist;
	}
}

int FuncHash(char* key) {
	int sum = 0;
	for (int i = 0; key[i]; ++i)
		sum += key[i];
	return sum % 100;
}

int FuncHashSec(char ch) {
	int sum = ch * 37;
	return sum % 100;
}