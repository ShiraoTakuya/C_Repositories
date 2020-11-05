#ifndef ____include_h__
#define ____include_h__

#define __DOWN_ERROR (-2)
#define __DOWN_NOTFINISH (0)
#define __DOWN_DURING (1)
#define __DOWN_FINISH (2)
#define __DOWN_NA (3)
#define MAX (4096)
#define DOWNUNIT (1024*1024)
#define __FIRST_START (1) //__arrayanalysis用の宣言 はじめにValueが見つかるぽいんと　
#define __FIRST_END (2) //__arrayanalysis用の宣言 はじめにValueが終わるポイント
#define FLAG_FORCE_END (-1)
#define IP_DEVIDE_NUM (2)
#define RETRY_NUM (10)
#define MAX_BUF	(1024)
#define FLAG_ON (1)
#define FLAG_OFF (0)
#define TIMEOUT_S (10)

#include <math.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <process.h>
#include "WinSock2.h"
#include <windows.h>
#include <Wininet.h>
#include <mbstring.h>
#include <time.h>
#include "Ws2tcpip.h"
#include <io.h>
#include <Wincrypt.h>
#include <curl/curl.h>


#pragma comment( lib, "Wininet.lib" )
#pragma comment(lib,"ws2_32.lib")

/* 構造体定義 */
typedef struct{
	char *stData;
	int iIP;
	char *stURL;
	char *stCookie;
	int iDownstartbyte;
	int *piDownsize;
	char *pcState; //ダウンロードのステータスを記録する
	BOOL bFlag;
} THCONNECT;

/* グローバル変数宣言 */
HWND __global__hDraw = NULL;
BOOL __global__bSubflag = FLAG_OFF;
BOOL __global__bDownloadFailure = FLAG_OFF;
BOOL __global__bStopDownload = FLAG_OFF;
BOOL __global__bListDelete = FLAG_ON;
char __global__stTargetFileURL[MAX];
char __global__stTargetFileURL_COPY[MAX];
char __global__stFileName[MAX];
char __global__stFilePath[MAX];
int __global__iFileSize;
int __global__iFileCountNum; //= (iFileSize - 1)/DOWNUNIT + 1
double __global__dSpeed;
int __global__iDownloadedSize;
int __global__iRestMainFileNum;
int __global__iRestSubFileNum;
int __global__iAdaptNum;
HWND __global_hWndWindow = 0;
char **__global__pstBigdata;
BOOL *__global__pbMemoryMallocFlag;
char *__global__stState;
int *__global__piDownSize;
int *__global__piAdaptIP;
BOOL __global__bAddHistoryFlag = FLAG_ON;
char __global__stID[MAX];

#define DOWNLOAD_LIST "DownloadList.ini"
#define HISTORY_LIST "HistoryList.ini"
#define PROGRAM_NAME "MultiNetLineReceiver"

#endif ____include_h__