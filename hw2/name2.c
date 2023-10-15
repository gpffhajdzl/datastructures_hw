#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_YEAR_DURATION   10   // 기간
#define SORT_BY_NAME   0 // 이름 순 정렬
#define SORT_BY_FREQ   1 // 빈도 순 정렬

// 구조체 선언
typedef struct {
    char   name[20];      // 이름
    char   sex;         // 성별 M or F
    int      total_freq;      // 연도별 빈도 합
    int      freq[MAX_YEAR_DURATION]; // 연도별 빈도
} tName;

typedef struct {
    int      len;      // 배열에 저장된 이름의 수
    int      capacity;   // 배열의 용량 (배열에 저장 가능한 이름의 수)
    tName* data;      // 이름 배열의 포인터
} tNames;

////////////////////////////////////////////////////////////////////////////////
// 함수 원형 선언

// 입력 파일을 읽어 이름 정보(연도, 이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// 주의사항: 이름과 성별에 의해 정렬 리스트(ordered list)를 유지해야 함 (qsort 함수 사용하지 않음)
// 1. 이미 등장한 이름인지 검사하기 위해
// 2. 새로운 이름을 삽입할 위치를 찾기 위해 binary_search 함수를 사용
// 새로운 이름을 저장할 메모리 공간을 확보하기 위해 memmove 함수를 이용하여 메모리에 저장된 내용을 복사
// names->capacity는 1000으로부터 시작하여 1000씩 증가 (1000, 2000, 3000, ...)
// start_year : 시작 연도 (2009)
void load_names(FILE* fp, int start_year, tNames* names);

// 구조체 배열을 화면에 출력
void print_names(tNames* names, int num_year);

// binary_search, qsort를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare_by_name(const void* n1, const void* n2);

// 정렬 기준 : 빈도 내림차순(1순위), 이름(2순위), 성별(3순위)
int compare_by_freq(const void* n1, const void* n2);

// 이진탐색 함수
// found : key가 발견되는 경우 1, key가 발견되지 않는 경우 0
// return value: key가 발견되는 경우, 배열의 인덱스
//            key가 발견되지 않는 경우, key가 삽입되어야 할 배열의 인덱스
int binary_search(const void* key, const void* base, size_t nmemb, size_t size,
    int (*compare)(const void*, const void*), int* found);

////////////////////////////////////////////////////////////////////////////////
// 함수 정의

// 이름 구조체 초기화
// len를 0으로, capacity를 1000로 초기화
// return : 구조체 포인터
tNames* create_names(void)
{
    tNames* pnames = (tNames*)malloc(sizeof(tNames));

    pnames->len = 0;
    pnames->capacity = 1000;
    pnames->data = (tName*)malloc(pnames->capacity * sizeof(tName));

    //할당된 메모리 초기화
    //pnames->data->name;
    //pnames->data->sex = 0;
    //pnames->data->total_freq = 0;
    for (int i = 0; i < MAX_YEAR_DURATION; i++) {
        pnames->data->freq[i] = 0;
    } // --> 이거 안해주면 Conditional jump or move depends on uninitialised value(s)랑 Use of uninitialised value of size 어쩌구 오류남
    return pnames;
}

// 이름 구조체에 할당된 메모리를 해제
void destroy_names(tNames* pnames)
{
    //printf("\n\n해제하는 메모리의 크기 : %lu\n\n", sizeof(pnames->data));
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

    // 연도별 입력 파일(이름 정보)을 구조체에 저장
    load_names(fp, 2009, names);

    fclose(fp);

    if (option == SORT_BY_NAME) {
        // 정렬 (이름순 (이름이 같은 경우 성별순))
        qsort( names->data, names->len, sizeof(tName), compare_by_name);
    }
    else { // SORT_BY_FREQ
        // 정렬 (빈도순 (빈도가 같은 경우 이름순))
        qsort( names->data, names->len, sizeof(tName), compare_by_freq);
    }

    // 이름 구조체를 화면에 출력
    print_names(names, MAX_YEAR_DURATION);

    // 이름 구조체 해제
    destroy_names(names);

    return 0;
}

// 입력 파일을 읽어 이름 정보(연도, 이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// 주의사항: 이름과 성별에 의해 정렬 리스트(ordered list)를 유지해야 함 (qsort 함수 사용하지 않음)
// 1. 이미 등장한 이름인지 검사하기 위해
// 2. 새로운 이름을 삽입할 위치를 찾기 위해 binary_search 함수를 사용
// 새로운 이름을 저장할 메모리 공간을 확보하기 위해 memmove 함수를 이용하여 메모리에 저장된 내용을 복사
// names->capacity는 1000으로부터 시작하여 1000씩 증가 (1000, 2000, 3000, ...)
// start_year : 시작 연도 (2009)
void load_names(FILE* fp, int start_year, tNames* names) {
    int year = 0;
    char name[20];
    char sex = 0;
    int freq = 0;
    int found = 0;
    int loc = 0;
    tName this;

    int i = 0;

    //자꾸 바이너리 서치의 첫번째 단계에서 l=0, r=-1일 때 m=0이 되서 그냥 첫번째 데이터값은 바로 가져와서 입력
    fscanf(fp, "%d\t%s\t%c\t%d\n", &year, name, &sex, &freq);
    strcpy(names->data[0].name, name);
    names->data[0].sex = sex;
    names->data[0].total_freq = freq;
    names->data[0].freq[year - start_year] = freq;
    names->len++;

    
    while (1) {
        //데이터 4개가 안잡히면 while문 종료
        if (fscanf(fp, "%d\t%s\t%c\t%d\n", &year, name, &sex, &freq) != 4)
            break;
        else {
            //데이터 갯수 용량 초과할 때마다 1000개씩 추가해서 realloc
            if (names->len == names->capacity) {
                names->capacity += 1000;
                names->data = (tName*)realloc(names->data, sizeof(tName) * names->capacity);
            }

            strcpy(this.name, name);
            this.sex = sex;


            

            //데이터가 잘 들어왔는지 확인
            //printf("%s %d %c %d\n", name, year, sex, freq);

            //바이너리 서치로 위치값 가져오기
            loc = binary_search(&this, names->data, names->len, sizeof(tName), compare_by_name, &found);

            if (found == 1) {
                names->data[loc].total_freq += freq;
                names->data[loc].freq[year - start_year] += freq;
                //printf("found 1 거쳐감\n");
            }
            else {
                memmove(names->data + (loc + 1), names->data + loc, sizeof(tName) * (names->len - loc));
                //memmove로는 복사를 하는거라 이전에 있던 위치 공간을 한 번 비워줘야되네 이거때문이었네 진짜 개빡취네진짜
                for (int i = 0; i < 10; i++) {
                    names->data[loc].freq[i] = 0;
                }

                //새로운 데이터 넣는거니까 다시 그대로 입력
                strcpy(names->data[loc].name, this.name);
                names->data[loc].sex = this.sex;
                names->data[loc].total_freq = freq;
                names->data[loc].freq[year - start_year] = freq;
                //printf("found 0 거쳐감\n");
                names->len++;
            }


        }
    }
}

void print_names(tNames* names, int num_year) {
    for (int i = 0; i < names->len; i++) {
        printf("%s\t%c\t%d", names->data[i].name, names->data[i].sex, names->data[i].total_freq);
        for (int j = 0; j < num_year; j++) {
            printf("\t%d", names->data[i].freq[j]);
        }
        printf("\n");
    }
}

int binary_search(const void* key, const void* base, size_t nmemb, size_t size,
    int (*compare)(const void*, const void*), int* found) {
    tName* this_base = (tName*)base;
    int l, r, m;
    l = 0;
    r = nmemb - 1;
    //printf("\n\n%d\n\n", r);

    while (1) {
        m = (l + r) / 2;
        //printf("%d\n", m);

        if (compare(key, this_base + m) == 0) {
            *found = 1;
            //printf("키랑 m 자리 같다고 나옴\n");
            return m;
        }
        else if (compare(key, this_base + m) > 0) {
            //printf("%s %c 지금 베이스에 들어있는거\n", this_base[m].name, this_base[m].sex);
            l = m + 1;
            //printf("l 업데이트함\n");
        }
        else {
            r = m - 1;
            //printf("r 업데이트함\n");
        }
        if (r < l) {
            break;
        }
    }

    *found = 0;
    return l;
}

int compare_by_name(const void* n1, const void* n2) {
    tName* first = (tName*)n1;
    tName* second = (tName*)n2;

    if (strcmp(first->name, second->name) == 0) {
        return first->sex - second->sex;
    }
    else {
        return strcmp(first->name, second->name);
    }

    return 0;
}

// 정렬 기준 : 빈도 내림차순(1순위), 이름(2순위), 성별(3순위)
int compare_by_freq(const void* n1, const void* n2) {
    tName* first = (tName*)n1;
    tName* second = (tName*)n2;

    if (first->total_freq == second->total_freq) {
        if (strcmp(first->name, second->name) == 0) {
            return first->sex - second->sex;
        }
        else {
            return strcmp(first->name, second->name);
        }
    }
    else return second->total_freq - first->total_freq;
}