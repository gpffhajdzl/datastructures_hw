#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_YEAR_DURATION   10   // 기간
#define SORT_BY_NAME   0 // 이름 순 정렬
#define SORT_BY_FREQ   1 // 빈도 순 정렬

// 구조체 선언
// 이름 구조체
typedef struct {
	char   name[20];      // 이름
	char   sex;         // 성별 'M' or 'F'
	int      total_freq;      // 연도별 빈도 합
	int      freq[MAX_YEAR_DURATION]; // 연도별 빈도 memset으로 해결해도됨!! 교수님 HINT!
} tName;

// 헤더 구조체
typedef struct {
	int      len;      // 배열에 저장된 이름의 수
	int      capacity;   // 배열의 용량 (배열에 저장 가능한 이름의 수)
	tName* data;      // 이름 구조체 배열의 포인터
} tNames;

////////////////////////////////////////////////////////////////////////////////
// 함수 원형 선언(declaration)

// 이름 순으로 정렬된 입력 파일을 읽어 이름 정보(이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// names->capacity는 1000으로부터 시작하여 1000씩 증가 (1000, 2000, 3000, ...)
// start_year : 시작 연도 (2009)
void load_names(FILE* fp, int start_year, tNames* names);

// 구조체 배열을 화면에 출력 (이름, 성별, 빈도 합, 연도별 빈도)
void print_names(tNames* names, int num_year);

// qsort를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare_by_name(const void* n1, const void* n2);

// 정렬 기준 : 빈도 내림차순(1순위), 이름(2순위), 성별(3순위)
int compare_by_freq(const void* n1, const void* n2);

// 내가 임의로 만든 함수들
void if_expand_names(tNames* names);
void set_new_name(tName* set_name, char* name, char sex);
void update_name(tName* name, int freq, int year, int start_year);

////////////////////////////////////////////////////////////////////////////////
// 함수 정의 (definition)

// 이름 구조체를 초기화
// len를 0으로, capacity를 1000으로 초기화
// return : 구조체 포인터
tNames* create_names(void)
{
	tNames* pnames = (tNames*)malloc(sizeof(tNames));

	pnames->len = 0;
	pnames->capacity = 1000;
	pnames->data = (tName*)malloc(pnames->capacity * sizeof(tName));

	return pnames;
}

// 이름 구조체에 할당된 메모리를 해제
void destroy_names(tNames* pnames)
{
	free(pnames->data);
	pnames->len = 0;
	pnames->capacity = 0;

	free(pnames);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	tNames* names;
	int option;
	FILE* fp;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s option FILE\n\n", argv[0]);
		fprintf(stderr, "option\n\t-n\t\tsort by name\n\t-f\t\tsort by frequency\n");
		return 1;
	}

	if (strcmp(argv[1], "-n") == 0) option = SORT_BY_NAME;
	else if (strcmp(argv[1], "-f") == 0) option = SORT_BY_FREQ;
	else {
		fprintf(stderr, "unknown option : %s\n", argv[1]);
		return 1;
	}

	// 이름 구조체 초기화
	names = create_names();

	if ((fp = fopen(argv[2], "r")) == NULL)
	{
		fprintf(stderr, "cannot open file : %s\n", argv[2]);
		return 1;
	}

	// 입력 파일(이름 정보)을 구조체에 저장
	load_names(fp, 2009, names);

	fclose(fp);

	if (option == SORT_BY_NAME) {
		// 정렬 (이름순 (이름이 같은 경우 성별순))
		qsort(names->data, names->len, sizeof(tName), compare_by_name);
	}
	else { // SORT_BY_FREQ
	   // 정렬 (빈도순 (빈도가 같은 경우 이름순))
		qsort(names->data, names->len, sizeof(tName), compare_by_freq);
	}

	// 이름 구조체를 화면에 출력
	print_names(names, MAX_YEAR_DURATION);

	// 이름 구조체 해제
	destroy_names(names);

	return 0;
}

// ----------------------------------과제에서 물어본 함수들------------------------------------------

void load_names(FILE* fp, int start_year, tNames* names) {
	int year, freq;
	char sex, name[20];
	tName* now_name = &(names->data[names->len]);
	int break_flag = 0;


	while (1) {

		break_flag = fscanf(fp, "%d %s %c %d", &year, name, &sex, &freq);
		//자료가 잘 들어왔는지 확인
		//printf("%d %s %c %d\n", year, name, sex, freq);
		if (break_flag != 4) break;
		if ((strcmp(now_name->name, name) != 0) || (now_name->sex != sex)) {
			if_expand_names(names);

			names->len++;
			now_name = &(names->data[names->len - 1]);

			set_new_name(now_name, name, sex);

		}

		update_name(now_name, freq, year, start_year);

	}
}

// name 기준 비교함수
int compare_by_name(const void* n1, const void* n2) {
	tName* first = (tName*)n1;
	tName* second = (tName*)n2;

	return strcmp(first->name, second->name) ? (strcmp(first->name, second->name)) : (first->sex - second->sex);
}

// freq 기준 비교함수
int compare_by_freq(const void* n1, const void* n2) {
	tName* first = (tName*)n1;
	tName* second = (tName*)n2;

	if (second->total_freq - first->total_freq) {
		return (second->total_freq - first->total_freq);
	}
	else if (strcmp(first->name, second->name)) {
		return (strcmp(first->name, second->name));
	}
	else {
		return (first->sex - second->sex);
	}
}

// names 출력함수
void print_names(tNames* names, int num_year) {

	for (int i = 0; i < names->len; i++) {
		tName* n = &(names->data[i]);
		printf("%s\t%c\t%d\t", n->name, n->sex, n->total_freq);
		for (int j = 0; j < num_year - 1; j++) {
			printf("%d\t", n->freq[j]);
		}
		printf("%d\n", n->freq[num_year - 1]);
	}
}

// ----------------------------------내가 임의로 만든 함수들----------------------------------------------
void if_expand_names(tNames* names) {
	if (names->len == names->capacity) {
		names->capacity += 1000;
		names->data = realloc(names->data, sizeof(tName) * names->capacity);
	}
}

void set_new_name(tName* set_name, char* name, char sex) {
	strcpy(set_name->name, name);
	set_name->sex = sex;
	set_name->total_freq = 0;
	memset(set_name->freq, 0, sizeof(int) * MAX_YEAR_DURATION);
}

void update_name(tName* name, int freq, int year, int start_year) {
	name->total_freq += freq;
	name->freq[year - start_year] = freq;
}