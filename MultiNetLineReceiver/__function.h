#ifndef ____function_h_
#define ____function_h_

/* 汎用関数 */

//文字列を結合して戻り値で返す、文字列は呼び出すたびに消去される、関数の最終引数はNULLを入れること
char* __mg(int iNum, const char* format, ...){
	static BOOL bFlagRun = FLAG_OFF;
	static char *stString; 
	char *p;
	int iSize = 0;
	va_list args;

	//前回実行分のメモリ領域を解放
	if (bFlagRun == FLAG_ON)
		free(stString);

	//文字列の総文字数を計算
	va_start(args, format);
	for (int i = 0; i < iNum; i++){
		p = (char*)va_arg(args, char*);
		if (p == NULL)
			break;
		iSize += strlen(p);
	}
	iSize += strlen(format) + 1;

	//メモリ確保し実行フラグを立てる
	stString = (char*)malloc(iSize);
	bFlagRun = FLAG_ON;

	//文字列を結合する
	va_start(args, format);
	vsprintf(stString, format, args);
	va_end(args);

	return stString;
}

//__drawwindowtextの従属関数
HFONT __SetFont(LPCTSTR face, int h, int angle)
{
	HFONT   hFont;
	while (1){
		hFont = CreateFont(h,       //フォント高さ
			0,                      //文字幅（高さと同じ）
			angle,                  //テキストの角度
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
			face);                  //書体名

		if (hFont != NULL)
			break;
		Sleep(1);
	}
	return hFont;
}

//汎用関数 logファイルに出力する使い方はprintfと同じ
void __wprintf(const char* format, ...){

#ifdef __LOG_OUTPUT__
	char stBuf[MAX];
	va_list args;

	//可変長形式をただの文字列に直す
	va_start(args, format);
	vsprintf(stBuf, format, args);
	va_end(args);

	FILE *fp;

	//デバッグ時のみファイルに記録を残す
	while ((fp = fopen("log.ini", "a")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stBuf);

	fclose(fp);
#endif

	//細かい処理の出力は不要
	return;
}

//汎用関数 IEのクッキーを取得する、stCookieに代入、iSizeはstCookieのサイズ
void __GetIECookie(char *stCookie){

	stCookie[0] = 0;
	int iSize = MAX;
	
	// WinInetの初期化
	HINTERNET hInet = NULL;
	hInet = InternetOpen("WININET API Ver0.01", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInet != NULL){
		//IEのCookie情報を取得出来たらstCookieと融合する
		InternetGetCookie(__global__stTargetFileURL, NULL, stCookie, (LPDWORD)&iSize);
	}

	/* 後処理 */
	InternetCloseHandle(hInet);

	return;
}

//__DownloadWithCURLの従属関数
//__global__dSpeedにダウンロードスピード書き込み
void __DownSpeed(int iDownSize){
#define STACK_SIZE (100)
	//時間とダウンロードサイズの格納領域確保
	static int piDownSize[STACK_SIZE + 1] = { 0 };
	static time_t piDownTime[STACK_SIZE + 1] = { 0 };

	//現在のダウンロードサイズと時間をPUSH
	LOOP_I(STACK_SIZE - 1){
		piDownSize[STACK_SIZE - 1 - i] = piDownSize[STACK_SIZE - 2 - i];
		piDownTime[STACK_SIZE - 1 - i] = piDownTime[STACK_SIZE - 2 - i];
	}
	piDownSize[0] = iDownSize;
	piDownTime[0] = time(NULL);

	//ダウンロード速度計算 (iDownSize(t) - iDownSize(t-1))/(iDownTime(t) - iDownTime(t-1))
	double dDiffTime, dSpeed;
	int iDiffSize;
	int iPoint;
	for (iPoint = 1; piDownSize[iPoint] != 0; iPoint++);
	//初書き込みの場合はリターン
	if (iPoint == 1) return;

	//時間差を計算
	dDiffTime = difftime(piDownTime[0], piDownTime[iPoint - 1]);

	//時間差がないならリターン
	if (dDiffTime == 0)
		return;

	//ダウンロード総量を計算
	int iSumDownSize = 0;
	LOOP_I(iPoint)
		iSumDownSize += piDownSize[i];

	//ダウンロード速度算出、描画
	char stBuf[MAX];
	__global__dSpeed = iSumDownSize / dDiffTime / 1024;

	return;
}

size_t write_data(void *pBuf, size_t size, size_t nmemb, char **pstData)
{
	int iDownSize = size * nmemb;

	/* Copy the data from the curl buffer into our buffer */
	memcpy((void *)*pstData, pBuf, (size_t)iDownSize);

	/* Update the write index */
	*pstData += iDownSize;

	(*pstData)[0] = '\0';

	//__global__dSpeedにダウンロードスピード書き込み
	__DownSpeed(iDownSize);
	
	/* Return the number of bytes received, indicating to curl that all is okay */
	return iDownSize;
}

//cURLでファイルをダウンロード
void __DownloadWithCURL(char *stData, char *stURL, BOOL *pbFailureFlag, char *stRange, char *stInterface, char *stCookie, char *stOtherHeader, BOOL bHeaderFlag, int *piDownSize, char *stPostData){

	CURL *curl;
	CURLcode ret;
	char *p;

	/* First step, init curl */
	curl = curl_easy_init();
	if (!curl) {
		*pbFailureFlag = FLAG_ON;
		return;
	}

	//POST用処理
	if (stPostData[0] != '\0'){
		curl_easy_setopt(curl, CURLOPT_POST, TRUE);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, stPostData);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(stPostData));
	}

	//cURLにコマンドを設定
	p = stData;
	curl_easy_setopt(curl, CURLOPT_URL, stURL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (char **)&p);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	if (stRange[0] != '\0')
		curl_easy_setopt(curl, CURLOPT_RANGE, stRange);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.50.3"); 
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);	//リダイレクト
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, TRUE); //ロケーションをたどる
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
	if (stCookie != '\0');
		curl_easy_setopt(curl, CURLOPT_COOKIE, stCookie);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, TRUE);
	if (bHeaderFlag == FLAG_ON)
		curl_easy_setopt(curl, CURLOPT_HEADER, TRUE);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_S);
	curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL); //URL転送
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	if (stInterface[0] != '\0')
		curl_easy_setopt(curl, CURLOPT_INTERFACE, stInterface);
	if (stOtherHeader[0] != '\0')
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, stOtherHeader);

	/* Allow curl to perform the action */
	ret = curl_easy_perform(curl);

	//ダウンロードサイズ計算
	*piDownSize = p - stData;

	curl_easy_cleanup(curl);

	(ret != 0) ? *pbFailureFlag = FLAG_ON : *pbFailureFlag = FLAG_OFF;

	return;
}

//__downの従属関数 
unsigned __stdcall __thDonwnload(void *pth)
{
	//THCONNECTをコピーして解放フラグON
	THCONNECT th = *(THCONNECT*)pth;
	((THCONNECT*)pth)->bFlag = FLAG_ON;

	//stDomainからstURLのデータをstDataにダウンロード 
	char *pcState = th.pcState;
	int iDownSize = 0;
	BOOL bFailureFlag = FLAG_OFF;
	char *stData;
	char stDownloadRange[MAX_BUF];
	int iIP = ((THCONNECT*)pth)->iIP;
	char stIP[MAX_BUF];

	sprintf(stDownloadRange, "%d-%d", th.iDownstartbyte, th.iDownstartbyte + *th.piDownsize);
	sprintf(stIP, "%d.%d.%d.%d", (0x000000FF & iIP), (0x0000FF00 & iIP) >> 8, (0x00FF0000 & iIP) >> 16, (0xFF000000 & iIP) >> 24);
	__DownloadWithCURL(th.stData, th.stURL, &bFailureFlag,
		stDownloadRange,			//ダウンロード範囲を指定
		stIP,			//クライアントIPを指定
		th.stCookie,			//Cookieを指定
		"",			//その他のHTTPヘッダを指定
		FLAG_OFF,	//HTTPヘッダのダウンロードフラグ
		&iDownSize,	//ダウンロードサイズ返却
		""
		);

	if (bFailureFlag == FLAG_ON){
		*pcState = __DOWN_ERROR;
		return -1;
	}
	
	//仕事完了フラグ設定
	*pcState = __DOWN_FINISH;
		
	return 0;//コンパイラの警告を殺す
}

//__allocatedownloadの従属関数 thDownloadを呼び出す　※要stData解放
void __DownloadThreadWithCURL(char *stData, int iIP, char *stCookie, int iDownstartbyte, int *piDownsize, char *pcState){
	HANDLE Handle;
	THCONNECT th{ stData, iIP, __global__stTargetFileURL, stCookie, iDownstartbyte, piDownsize, pcState, FLAG_OFF };

	Handle = (HANDLE)_beginthreadex(NULL, 0, __thDonwnload, &th, 0, NULL);
	//th構造体が解放OKになるまで待機
	SLEEP_UNTIL_FLAG_ON(th.bFlag);

	CloseHandle(Handle);
}


/* __downloop */

//__gettotaldownsizeの従属関数 アダプタIPを返却
int __GetAdoptIP(){
	SOCKET_ADDRESS_LIST sList[MAX];
	SOCKET sock;
	WSADATA wsaData;
	ULONG iIP;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		__wprintf("WSAStartup failed\n");
		return 0;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		__wprintf("socket : %d\n", WSAGetLastError());
		return 0;
	}

	DWORD d = 0;
	int iIPNum = 0;
	int piIPlist[MAX];
	if (0 == WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0, sList, MAX, &d, NULL, NULL)){
		// iAddressCount個のデータが取得されている
		LOOP_I(sList->iAddressCount){
			//アドレスファミリがTCP/IPであるかどうかの確認
			if (sList->Address[i].lpSockaddr->sa_family == AF_INET){
				piIPlist[iIPNum] = ((sockaddr_in*)(sList->Address[i].lpSockaddr))->sin_addr.S_un.S_addr;
				iIPNum++;
			}
		}
	}
	closesocket(sock);
	WSACleanup();

	static int iNext = 0;
	if (iNext >= iIPNum)
		iNext = 0;

	iIP = piIPlist[iNext];
	iNext++;

	return iIP;
}
//__downloopの従属関数
void __GetFileSize(char *stCookie){

	//変数宣言
	static char stBuf[MAX*MAX];
	char *p;
	int s = 0, e = 0;
	int iTotalsize;
	int bFailureFlag = FLAG_OFF;
	int iDownSize = 0;

	//初期化
	__global__iFileSize = 0;

	//IP選択
	ULONG lIPclient;
	LOOP_I(RETRY_NUM){
		//リトライ回数が規定以上で終了
		if (i == RETRY_NUM - 1){
			__global__bDownloadFailure = FLAG_ON;
			return;
		}
		//ダウンロード
		__DownloadWithCURL(stBuf, __global__stTargetFileURL, &bFailureFlag,
			"0-0",			//ダウンロード範囲を指定
			"",			//クライアントIPを指定
			stCookie,			//Cookieを指定
			"",			//その他のHTTPヘッダを指定
			FLAG_ON,	//HTTPヘッダのダウンロードフラグ
			&iDownSize,	//ダウンロードサイズ返却
			""
			);

		if (bFailureFlag == FLAG_ON){
			Sleep(1);
			continue;
		}
		else
			break;
	}

	//ヘッダからトータルダウンロードサイズを分離
	p = strstr(stBuf, "Content-Range:");
	if (p == NULL){
		__global__bDownloadFailure = FLAG_ON;
		return;
	}
	*(strchr(p, '\n')) = '\0';

	//ダウンロードデータからファイルサイズ抽出
	p = strchr(p, '/')+1;
	if (strchr(p, '*') == NULL)
		iTotalsize = atoi(p);
	else
		iTotalsize = 0;

	__global__iFileSize = iTotalsize;
	__global__iFileCountNum = floor((iTotalsize - 1) / DOWNUNIT) + 1;

	if (__global__iFileSize <= 0)
		__global__bDownloadFailure = FLAG_ON;

	return;
}

//__downloopの従属関数 mallocするけど解放しない、再呼び出し時に前回分を開放する
char* __atost(int i){
	static char *stBuf = NULL;

	if (stBuf != NULL){
		free(stBuf);
		stBuf = NULL;
	}

	//数字の桁数＋1を計算
	int iSize;
	if (i < 1)
		i = 1;
	iSize = (int)floor(log10(i)) + 2;

	stBuf = (char*)malloc(iSize);
	sprintf(stBuf, "%d", i);

	return stBuf;
}

//__allocatedownloadの従属関数
void __GetDownPoint(int *piDownPoint, char *stState){
	
	*piDownPoint = -1;
	for (int i = 0; stState[i] != __DOWN_NA; i++){
		if (stState[i] == __DOWN_NOTFINISH || stState[i] == __DOWN_ERROR){
			*piDownPoint = i;
			break;
		}
	}

	return;
}

//__getsuitableadoptIPの従属関数
BOOL __CheckSuitableIP(int *piAdaptIP, char *stState, int iIP){
	int iCount = 0;
	BOOL bFlag;

	for(int i = 0; stState[i] != __DOWN_NA; i++){
		(piAdaptIP[i] == iIP) ? (stState[i] == __DOWN_DURING) ? iCount++ : NULL : NULL;
	}

	bFlag = (iCount < IP_DEVIDE_NUM) ? FLAG_ON : FLAG_OFF;

	return bFlag;
}

//__allocatedownloadの従属関数
void __GetSuitableAdoptIP(int *piIP, char *stState, int *piAdaptIP){
	SOCKET_ADDRESS_LIST sList[MAX];
	SOCKET sock;
	WSADATA wsaData;
	int piIPlist[MAX] = { 0 };
	int iIPNum = 0;
	*piIP = NULL;

	//アダプタIPリストを取得
	while (1){
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
			__wprintf("WSAStartup failed\n");
			Sleep(1);
			continue;
		}

		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			__wprintf("socket : %d\n", WSAGetLastError());
			Sleep(1);
			continue;
		}

		DWORD d = 0;
		if (0 == WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0, sList, MAX, &d, NULL, NULL)){
			// iAddressCount個のデータが取得されている
			LOOP_I(sList->iAddressCount){
				//アドレスファミリがTCP/IPであるかどうかの確認
				if (sList->Address[i].lpSockaddr->sa_family == AF_INET){
					piIPlist[iIPNum] = ((sockaddr_in*)(sList->Address[i].lpSockaddr))->sin_addr.S_un.S_addr;
					iIPNum++;
				}
			}
		}

		closesocket(sock);
		WSACleanup();

		break;
	}

	//指定した分割数未満のIPがあるかチェック
	static int iNext = 0;
	(iNext >= iIPNum) ? iNext = 0 : NULL;
	for (; iNext < iIPNum; iNext++){
		if (__CheckSuitableIP(piAdaptIP, stState, piIPlist[iNext]) == FLAG_ON)
			break;
	}

	//suitableIPがあればIPを返却
	(iNext < iIPNum) ? *piIP = piIPlist[iNext]: NULL;
	iNext++;

	//IPの個数をグローバル変数に格納
	__global__iAdaptNum = iIPNum;

	return;
}

//__downloopの従属関数
void __AllocateDownload(char *stCookie){
	//ダウンロード中スレッドの個数をithNumに格納
	int ithNum = 0;
	for (int i = 0; __global__stState[i] != __DOWN_NA; i++) (__global__stState[i] == __DOWN_DURING) ? ithNum++ : NULL;

	//次のダウンロード位置取得
	int iDownPoint = 0;
	__GetDownPoint(&iDownPoint, __global__stState);

	//ダウンロード領域がない場合リターン
	if (iDownPoint == -1) return;

	//適任のアダプタIPを選定
	int iIP = NULL;
	__GetSuitableAdoptIP(&iIP, __global__stState, __global__piAdaptIP);
	
	//適任のアダプタがない場合リターン
	if (iIP == NULL) return;

	//エラー領域の場合解放
	if(__global__stState[iDownPoint] == __DOWN_ERROR)
		free(__global__pstBigdata[iDownPoint]);

	//ダウンロード割り振り
	__global__stState[iDownPoint] = __DOWN_DURING;
	__global__piAdaptIP[iDownPoint] = iIP;
	__global__pstBigdata[iDownPoint] = (char*)malloc(__global__piDownSize[iDownPoint] + 2);
	__global__pbMemoryMallocFlag[iDownPoint] = FLAG_ON;
	__DownloadThreadWithCURL(__global__pstBigdata[iDownPoint], iIP, stCookie, iDownPoint * DOWNUNIT, &__global__piDownSize[iDownPoint], &__global__stState[iDownPoint]);

	return;
}

//__writetofileの従属関数
void __getfilesize(int *piFilesize, char *stFilepath){
	static FILE *fp = 0;
	static char stBuf[MAX] = { 0 };

	//ファイルオープン //前回と同じならオープン処理省略
	if (!STR_MATCH(stFilepath, stBuf) || (fp == 0)){
		//前回と違うファイルならクローズ処理
		if (fp != 0)
			fclose(fp);
		//新規ファイルオープン
		while ((fp = fopen(stFilepath, "rb+")) == NULL)
			Sleep(1);
		strcpy(stBuf, stFilepath);
	}

	//ファイル末尾にカーソルを移動
	fseek(fp, 0, SEEK_END);

	//ファイルサイズを取得
	*piFilesize = (int)floor(ftell(fp));

	return;
}

//__writetofileの従属関数
void __writetofileunit(char *stFilepath, char *stData, char cState, int iDownsize, int iWritepoint){
	//ダウンロード未完了なら戻る
	if (cState != __DOWN_FINISH)
		return;

	//ファイルを開く
	FILE *fp;
	while ((fp = fopen(stFilepath, "rb+")) == NULL)
		Sleep(1);

	//ファイルの書き込み位置を設定
	fseek(fp, iWritepoint*DOWNUNIT, SEEK_SET);

	//ファイルに書き込み
	fwrite(stData, iDownsize, 1, fp);
	free(stData);

	//解放フラグを記載
	__global__pbMemoryMallocFlag[iWritepoint] = FLAG_OFF;

	fclose(fp);
	return;
}

//__writetofileの従属関数 ダウンロードスピードと量を表示
void __DownSize(int iDownFinishSize, int iTotalDownloadFileSize){
	//時間とダウンロードサイズの格納領域確保
	static int piDownSize[STACK_SIZE + 1] = { 0 };
	static time_t piDownTime[STACK_SIZE + 1] = { 0 };

	//ダウンロードサイズに変化がない場合リターン
	if (piDownSize[0] == iDownFinishSize)
		return;

	//ダウンロードサイズが減少したらリセット
	(iDownFinishSize < piDownSize[0]) ? memset(piDownSize, 0, sizeof(int)*STACK_SIZE), memset(piDownTime, 0, sizeof(int)*STACK_SIZE) : NULL;

	piDownSize[0] = iDownFinishSize;
	piDownTime[0] = time(NULL);

	//グローバル変数に格納
	__global__iDownloadedSize = iDownFinishSize;

	return;
}

//__downloopの従属関数 ダウンロード済みから書き込めるものを書き込む
void __writetofile(){
	
	//ファイルの大きさ取得
	int iFilesize = 0;
	__getfilesize(&iFilesize, __global__stFilePath);

	//次の書き込みポイント設定
	int iWritepoint = floor(iFilesize / DOWNUNIT);

	//ファイルに書き込み //ファイルの書き込み位置設定
	__writetofileunit(__global__stFilePath, __global__pstBigdata[iWritepoint], __global__stState[iWritepoint], __global__piDownSize[iWritepoint], iWritepoint);

	//ダウンロードサイズを__global__iDownloadedSizeに書き込む
	__global__iDownloadedSize = (iWritepoint)*DOWNUNIT + __global__piDownSize[iWritepoint];

	//ファイルの終端まで書き込んだら終了依頼
	if (__global__stState[iWritepoint + 1] == __DOWN_NA && __global__stState[iWritepoint] == __DOWN_FINISH)
		__global__bStopDownload = FLAG_ON;

	return;
}

void __ClearArray(){
	free(__global__piAdaptIP);
	free(__global__piDownSize);
	free(__global__stState);
	free(__global__pbMemoryMallocFlag);
	free(__global__pstBigdata);
}

//__downloopの従属関数
void __InitArray(){
	//ダウンロードの状態・データ・サイズの格納領域確保
	__global__pstBigdata = (char**)malloc(sizeof(char*)*__global__iFileCountNum);
	__global__pbMemoryMallocFlag = (BOOL*)malloc(sizeof(BOOL)*__global__iFileCountNum);
	__global__stState = (char*)malloc(sizeof(char)*__global__iFileCountNum + 1);//+1は番兵
	__global__piDownSize = (int*)malloc(sizeof(int)*__global__iFileCountNum);
	__global__piAdaptIP = (int*)malloc(sizeof(int)*__global__iFileCountNum);

	//データ格納用配列配列初期化
	memset(__global__pstBigdata, 0, sizeof(char*)* __global__iFileCountNum);

	//MallocFlag初期化
	memset(__global__pbMemoryMallocFlag, FLAG_OFF, sizeof(BOOL*)* __global__iFileCountNum);

	//ダウンロード状態配列初期化
	int iDownloadedSize = 0, iDownloadedCountNum;
	__getfilesize(&iDownloadedSize, __global__stFilePath);
	iDownloadedCountNum = iDownloadedSize / DOWNUNIT;
	memset(__global__stState, __DOWN_NA, sizeof(char)* __global__iFileCountNum + 1);
	memset(__global__stState, __DOWN_NOTFINISH, sizeof(char)* __global__iFileCountNum);
	memset(__global__stState, __DOWN_FINISH, sizeof(char)* iDownloadedCountNum);

	//アダプタアドレスリスト初期化
	LOOP_I(__global__iFileCountNum)
		__global__piAdaptIP[i] = NULL;

	//ダウンロードサイズ初期化
	memset(__global__piDownSize, 0, sizeof(int)* __global__iFileCountNum);
	LOOP_I(__global__iFileCountNum - 1)
		__global__piDownSize[i] = DOWNUNIT;
	__global__piDownSize[__global__iFileCountNum - 1] = __global__iFileSize % DOWNUNIT;
}

//DownloadListのmainにURLがあったらFLAG_ONを返却
void __CheckMainURL(BOOL *bFlag){
	char *p;
	FILE *fp;
	char stBuf[MAX];

	*bFlag = FLAG_OFF;

	//Downloadリストをオープン
	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
	Sleep(1);
		
	//mainにURLの登録があるか確認
	while (fgets(stBuf, MAX, fp)){
		if (strstr(stBuf, "【main】")) 
			break;
	}
		
	//mainにURLの登録があったらフラグを立てる
	fgets(stBuf, MAX, fp);
	if (strstr(stBuf, "http") != NULL){
		*bFlag = FLAG_ON;
	}

	fclose(fp);
	return;
}

//__downloopの従属関数
void __EndDownAddNormalDuringSub(){
	if (__global__bSubflag == FLAG_OFF)
		return;

	//MainURLがあるかチェック
	BOOL bFlag;
	__CheckMainURL(&bFlag);

	//リスト追加があったらダウンロード終了フラグをON
	if (bFlag == FLAG_ON){
		__global__bListDelete = FLAG_OFF;
		__global__bStopDownload = FLAG_ON;
	}

	return;
}

//ダウンロード終了後の未開放のメモリなどの検査、削除
void __DeleteDownloadBacklog(){
	//ダウンロードスレッドが全て終了するまで待機
	while (1){
		for (int i = 0; __global__stState[i] != __DOWN_DURING; i++){
			if (__global__stState[i] == __DOWN_NA){
				return;
			}
		}
		Sleep(1);
	}

	//未開放のメモリがあれば解放
	for (int i = 0; i < __global__iFileCountNum; i++){
		if (__global__pbMemoryMallocFlag[i] == FLAG_ON){
			free(__global__pstBigdata[i]);
		}
	}

	return;
}

//WinMainの従属関数
void __Download(){
	//Cookieを取得
	char stCookie[MAX] = { 0 };
	__GetIECookie(stCookie);

	//ファイルサイズ取得 //終了座標計算
	__GetFileSize(stCookie);
	if (__global__bDownloadFailure == FLAG_ON)
		return;

	//ファイルダウンロード用配列初期化
	__InitArray();

	while (1){
		//仕事を割り振り　//ダウンロード実行 //アダプタ取得 //同時ダウンロード数の上限機能
		__AllocateDownload(stCookie);

		//書き込み処理 //FLAG_ONでファイル書き込み完了
		__writetofile();

		//サブリストダウンロード中にノーマルリストに追加があったら終了、削除フラグOFF
		__EndDownAddNormalDuringSub();

		//終了フラグが立っていたら終了処理
		if (__global__bStopDownload == FLAG_ON){
			__DeleteDownloadBacklog();
			break;
		}

		Sleep(1);
	}

	//ファイルダウンロード用配列解放
	__ClearArray();
	
	return;
}

#endif ____function_h_
