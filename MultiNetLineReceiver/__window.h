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

/* �֐��錾 */

//__geturl�̏]���֐� �w�蕶����stBuf����URL���擾����stURL�Ɋi�[(����������ϐ������Ă��Ă��ǂ�) //__geturl�̏]���֐� �N���b�v�{�[�h�̕������stClip�Ɋi�[
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

//__wndproc�̏]���֐�
int __GetURL(char *stURL){
	//�N���b�v�{�[�h�̕�����擾
	char stBuf[MAX];
	char *p;
	
	//�N���b�v�{�[�h�̓��e��stBuf�ɃR�s�[
	if (__GetClipboard(stBuf, MAX) == -1)
		return -1;

	//URL���Ȃ����return
	if ((p = strstr(stBuf, "http")) == NULL){
		__wprintf("error: __geturl");
		return -1;
	}

	//URL������
	char stUrlbuf[MAX];
	strcpy(stUrlbuf, p);

	//URL�Ɏg�pNG�����������NULL������ǉ����ďI��
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

//__wndproc�̏]���֐� �w�肵��hWnd�̃t�H���g�������̂ɂ���
void __SendFont(HWND hWnd){
	static HFONT hFont = NULL;

	//���߂Ă̎��s�̏ꍇ�t�H���g���K�肷��
	if (hFont == NULL){
		while (1){
			hFont = CreateFont(12,       //�t�H���g����
				0,                      //�������i�����Ɠ����j
				0,                  //�e�L�X�g�̊p�x
				0,                      //�x�[�X���C���Ƃ����Ƃ̊p�x
				FW_REGULAR,             //�t�H���g�̏d���i�����j
				FALSE,                  //�C�^���b�N��
				FALSE,                  //�A���_�[���C��
				FALSE,                  //�ł�������
				SHIFTJIS_CHARSET,       //�����Z�b�g
				OUT_DEFAULT_PRECIS,     //�o�͐��x
				CLIP_DEFAULT_PRECIS,    //�N���b�s���O���x
				PROOF_QUALITY,          //�o�͕i��
				FIXED_PITCH | FF_MODERN,  //�s�b�`�ƃt�@�~���[
				(LPCTSTR)"�l�r �o�S�V�b�N");                  //���̖�

			if (hFont != NULL)
				break;
			Sleep(1);
		}
	}

	//�w�肵���n���h���Ƀt�H���g��ݒ肷��
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
}

void __GetRestFileNum(){

	FILE *fp;
	char stBuf[MAX];
	int iRestMainFileNum = 0, iRestSubFileNum = 0;

	
	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//main�̈ꗗ�܂ňړ�
	while (fgets(stBuf, MAX - 1, fp)){
		if (strstr(stBuf, "�ymain�z") != NULL)
			break;
	}

	//�󔒕����܂ł̍s�����J�E���g
	for (iRestMainFileNum = 0; fgets(stBuf, MAX - 1, fp); iRestMainFileNum++){
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
	}

	//sub�̈ꗗ�܂ňړ�
	while (fgets(stBuf, MAX - 1, fp)){
		if (strstr(stBuf, "�ysub�z") != NULL)
			break;
	}

	//�󔒕����܂ł̍s�����J�E���g
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

	//�N���b�v�{�[�h����URL���擾
	if (__GetURL(stURL) == -1)
		return;

	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//�Ώۂ�URL�܂ł̏���ۊ�
	//�_�E�����[�hURL������
	char *p;
	char *stList = (char*)malloc(MAX);
	memset(stList, 0, MAX);
	for (iWriteLine = 0; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//���s�����폜
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
		//����������Break
		if (strstr(stBuf, "�ymain�z"))
			break;
	}

	//�󔒍s�܂ňړ�
	for (; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//����������Break
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
		//���s�����폜
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//URL�����X�g�ɑ}��
	sprintf(stList, "%s%s\n\n", stList, stURL);

	//�_�E�����[�hURL�ȍ~�̍s�����擾
	for (; fgets(stBuf, MAX, fp); iWriteLine++){
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//URL��ǉ��������X�g���쐬
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//�I������
	free(stList);
	fclose(fp);

	return;
}

void __AddSubList(){
	FILE *fp;
	int iWriteLine;
	char stBuf[MAX];
	char stURL[MAX];

	//�N���b�v�{�[�h����URL���擾
	if (__GetURL(stURL) == -1)
		return;

	while ((fp = fopen(DOWNLOAD_LIST, "r")) == NULL)
		Sleep(1);

	//�Ώۂ�URL�܂ł̏���ۊ�
	//�_�E�����[�hURL������
	char *p;
	char *stList = (char*)malloc(MAX);
	memset(stList, 0, MAX);
	for (iWriteLine = 0; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//���s�����폜
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
		//����������Break
		if (strstr(stBuf, "�ysub�z"))
			break;
	}

	//�󔒍s�܂ňړ�
	for (; fgets(stBuf, MAX - 1, fp); iWriteLine++){
		//����������Break
		if (stBuf[0] == '\0' || stBuf[0] == '\n')
			break;
		//���s�����폜
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}

	//URL�����X�g�ɑ}��
	sprintf(stList, "%s%s\n", stList, stURL);

	//�_�E�����[�hURL�ȍ~�̍s�����擾
	for (; fgets(stBuf, MAX, fp); iWriteLine++){
		p = strchr(stBuf, '\n');
		if (p != NULL)
			*p = '\0';
		stList = (char*)realloc(stList, (iWriteLine + 1) * MAX);
		sprintf(stList, "%s%s\n", stList, stBuf);
	}
	fclose(fp);

	//URL��ǉ��������X�g���쐬
	while ((fp = fopen(DOWNLOAD_LIST, "w")) == NULL)
		Sleep(1);
	fprintf(fp, "%s", stList);

	//�I������
	free(stList);
	fclose(fp);

	return;
}

//�ȈՃE�B���h�E��wndproc
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
	static char pstButtonName[][MAX] = { "ADD URL", "ADD SUBLIST", "�ҏW" };
	double dPercent = 0;

	switch (msg) {
	case WM_PAINT:
		hDC = BeginPaint(__global__hDraw, &ps);

		//�o�b�t�@���m��
		hDCBuf = CreateCompatibleDC(hDC);
		hBitmapBuf = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
		SelectObject(hDCBuf, hBitmapBuf);
		DeleteObject(hBitmapBuf);

		//�`��t�H���g�ݒ�
		hFont = __SetFont((LPCTSTR)"�l�r �o�S�V�b�N", 12, 0);
		while ((hFontOld = (HFONT)SelectObject(hDCBuf, hFont)) == NULL)
			Sleep(1);

		//�c���m�F
		__GetRestFileNum();

		//���x�ƃ_�E�����[�h���`��
		(__global__iFileSize != 0) ? dPercent = (double)__global__iDownloadedSize / __global__iFileSize * 100 : NULL;
		sprintf(stBuf,
			"���x: %.1fKB/s\n\
			����: %.1f%%(%.0fKB / %.0fKB)\n\
			�c��: %d(��), %d(��)\n\
			���: %d��\n\
			����: %s\n\
			URL: %s",
			__global__dSpeed,
			dPercent, (double)__global__iDownloadedSize / 1024, (double)__global__iFileSize / 1024,
			__global__iRestMainFileNum, __global__iRestSubFileNum,
			__global__iAdaptNum,
			__global__stFileName,
			__global__stTargetFileURL);

		//�`��̈�擾
		GetClientRect(__global__hDraw, &rc);

		//�`��
		SetBkMode(hDCBuf, TRANSPARENT);
		SetTextColor(hDCBuf, RGB(255, 255, 255));
		DrawText(hDCBuf, stBuf, -1, &rc, DT_EDITCONTROL | DT_WORDBREAK | DT_WORD_ELLIPSIS);
		BitBlt(hDC, 0, 0, rc.right, rc.bottom, hDCBuf, 0, 0, SRCCOPY);

		//�`��̈���
		DeleteDC(hDCBuf);
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
		EndPaint(__global__hDraw, &ps);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wp);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case ID_BUTTON1:
			/* ADD URL�{�^���̓���*/
			__AddMainList();
			break;
		case ID_BUTTON2:
			/* ADD SUBLIST�{�^���̓���*/
			__AddSubList();
			break;
		case ID_BUTTON3:
			/* �ҏW�{�^���̓��� */
			ShellExecute(NULL, "open", DOWNLOAD_LIST, NULL, NULL, SW_SHOWNORMAL);
			break;
		default:
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;
	case WM_CREATE:
		//�N���C�A���g�̗̈���擾
		GetClientRect(hWnd, &rc);

		//������`��̈�ݒ�
		__global__hDraw = CreateWindow(TEXT("STATIC"), TEXT(NULL), SS_SIMPLE | WS_CHILD | WS_VISIBLE | WS_BORDER,
			rc.right / 2,
			0,
			rc.right / 2,
			rc.bottom,
			hWnd, (HMENU)ID_DRAW1, NULL, NULL);
		
		//�{�^���z�u����
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

//__thwindow�̏]���֐� �Z���^�[���W��ԋp����
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

//�ȈՃE�B���h�E
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

//�ȈՃE�B���h�E���X���b�h�ŌĂяo��
HWND __CreateWindow(void *hInstance){
	HANDLE thWindow;
	thWindow = (HANDLE)_beginthreadex(NULL, 0, __thwindow, hInstance, 0, NULL);
	while (__global_hWndWindow == 0)
		Sleep(100);

	return __global_hWndWindow;
}

#endif ____window_h_