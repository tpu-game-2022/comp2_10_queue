#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#include "Windows.h"                    // Windows API の機能定義
#include <stdlib.h>

#include "../include/lib_func.h"


// mem_size の容量でキュー用のメモリを確保する
void initialize(QUEUE* q, size_t mem_size)
{
	if (q == NULL) return;

	q->memory_begin = (int*)malloc(mem_size);
	q->memory_end = q->memory_begin + mem_size / sizeof(int);
	q->head = q->memory_begin;
	q->tail = q->memory_begin;
}


// 確保したメモリを解放する
void finalize(QUEUE* q)
{
	if (q == NULL) return;

	free(q->memory_begin);

	q->memory_begin = NULL;
	q->memory_end = NULL;
	q->head = NULL;
	q->tail = NULL;
}


// valの値をキューに入れる。実行の成否を返す
bool enqueue(QUEUE* q, int val)
{
	if (q == NULL || q->memory_begin == NULL) return false;// 未初期化
	if (countQueueableElements(q) < 1) return false;// 容量不足

	// 値を入れる
	*q->head++ = val;

	// アドレスが最後まで行ったら最初に戻る
	if(q->memory_end <= q->head){
		q->head = q->memory_begin;
	}

	return true;
}


// addrから始まるnum個の整数をキューに入れる。実行の成否を返す
bool enqueue_array(QUEUE* q, int* addr, int num)
{
	if (q == NULL || q->memory_begin == NULL) return false;// 未初期化
	if (countQueueableElements(q) < num) return false;// 容量不足
	if (addr == NULL) return false;// 入力データが変
	if (num <= 0) return false;// 何も追加しない

	// 分けて記録しなければならないか？
	int num_for_end = (int)(q->memory_end - q->head);
	if (num_for_end < num) {
		// 最後まで
		int num_left = num - num_for_end;
		size_t size = sizeof(int) * num_for_end;
		memcpy_s(q->head, size, addr, sizeof(int) * num);
		// 最初から
		size = sizeof(int) * num_left;
		memcpy_s(q->memory_begin, size, addr + num_for_end, size);

		q->head = q->memory_begin + num_left;
		return true;
	}

	// 一回で記録
	size_t size = sizeof(int) * num;
	memcpy_s(q->head, size, addr, size);
	q->head += num;

	// アドレスが最後まで行ったら最初に戻る
	if (q->memory_end <= q->head) {
		q->head = q->memory_begin;
	}

	return true;
}

// キューから一つの要素を取り出す(不具合時は0を返す)
int dequeue(QUEUE* q)
{
	if (q == NULL || q->memory_begin == NULL) return 0;// 未初期化
	if (countQueuedElements(q) < 1) return 0;// データが入っていない

	int ret = *q->tail++;

	// アドレスが最後まで行ったら最初に戻る
	if(q->memory_end <= q->tail){
		q->tail = q->memory_begin;
	}

	return ret;
}

// addrにキューからnumの要素を取り出す。取り出せた個数を返す
int dequeue_array(QUEUE* q, int* addr, int num)
{
	if (q == NULL || q->memory_begin == NULL) return 0;// 未初期化
	if (addr == NULL) return false;// 入力データが変
	if (num <= 0) return 0;// 何も取り出さない

	// 取り出す個数は格納された数が上限
	int num_queued = countQueuedElements(q);
	num = (num < num_queued) ? num : num_queued;

	// 最初に戻って読み込まなくてはならない場合
	int num_for_end = (int)(q->memory_end - q->tail);
	if(num_for_end < num){
		// 最後まで
		size_t size = sizeof(int) * num_for_end;
		memcpy_s(addr, size, q->tail, size);
		// 最初から
		addr -= num_for_end;
		int num_left = num - num_for_end;
		size = sizeof(int) * num_left;
		memcpy_s(addr, size, q->memory_begin, size);

		q->tail = q->memory_begin + num_left;
		return num;
	}

	memcpy_s(addr, sizeof(int) * num, q->tail, sizeof(int) * num);

	q->tail += num;
	if (q->memory_end <= q->tail) {
		q->tail = q->memory_begin;
	}

	return num;
}

// キューが空かどうかを調べる
bool isEmpty(const QUEUE* q)
{
	if (q == NULL) return false;

	return q->head == q->tail;
}

static int getMaxCount(const QUEUE* q)
{
	if (q == NULL || q->memory_begin == NULL) return 0;

	return (int)(q->memory_end - q->memory_begin);
}

// 挿入されたデータ数を得る
int countQueuedElements(const QUEUE* q)
{
	if (q == NULL || q->memory_begin == NULL) return 0;

	int max_counts = getMaxCount(q);
	return (q->head + max_counts - q->tail) % max_counts;
}

// 挿入可能なデータ数を得る
int countQueueableElements(const QUEUE* q)
{
	return getMaxCount(q) - countQueuedElements(q) - 1;
}
