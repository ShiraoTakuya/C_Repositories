#ifndef ____window_h_
#define ____window_h_

#define ID_BUTTON1 (0)
#define ID_BUTTON2 (1)
#define ID_BUTTON3 (2)
#define ID_EDIT1 (6)
#define ID_DRAW1 (7)
#define ID_TIMER (8)
#define WINDOW_SIZE_X (368)
#define WINDOW_SIZE_Y (207)
#define BUTTON_HEIGHT (20)
#define BUTTON_WIDTH (100)
#define BUTTON_NUM (3)

#include "__include.h"

/* 関数宣言 */

//__geturlの従属関数 指定文字列stBufからURLを取得してstURLに格納(同じ文字列変数をしてしても良い) //__geturlの従属関数 クリップボードの文字列をstClipに格納
int __GetClipboard(char *stClip, int iSize){
	HGLOBAL hMem;
	char *stBuf;
	if (OpenClipboard(NULL)){
		if ((hMem = GetClipboardData(CF_TEXT)) != NULL){
			if ((stBuf = (LPTSTR)GlobalLock(hMem)) != NULL){
				if (sizeof(stBuf) > iSize){
					__wprintf("\nerror: __getclipboard\n");
					return -1;
				}
				strcpy(stClip, stBuf);
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
	}

	return 0;
}

//__wndprocの従属関数
int __GetURL(char *stURL){
	//クリップボードの文字列取得
	char stBuf[MAX];
	char *p;
	
	//クリップボードの内容をstBufにコピー
	if (__GetClipboard(stBuf, MAX) == -1)
		return -1;

	//URLがなければreturn
	if ((p = strstr(stBuf, "http")) == NULL){
		__wprintf("error: __geturl");
		return -1;
	}

	//URL文字列
	char stUrlbuf[MAX];
	strcpy(stUrlbuf, p);

	//URLに使用NG文字があればNULL文字を追加して終了
	char stNG[] = "\\'|`^\"<>)(}{][ \r\n";
	LOOP_I(strlen(stUrlbuf)){
		LOOP_J(strlen(stNG)){
			if (stUrlbuf[i] == stNG[j]){
				stUrlbuf[i] = '\0';
				strcpy(stURL, stUrlbuf);
				return 0;
			}
		}
	}
	strcpy(stURL, stUrlbuf);
	
	return 0;
}

//__wndprocの従属関数 指定したhWndのフォントをいつものにする
void __SendFont(HWND hWnd){
	static HFONT hFont = NULL;

	//初めての実行の場合フォントを規定する
	if (hFont == NULL){
		while (1){
			hFont = CreateFont(12,       //フォント高さ
				0,                      //文字幅（高さと同じ）
				0,                  //テキストの角度
				0,                      //ベースラインとｘ軸との角度
				FW_REGULAR,             //フォントの重さ（太さ）
				FALSE,                  //イタリック体
				FALSE,                  //アンダーライン
				FALSE,                  //打ち消し線
				SHIFTJIS_CHARSET,       //文字セット
				OUT_DEFAULT_PRECIS,     //出力精度
				CLIP_DEFAULT_PRECIS,    //クリッピング精度
				PROOF_QUALITY,          //出力品質
				FIXED_PITCH | FF_MODERN,  //ピッチとファミリー
				(LPCTSTR)"ＭＳ Ｐゴシック");                  //書体名

			if (hFont != NULL)
				break;
			Sleep(1);
		}
	}

	//指定したハンドルにフォントを設定する
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
}

void __GetRestFileNum(){

	FILE *fp;
	char stBuf[MAX];
	int iRestMainFileNum = 0, iRestSubFileNum = 0;

	
	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//mainの一覧まで移動
	while (fgets(stBuf, MAX - 1, fp)){
		if (strstr(stBuf, "【main】") != NULL)
			break;
	}

	//空白文字までの行数をカウント
	for (iRestMainFileNum = 0; fgets(stBuf, MAX - 1, fp); iRestMainFileNum++){
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
	}

	//subの一覧まで移動
	while (fgets(stBuf, MAX - 1, fp)){
		if (strstr(stBuf, "【sub】") != NULL)
			break;
	}

	//空白文字までの行数をカウント
	for (iRestSubFileNum = 0; fgets(stBuf, MAX - 1, fp); iRestSubFileNum++){
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
	}

	__global__iRestMainFileNum = iRestMainFileNum;
	__global__iRestSubFileNum = iRestSubFileNum;

	fclose(fp);

	return;
}

void __AddMainList(){
	FILE *fp;
	int iWriteLine;
	char stBuf[MAX];
	char stURL[MAX];

	//クリップボードからURLを取得
	if (__GetURL(stURL) == -1)
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
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
		//見つかったらBreak
		if (strstr(stBuf, "【main】"))
			break;
	}

	//空白行まで移動
	for (; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//見つかったらBreak
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
		//改行文字削除
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//URLをリストに挿入
	sprintf(stList, "%s%s\n\n", stList, stURL);

	//ダウンロードURL以降の行数を取得
	for (; fgets(stBuf, MAX, fp); iWriteLine++){
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//URLを追加したリストを作成
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//終了処理
	free(stList);
	fclose(fp);

	return;
}

void __AddSubList(){
	FILE *fp;
	int iWriteLine;
	char stBuf[MAX];
	char stURL[MAX];

	//クリップボードからURLを取得
	if (__GetURL(stURL) == -1)
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
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
		//見つかったらBreak
		if (strstr(stBuf, "【sub】"))
			break;
	}

	//空白行まで移動
	for (; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//見つかったらBreak
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
		//改行文字削除
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//URLをリストに挿入
	sprintf(stList, "%s%s\n", stList, stURL);

	//ダウンロードURL以降の行数を取得
	for (; fgets(stBuf, MAX, fp); iWriteLine++){
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//URLを追加したリストを作成
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//終了処理
	free(stList);
	fclose(fp);

	return;
}

//簡易ウィンドウのwndproc
LRESULT CALLBACK __wndproc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	int wmId;
	char stURL[MAX];
	char stBuf[MAX];
	static HWND hWndButton[3];
	HDC hDC, hDCBuf;
	HBITMAP hBitmapBuf;
	PAINTSTRUCT ps;
	static RECT rc;
	int iWinChildWidth;
	HFONT hFont, hFontOld;
	static char pstButtonName[][MAX] = { "ADD URL", "ADD SUBLIST", "編集" };
	double dPercent = 0;

	switch (msg) {
	case WM_PAINT:
		hDC = BeginPaint(__global__hDraw, &ps);

		//バッファを確保
		hDCBuf = CreateCompatibleDC(hDC);
		hBitmapBuf = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
		SelectObject(hDCBuf, hBitmapBuf);
		DeleteObject(hBitmapBuf);

		//描画フォント設定
		hFont = __SetFont((LPCTSTR)"ＭＳ Ｐゴシック", 12, 0);
		while ((hFontOld = (HFONT)SelectObject(hDCBuf, hFont)) == NULL)
			Sleep(1);

		//残数確認
		__GetRestFileNum();

		//速度とダウンロード率描画
		(__global__iFileSize != 0) ? dPercent = (double)__global__iDownloadedSize / __global__iFileSize * 100 : NULL;
		sprintf(stBuf,
			"速度: %.1fKB/s\n\
			完了: %.1f%%(%.0fKB / %.0fKB)\n\
			残数: %d(高), %d(低)\n\
			回線: %d個\n\
			名称: %s\n\
			URL: %s",
			__global__dSpeed,
			dPercent, (double)__global__iDownloadedSize / 1024, (double)__global__iFileSize / 1024,
			__global__iRestMainFileNum, __global__iRestSubFileNum,
			__global__iAdaptNum,
			__global__stFileName,
			__global__stTargetFileURL);

		//描画領域取得
		GetClientRect(__global__hDraw, &rc);

		//描画
		SetBkMode(hDCBuf, TRANSPARENT);
		SetTextColor(hDCBuf, RGB(255, 255, 255));
		DrawText(hDCBuf, stBuf, -1, &rc, DT_EDITCONTROL | DT_WORDBREAK | DT_WORD_ELLIPSIS);
		BitBlt(hDC, 0, 0, rc.right, rc.bottom, hDCBuf, 0, 0, SRCCOPY);

		//描画領域解放
		DeleteDC(hDCBuf);
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
		EndPaint(__global__hDraw, &ps);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wp);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case ID_BUTTON1:
			/* ADD URLボタンの動作*/
			__AddMainList();
			break;
		case ID_BUTTON2:
			/* ADD SUBLISTボタンの動作*/
			__AddSubList();
			break;
		case ID_BUTTON3:
			/* 編集ボタンの動作 */
			ShellExecute(NULL, "open", DOWNLOAD_LIST, NULL, NULL, SW_SHOWNORMAL);
			break;
		default:
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;
	case WM_CREATE:
		//クライアントの領域を取得
		GetClientRect(hWnd, &rc);

		//文字列描画領域設定
		__global__hDraw = CreateWindow(TEXT("STATIC"), TEXT(NULL), SS_SIMPLE | WS_CHILD | WS_VISIBLE | WS_BORDER,
			rc.right / 2,
			0,
			rc.right / 2,
			rc.bottom,
			hWnd, (HMENU)ID_DRAW1, NULL, NULL);
		
		//ボタン配置決定
		for (int i = 0; i < BUTTON_NUM; i++){
			hWndButton[i] = CreateWindow(TEXT("BUTTON"), pstButtonName[i], WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
				((rc.right / 2) - BUTTON_WIDTH) / 2,
				((rc.bottom-BUTTON_HEIGHT*BUTTON_NUM)/(BUTTON_NUM+1))*(i+1)+(i)*BUTTON_HEIGHT,
				BUTTON_WIDTH,
				BUTTON_HEIGHT,
				hWnd, (HMENU)i, NULL, NULL);
				__SendFont(hWndButton[i]);
		}

		SetTimer(hWnd, ID_TIMER, 100, NULL);
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_CLOSE:
		exit(0);
		break;
	case WM_RBUTTONUP:
		break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

//__thwindowの従属関数 センター座標を返却する
RECT __GetCenterRect(int width, int height){
	int iDispWidth, iDispHeight;

	iDispWidth = GetSystemMetrics(SM_CXSCREEN);
	iDispHeight = GetSystemMetrics(SM_CYSCREEN);

	POINT posCenter;
	posCenter.x = iDispWidth / 2;
	posCenter.y = iDispHeight / 2;

	RECT rc;
	rc.left = posCenter.x - width / 2;
	rc.top = posCenter.y - height / 2;
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	return rc;
}

//簡易ウィンドウ
unsigned __stdcall __thwindow(void *hInstance){
	MSG msg;
	WNDCLASS winc;
	RECT rc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = __wndproc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = (HINSTANCE)hInstance;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT(PROGRAM_NAME);

	if (!RegisterClass(&winc)) return 0;

	rc = __GetCenterRect(WINDOW_SIZE_X, WINDOW_SIZE_Y);
	__global_hWndWindow = CreateWindowEx(WS_EX_COMPOSITED, TEXT(PROGRAM_NAME), TEXT(PROGRAM_NAME), WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, (HINSTANCE)hInstance, NULL);

	while (GetMessage(&msg, NULL, 0, 0)) DispatchMessage(&msg);
	return msg.wParam;
}

//簡易ウィンドウをスレッドで呼び出す
HWND __CreateWindow(void *hInstance){
	HANDLE thWindow;
	thWindow = (HANDLE)_beginthreadex(NULL, 0, __thwindow, hInstance, 0, NULL);
	while (__global_hWndWindow == 0)
		Sleep(100);

	return __global_hWndWindow;
}

#endif ____window_h_