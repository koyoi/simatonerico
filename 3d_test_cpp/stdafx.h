// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#ifdef _WIN32_
#define NOMINMAX  // min/max マクロを無効化（std::min/max との衝突を防ぐ）
#include "windows.h"

// USE_GDIPLUS を定義すると GDI+ を使用（OpenCV不要）
// vcxproj で定義済みの場合は再定義しない
#ifndef USE_GDIPLUS
#define USE_GDIPLUS
#endif

#ifndef USE_GDIPLUS
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif
#endif

