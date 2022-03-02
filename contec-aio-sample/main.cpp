/*
 * CONTEC��AIO�{�[�h AIO-163202F-PE �̃T���v���v���O����
 *
 * �A�i���O�o�́F-10[V]����10[V]�܂�1[V]����, 500[ms]�Ԋu�ŏo�͂���B
 * �A�i���O���́F�A�i���O���͒l�i�V���O���G���h���́j����ɃR���\�[���ɏo��
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
	short Id; // �f�o�C�X���ʂɕK�v��ID�ŁA���������Ɏ擾����
	char ErrorString[256];

	// �f�o�C�X����������
	Ret = AioInit(const_cast<char*>("Aio000"), &Id); // 0������Ȗ߂�l
	ErrorRet = AioGetErrorString(Ret, ErrorString);
	if (Ret != 0) {
		printf("AioInit �F%s", ErrorString);
	}

	/*
	 * �ȈՃA�i���O���o�̓e�X�g
	 */

	 // �A�i���O�o�͊֘A�̐ݒ�
	 // �A�i���O�o�̓����W�ݒ�
	Ret = AioSetAoRange(Id, /*AoChannel*/ 0, /*AoRange*/ PM10); // PM10 -> +-10V�ɐݒ�
	Ret = AioSetAoRange(Id, /*AoChannel*/ 1, /*AoRange*/ PM10); // PM10 -> +-10V�ɐݒ�

	// �A�i���O�o�͊֘A�̐ݒ�
	// �A�i���O���͕����̐ݒ�
	// �O���E���h�ԓd�ʍ���m�C�Y�������傫���Ƃ��͍������͂�����
	Ret = AioSetAiInputMethod(Id, /*AiInputMethod*/ 0); //AiInputMethod: 0->�V���O���G���h����, 1->��������
	Ret = AioSetAiRange(Id, /*AiChannel*/ 0, /*AiRange*/ PM10);
	Ret = AioSetAiRange(Id, /*AiChannel*/ 1, /*AiRange*/ PM10);

	// �A�i���O�o�͂��I�������A�i���O���͒l�̕`��X���b�h���I�����邽�߂̕ϐ�
	isOutputFinished.store(false);

	// �A�i���O���͒l���擾���R���\�[���ɕ`��
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

	// �w�肵���d���l���A�i���O�o�͂���
	Ret = AioSingleAoEx(Id, 0, 0);
	for (long i = -10; i <= 10; i++) {
		Ret = AioSingleAoEx(Id, /*AoChannel*/0, /*AoData*/ (float)i); // 0[V]���o��
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	Ret = AioSingleAoEx(Id, 0, 0);

	// �A�i���O���͒l�̕`��X���b�h�I������
	isOutputFinished.store(true);
	aiThread.join();

	// �f�o�C�X�I������
	Ret = AioExit(Id);
	ErrorRet = AioGetErrorString(Ret, ErrorString);
	if (Ret != 0) {
		printf("AioExit �F%s", ErrorString);
	}

	return 0;
}