/*
 * CONTECのAIOボード AIO-163202F-PE のサンプルプログラム
 *
 * アナログ出力：-10[V]から10[V]まで1[V]刻み, 500[ms]間隔で出力する。
 * アナログ入力：アナログ入力値（シングルエンド入力）を常にコンソールに出力
 */

#include <iostream>
#include <chrono>
#include <windows.h>
#include <thread>
#include <atomic>

#include "Caio.h"

std::atomic_bool isOutputFinished;

int main() {


	long Ret;
	long ErrorRet;
	short Id; // デバイス識別に必要なIDで、初期化時に取得する
	char ErrorString[256];

	// デバイス初期化処理
	Ret = AioInit(const_cast<char*>("Aio000"), &Id); // 0が正常な戻り値
	ErrorRet = AioGetErrorString(Ret, ErrorString);
	if (Ret != 0) {
		printf("AioInit ：%s", ErrorString);
	}

	/*
	 * 簡易アナログ入出力テスト
	 */

	 // アナログ出力関連の設定
	 // アナログ出力レンジ設定
	Ret = AioSetAoRange(Id, /*AoChannel*/ 0, /*AoRange*/ PM10); // PM10 -> +-10Vに設定
	Ret = AioSetAoRange(Id, /*AoChannel*/ 1, /*AoRange*/ PM10); // PM10 -> +-10Vに設定

	// アナログ出力関連の設定
	// アナログ入力方式の設定
	// グラウンド間電位差やノイズ成分が大きいときは差動入力を検討
	Ret = AioSetAiInputMethod(Id, /*AiInputMethod*/ 0); //AiInputMethod: 0->シングルエンド入力, 1->差動入力
	Ret = AioSetAiRange(Id, /*AiChannel*/ 0, /*AiRange*/ PM10);
	Ret = AioSetAiRange(Id, /*AiChannel*/ 1, /*AiRange*/ PM10);

	// アナログ出力が終わったらアナログ入力値の描画スレッドを終了するための変数
	isOutputFinished.store(false);

	// アナログ入力値を取得しコンソールに描画
	std::thread aiThread([&Id]() {
		while (true) {
			long Ret;
			char ErrorString[256];
			float data;
			Ret = AioSingleAiEx(Id, /*AiChannel*/ 0, /*AiData*/ &data);
			if (Ret != 0) {
				AioGetErrorString(Ret, ErrorString);
				printf("AioSetAiInputMethod : %s\n", ErrorString);
			}
			else {
				printf("\rInput:%f", data);
			}
			if (isOutputFinished.load()) break;
		}
	});

	// 指定した電圧値をアナログ出力する
	Ret = AioSingleAoEx(Id, 0, 0);
	for (long i = -10; i <= 10; i++) {
		Ret = AioSingleAoEx(Id, /*AoChannel*/0, /*AoData*/ (float)i); // 0[V]を出力
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	Ret = AioSingleAoEx(Id, 0, 0);

	// アナログ入力値の描画スレッド終了処理
	isOutputFinished.store(true);
	aiThread.join();

	// デバイス終了処理
	Ret = AioExit(Id);
	ErrorRet = AioGetErrorString(Ret, ErrorString);
	if (Ret != 0) {
		printf("AioExit ：%s", ErrorString);
	}

	return 0;
}