/*
*  DKU Operating System Lab (2026)
*      Lab2 (Concurrency Data Structure: Hash Table)
*      Student id : 32225001
*      Student name : 홍성제
*      Date: 2026.05.05.
*/


#include "hashtable_impl.h"
#include <cstdlib>
#include <vector>
#include <algorithm>

// DefaultHashTable
// DefaultHashTable 생성자
DefaultHashTable::DefaultHashTable(int num_buckets) : num_buckets_(num_buckets) {
    // 버킷 배열 할당 및 초기화 (모두 nullptr)
    buckets_ = new HTNode*[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        buckets_[i] = nullptr;
    }
}

// DefaultHashTable 소멸자
DefaultHashTable::~DefaultHashTable() {
    // 각 버킷의 체인 노드 전부 해제
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            HTNode* tmp = node->next;
            delete node;
            node = tmp;
        }
    }
    delete[] buckets_;
}

// 해시 함수 : key를 버킷 수로 나눈 나머지
int DefaultHashTable::hash_func(int key) {
    return key % num_buckets_;
}

// 순회 함수 : 전체 버킷을 순회하여 key 오름차순으로 정렬 후 arr에 저장
// Hash Table은 삽입 순서를 보장하지 않으므로, std::sort로 정렬한다.
void DefaultHashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            KVC kvc;
            kvc.key     = node->key;
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);
            node = node->next;
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) {
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {
        arr[i] = tmp[i];
    }
}

// HashTable (without lock)
// HashTable 생성자
HashTable::HashTable(int num_buckets) : DefaultHashTable(num_buckets) {}

// HashTable 소멸자
HashTable::~HashTable() {}

void HashTable::insert(int key, int value) {
    // 구현
    int idx = hash_func(key);   //해시 함수로 들어갈 버킷 인덱스 계산
    HTNode* curr = buckets_[idx]; //해당 버킷의 첫 번째 노드 포인터 가져오기

    // 체인 돌면서 동일한 키가 있는지 확인
    // 키가 존재하면 기존 값에 더하고 업데이트 카운트 증가
    while (curr != nullptr) {
        if (curr->key == key) {  //만약 같은 키 찾으면
            curr->value += value; //기존 값에 새 값 더하기
            curr->upd_cnt += 1;   //업데이트 횟수 +1
            return;  //종료
        }
        curr = curr->next; //못 찾았으면 다음 노드로
    }

    // 동일한 키가 없으면 새 노드를 할당하고 현재 버킷 리스트 맨 앞에 삽입
    HTNode* newNode = new HTNode;   //동적 할당으로 새 노드 만들기
    newNode->key = key;   //키 값 세팅
    newNode->value = value; //value 세팅
    newNode->upd_cnt = 0;   //처음 넣는 거니까 횟수 0으로
    newNode->next = buckets_[idx];  //맨앞에 추가하기 위해 새 노드 다음 포인터를 기존 버킷 헤드로 연결
    buckets_[idx] = newNode;  // 버킷 헤드 포인터 갱신
    //구현
}

int HashTable::lookup(int key) {
    // 구현
    int idx = hash_func(key);   //찾은 키 버킷 인덱스 계산
    HTNode* curr = buckets_[idx];  //해당 버킷 헤드 노드부터 탐색 시작

    //해당 버킷 연결 리스트를 돌면서 키가 일치하면 value 반환
    while (curr != nullptr) {
        if (curr->key == key) {   //찾는 키랑 일치하는 노드 발견시
            return curr->value;   //value 값 리턴
        }
        curr = curr->next;   //못찾았으면 다음 노드로
    }
    return 0; // 없으면 0 반환
    //구현
}

void HashTable::remove(int key) {
    // 구현
    int idx = hash_func(key);    //지울 인덱스 위치 
    HTNode* curr = buckets_[idx];  //현재 탐색 중인 노드
    HTNode* prev = nullptr;  //이전 노드를 가리킬 포인터

    //리스트를 돌면서 삭제할 노드 탐색
    while (curr != nullptr) {
        if (curr->key == key) { //지울 노드를 찾았을 때
            if (prev == nullptr) {
                // 삭제할 노드가 체인의 첫 번째 노드인 경우 포인터 재연결
                buckets_[idx] = curr->next;
            } else {
                // 삭제할 노드가 체인의 중간이나 끝인 경우 포인터 재연결
                prev->next = curr->next;
            }
            delete curr; // 메모리 해제
            return;
        }
        prev = curr;    //현재 노드 prev에 킵
        curr = curr->next;  //curr은 다음 탐색할 노드로 이동
    }
    //구현
}

void HashTable::traversal(KVC* arr) {
    // 구현
    DefaultHashTable::traversal(arr); //부모 클래스의 로직 그대로 사용 
    //구현
}

// CoarseHashTable (coarse-grained lock)
// CoarseHashTable 생성자
CoarseHashTable::CoarseHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    pthread_mutex_init(&mutex_lock, nullptr);
}

// CoarseHashTable 소멸자
CoarseHashTable::~CoarseHashTable() {
    pthread_mutex_destroy(&mutex_lock);
}

void CoarseHashTable::insert(int key, int value) {
    // 구현
    pthread_mutex_lock(&mutex_lock); // 탐색 및 삽입 전 전체 락
    
    int idx = hash_func(key);        //해시 함수로 버킷 인덱스 계산
    HTNode* curr = buckets_[idx];    //해당 인덱스의 버킷 헤드 가져옴

    while (curr != nullptr) {    //버킷 리스트 뒤지기
        if (curr->key == key) {   //이미 있는 키라면
            curr->value += value;   //누적해서 값 더함
            curr->upd_cnt += 1;     //업데이트 카운트 +1
            pthread_mutex_unlock(&mutex_lock); // 리턴 전 잠금 해제
            return;
        }
        curr = curr->next;   //다음 노드로
    }

    //새 노드 삽입 처리
    HTNode* newNode = new HTNode;   //새 노드 메모리 할당
    newNode->key = key;        //키 입력
    newNode->value = value;    //값 입력
    newNode->upd_cnt = 0;       //업데이트 카운트 0
    newNode->next = buckets_[idx];  //맨 앞 삽입 위해 next를 기존 헤드에 연결
    buckets_[idx] = newNode;     //버킷 헤드를 새 노드로 변경

    pthread_mutex_unlock(&mutex_lock); // 작업 끝나고 전체 락 해제
    //구현
}

int CoarseHashTable::lookup(int key) {
    // 구현
    //읽는 중에도 다른 스레드의 쓰기가 발생할 수 있으므로 전체 락 획득
    pthread_mutex_lock(&mutex_lock);
    
    int idx = hash_func(key);  //인덱스 계산
    HTNode* curr = buckets_[idx]; //탐색할 첫 노드 포인터 세팅
    int result = 0;   //반환할 값 저장할 임시 변수

    while (curr != nullptr) {  //리스트 탐색 시작
        if (curr->key == key) {  //해당 키 찾으면
            result = curr->value;  //result 변수에 값을 담음
            break;  
        }
        curr = curr->next;   //못 찾았으면 다음 노드로 넘어감
    }

    pthread_mutex_unlock(&mutex_lock); // 전체 락 해제
    return result;
    //구현
}

void CoarseHashTable::remove(int key) {
    // 구현
    //전체 락 획득
    pthread_mutex_lock(&mutex_lock);
    
    int idx = hash_func(key);      //버킷 인덱스 계산
    HTNode* curr = buckets_[idx];   //현재 위치 노드
    HTNode* prev = nullptr;        //이전 위치 노드 저장용 변수

    while (curr != nullptr) {   //삭제할 대상 찾는 루프
        if (curr->key == key) {   //찾으면
            //맨 앞 노드 삭제와 중간/끝 노드 삭제를 구분하여 포인터 재연결
            if (prev == nullptr) buckets_[idx] = curr->next;
            else prev->next = curr->next;
            
            delete curr; // 메모리 해제
            break; // 삭제했으면 루프 탈출
        }
        prev = curr;   //못 찾았으면 현재 노드 prev로 밀어줌
        curr = curr->next;  //다음 노드로 넘어감
    }

    pthread_mutex_unlock(&mutex_lock); //전체락 해제
    //구현
}

void CoarseHashTable::traversal(KVC* arr) {
    // 구현
    //도는 도중 리스트 구조나 값이 바뀌는 걸 막기 위해 잠금
    pthread_mutex_lock(&mutex_lock);  // 전체 데이터를 순회하는 도중에 리스트가 바뀌면 꼬이니까 테이블 전체 잠금
    DefaultHashTable::traversal(arr);   //부모 클래스의 traversal 호출해서 배열에 복사
    pthread_mutex_unlock(&mutex_lock);  //락해제
    //구현
}

// FineHashTable (fine-grained lock)
// FineHashTable 생성자
FineHashTable::FineHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    // 버킷 수만큼 뮤텍스 배열 할당 및 초기화
    bucket_locks = new pthread_mutex_t[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_init(&bucket_locks[i], nullptr);
    }
}

// FineHashTable 소멸자
FineHashTable::~FineHashTable() {
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_destroy(&bucket_locks[i]);
    }
    delete[] bucket_locks;
}

void FineHashTable::insert(int key, int value) {
    // 구현
    int idx = hash_func(key);  //어느 버킷으로 갈지 인덱스 구함
    
    pthread_mutex_lock(&bucket_locks[idx]); // 전체가 아닌 타켓 인덱스 버킷 락만 얻어서 병렬성 극대화
    
    HTNode* curr = buckets_[idx];  //락 잡고 헤드 노드 가져옴
    while (curr != nullptr) {    //타겟 버킷 체인 안에서만 탐색
        if (curr->key == key) {   // 중복되는 키가 있으면
            curr->value += value; //기존 값에 더함
            curr->upd_cnt += 1;  //업데이트 +1 증가
            pthread_mutex_unlock(&bucket_locks[idx]); // 리턴 전 잠금 해제
            return;
        }
        curr = curr->next; //다음 노드 확인
    }

    HTNode* newNode = new HTNode;  //새 노드 동적 할당
    newNode->key = key; //키 저장
    newNode->value = value; //값 저장
    newNode->upd_cnt = 0; //초기 업데이트 카운트0
    newNode->next = buckets_[idx]; //새 노드의 다음을 기존 버킷 헤드랑 연결
    buckets_[idx] = newNode; //헤드를 방금 만든 새 노드로 갱신

    pthread_mutex_unlock(&bucket_locks[idx]); // 완료 후 잠금 해제
    //구현
}

int FineHashTable::lookup(int key) {
    // 구현
    int idx = hash_func(key);  // 조회할 인덱스 구하기
    
    //조회하려는 버킷의 락만 획득
    pthread_mutex_lock(&bucket_locks[idx]);
    
    HTNode* curr = buckets_[idx]; // 탐색 시작할 헤드 노드
    int result = 0; //찾은 결과 담을 변수 선언

    while (curr != nullptr) {  //버킷 체인 순회
        if (curr->key == key) {  //찾는 키 찾으면
            result = curr->value;  //반환용 변수에 값 저장
            break;  
        }
        curr = curr->next; //다음 노드로
    }

    pthread_mutex_unlock(&bucket_locks[idx]); //해당 버킷의 락만 해제
    return result;
    //구현
}

void FineHashTable::remove(int key) {
    // 구현
    int idx = hash_func(key); // 삭제할 버킷 위치 확인
    
    //타겟 버킷의 락만 획득
    pthread_mutex_lock(&bucket_locks[idx]);
    
    HTNode* curr = buckets_[idx]; // 현재 노드 추적
    HTNode* prev = nullptr; //이전 노드 추적용 포인터

    while (curr != nullptr) {  //삭제 대상 탐색
        if (curr->key == key) {  //발견하면
            if (prev == nullptr) buckets_[idx] = curr->next;  //맨 앞이면 버킷 헤드 포인터를 다음 노드로 덮어씀
            else prev->next = curr->next;   //중간 노드면 prev와 next 를 이어서 타겟을 체인에서 분리
            
            delete curr;  //타겟 노드 할당 해제
            break;
        }
        prev = curr; // 못 찾았으면 prev 포인터 한 칸 전진
        curr = curr->next; // curr 포인터 한 칸 전진
    }

    pthread_mutex_unlock(&bucket_locks[idx]); //해당 버킷의 락만 해제
    //구현
}

void FineHashTable::traversal(KVC* arr) {
    // 구현
    // 안전을 위해 모든 버킷의 락을 순서대로 획득 (Deadlock 방지)
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_lock(&bucket_locks[i]); // i번째 버킷 락 획득
    }
    
    DefaultHashTable::traversal(arr); //기본 순회 함수 실행
    
    // 완료 후 모든 락 해제
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_unlock(&bucket_locks[i]); // i번째 버킷 락 해제
    }
    //구현
}
