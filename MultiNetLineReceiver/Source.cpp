#define _CRT_SECURE_NO_WARNINGS

/* マクロ定義 */
#define SLEEP_UNTIL_FLAG_ON(flag) while(flag != FLAG_ON) Sleep(1)
#define SLEEP_UNTIL_FLAG_OFF(flag) while(flag != FLAG_OFF) Sleep(1)
#define SLEEP_UNTIL_FLAG_X(flag, x) while(flag != x) Sleep(1)
#define SLEEP_UNTIL_FLAG_NOTX(flag, x) while(flag == x) Sleep(1)
#define LOOP_I(times) for(int i = 0; i < (times); i++)
#define LOOP_J(times) for(int j = 0; j < (times); j++)
#define MALLOC_DEFINE_INITIALIZE(pointer, size, type) type *pointer = (type*)malloc((size)*sizeof(type)); memset(pointer, 0, (size)*sizeof(type))
#define STR_MATCH(str1, str2) ((strcmp(str1, str2) == 0))

#include "__include.h"
#include "__function.h"
#include "__window.h"

/* MAIN THREAD */

//初期化処理
void __Initialize(HINSTANCE hInst){
	//ウィンドウ作成、描画用hDraw取得まで待機
	__CreateWindow(hInst);
	SLEEP_UNTIL_FLAG_NOTX(__global__hDraw, NULL);
}

//ダウンロードリストからダウンロードURLを抽出
void __GetDownloadURL(){
	char *p;
	FILE *fp;
	char stBuf[MAX];
	while (1){

		//Downloadリストをオープン
		while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
			Sleep(1);

		//mainにURLの登録があるか確認
		while (fgets(stBuf, MAX, fp)){
			if (strstr(stBuf, "【main】")) break;
		}
		fgets(stBuf, MAX, fp);

		//mainにURLの登録があったらbreak
		if (strstr(stBuf, "http") != NULL) break;

		//subにURLの登録があるか確認
		while (fgets(stBuf, MAX, fp)){
			if (strstr(stBuf, "【sub】")) break;
		}
		fgets(stBuf, MAX, fp);
		//subにURLの登録があったらbreak
		if (strstr(stBuf, "http") != NULL){
			__global__bSubflag = FLAG_ON;
			break;
		}

		fclose(fp);
		Sleep(1);
	}

	//改行文字は削除
	if ((p = strchr(stBuf, '\n')) != NULL)
		*p = '\0';

	strcpy(__global__stTargetFileURL, stBuf);
	fclose(fp);

	return;
}

//ファイル名を採番
void __GetFileName(){
	FILE *fp;
	char stBuf[MAX];
	__global__stFileName[0] = 0;

	while ((fp = fopen(HISTORY_LIST, "r")) == NULL)
		Sleep(1);

	//履歴にURLがある場合同じファイル名を代入
	while (fgets(stBuf, MAX, fp)){
		if (strstr(stBuf, __global__stTargetFileURL) != NULL){
			*strchr(stBuf, '\t') = '\0';
			sprintf(__global__stFileName, "%s.mp4", stBuf);
			__global__bAddHistoryFlag = FLAG_OFF;
			break;
		}
	}
	fseek(fp, SEEK_SET, 0);
	//履歴にURLがなかった場合採番
	int iNum;
	int iMax = 1;
	int iBuf;
	if (__global__stFileName[0] == 0){
		//履歴ファイルの中の一番大きい番号をiMaxに代入
		while (fgets(stBuf, MAX, fp)){
			*strchr(stBuf, '\t') = '\0';
			iBuf = atoi(stBuf);
			(iBuf > iMax) ? iMax = iBuf : NULL;
		}
		sprintf(__global__stFileName, "%d.mp4", iMax + 1);
	}

	//ファイルパスを設定
	sprintf(__global__stFilePath, ".\\Download\\%s", __global__stFileName);

	//保存ファイルを作成(あれば何もしない)
	if (_access_s(__global__stFilePath, 0) == ENOENT)
	{
		// ファイルが存在しない場合の処理
		FILE *fp;
		while ((fp = fopen(__global__stFilePath, "wb+")) == NULL)
			Sleep(1);
		fclose(fp);
	}

	return;
}

//ヒストリーにURLを追加
void __AddHistory(){
	FILE *fp;

	//採番済みならリターン
	if (__global__bAddHistoryFlag == FLAG_OFF)
		return;

	//履歴ファイルに書き込み
	while ((fp = fopen(HISTORY_LIST, "a")) == NULL)
		Sleep(1);
	char *p;
	if ((p = strchr(__global__stTargetFileURL, '\n')) != NULL)
		*p = '\0';
	fprintf(fp, "%d\t%s\n", atoi(__global__stFileName), __global__stTargetFileURL);
	fclose(fp);

	return;
}

//ダウンロードしたURLをダウンロードリストから削除
void __DeleteList(){
	FILE *fp;
	int iWriteLine;
	char stBuf[MAX];

	//ダウンロード失敗フラグが立っていたらリターン
	if (__global__bDownloadFailure == FLAG_ON)
		return;

	if (__global__bListDelete == FLAG_OFF)
		return;

	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//対象のURLまでの情報を保管
	//ダウンロードURLを検索
	char *p;
	char *stList = (char*)malloc(MAX);
	memset(stList, 0, MAX);
	for (iWriteLine = 0; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//改行文字削除
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		p = strchr(__global__stTargetFileURL, '\n');
		if (p != NULL)
			*p = '\0';
		//ダウンロードURLが見つかったらBreak
		if (STR_MATCH(stBuf, __global__stTargetFileURL)) break;
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//ダウンロードURL以降の行数を取得
	for (; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//改行文字削除
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//ダウンロードURL以外の行数を書き出し
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//終了処理
	free(stList);
	fclose(fp);

	return;
}

//失敗したURLをダウンロードリストの最下層に移動
void __LowestPriority(){
	FILE *fp;
	int iWriteLine;
	char stBuf[MAX];

	//ダウンロード失敗フラグがOFFならリターン
	if (__global__bDownloadFailure == FLAG_OFF)
		return;

	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//対象のURLまでの情報を保管
	//ダウンロードURLを検索
	char *p;
	char *stList = (char*)malloc(MAX);
	memset(stList, 0, MAX);
	for (iWriteLine = 0; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//改行文字削除
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		//ダウンロードURLが見つかったらBreak
		if (STR_MATCH(stBuf, __global__stTargetFileURL)) break;
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//ダウンロードURL以降の行数を取得
	for (; fgets(stBuf, MAX, fp); iWriteLine++){
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//失敗したURLを最後に挿入
	sprintf(stList, "%s%s\n", stList, __global__stTargetFileURL);

	//ダウンロードURL以外の行数を書き出し
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//終了処理
	free(stList);
	fclose(fp);

	return;
}

//次のダウンロードに備えてグローバル変数を初期化
void __ClearGlobalVariable(){
	__global__bSubflag = FLAG_OFF;
	__global__bDownloadFailure = FLAG_OFF;
	__global__bListDelete = FLAG_ON;
	__global__bStopDownload = FLAG_OFF;
	__global__bAddHistoryFlag = FLAG_ON;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, PSTR lpCmdLine, int nCmdShow)
{
	//初期化
	__Initialize(hInst);

	while (1){
		//ダウンロードURLをダウンロードリストから読み取り
		__GetDownloadURL();

		//ファイル名を採番号(ついでにファイルパスも設定)
		__GetFileName();

		//履歴ファイル更新	
		__AddHistory();

		//ダウンロード開始
		__Download();

		//ダウンロードしたファイルをリストから削除する
		__DeleteList();

		//ダウンロードが失敗だった場合優先度を最低にする
		__LowestPriority();

		//グローバル変数を初期化する
		__ClearGlobalVariable();
		
	}

	return 0;
}
