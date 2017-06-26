#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <stdio.h>
//#include "resource.h"
using namespace Gdiplus;
using namespace std;
#pragma comment(lib, "gdiplus")
#pragma warning(disable:4996)

const int WIDTH = 800;
const int HEIGHT = 600;

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("GdiPlusStart");

void OnPaint(HDC hdc, int ID, int x, int y);
void OnPaintA(HDC hdc, int ID, int x, int y, double alpha);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

HWND snapshot;
int window_x, window_y;
void play(UINT imsg, WPARAM wParam, LPARAM lParam, HDC MemDC);
void hijack();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND     hWnd;
	MSG		 msg;
	WNDCLASS WndClass;

	g_hInst = hInstance;

	ULONG_PTR gpToken;
	GdiplusStartupInput gpsi;
	if (GdiplusStartup(&gpToken, &gpsi, NULL) != Ok)
	{
		MessageBox(NULL, TEXT("GDI+ 라이브러리를 초기화할 수 없습니다."), TEXT("알림"), MB_OK);
		return 0;
	}


	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = L"Window Class Name";
	RegisterClass(&WndClass);
	hWnd = CreateWindow(
		L"Window Class Name",
		L"Window Title Name",
		WS_OVERLAPPEDWINDOW,
		GetSystemMetrics(SM_CXFULLSCREEN) / 2 - WIDTH/2 - 200,
		GetSystemMetrics(SM_CYFULLSCREEN) / 2 - HEIGHT/2,
		WIDTH,
		HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
		);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, MemDC;
	PAINTSTRUCT ps;

	HBITMAP hBit, OldBit;
	RECT crt;

	HBRUSH hBrush, oldBrush;


	switch (iMsg)
	{
	case WM_CREATE:
		hijack();
		SetTimer(hWnd, 1, 10, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		break;


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &crt);

		MemDC = CreateCompatibleDC(hdc);
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
		OldBit = (HBITMAP)SelectObject(MemDC, hBit);
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
		oldBrush = (HBRUSH)SelectObject(MemDC, hBrush);
		//hPen = CreatePen(PS_SOLID, 5, RGB(255, 255, 255));
		//oldPen = (HPEN)SelectObject(MemDC, hPen);

		//FillRect(MemDC, &crt, hBrush);

		play(iMsg, wParam, lParam, MemDC);



		BitBlt(hdc, 0, 0, crt.right, crt.bottom, MemDC, 0, 0, SRCCOPY);
		SelectObject(MemDC, OldBit);
		DeleteDC(MemDC);
		//SelectObject(MemDC, oldPen);
		//DeleteObject(hPen);
		//SelectObject(MemDC, oldBrush);
		//DeleteObject(hBrush);
		DeleteObject(hBit);

		EndPaint(hWnd, &ps);
		break;

	/*case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_KEYUP:
	case WM_KEYDOWN:
		hijack(iMsg, wParam, lParam);
		break;*/

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void OnPaint(HDC hdc, int ID, int x, int y)
{
	Graphics G(hdc);
	HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (!hResource)
		return;

	DWORD imageSize = SizeofResource(g_hInst, hResource);
	HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
	LPVOID pData = LockResource(hGlobal);

	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pBuffer = GlobalLock(hBuffer);
	CopyMemory(pBuffer, pData, imageSize);
	GlobalUnlock(hBuffer);

	IStream *pStream;
	HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);

	Image I(pStream);
	pStream->Release();
	if (I.GetLastStatus() != Ok) return;

	G.DrawImage(&I, x, y, I.GetWidth(), I.GetHeight());
}

void OnPaintA(HDC hdc, int ID, int x, int y, double alpha)
{
	Graphics G(hdc);
	HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (!hResource)
		return;

	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, (Gdiplus::REAL)alpha, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	ImageAttributes ImgAttr;
	ImgAttr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

	DWORD imageSize = SizeofResource(g_hInst, hResource);
	HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
	LPVOID pData = LockResource(hGlobal);

	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	LPVOID pBuffer = GlobalLock(hBuffer);
	CopyMemory(pBuffer, pData, imageSize);
	GlobalUnlock(hBuffer);

	IStream *pStream;
	HRESULT hr = CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);

	Image I(pStream);
	pStream->Release();
	if (I.GetLastStatus() != Ok) return;

	RectF destination(0, 0, (Gdiplus::REAL)I.GetWidth(), (Gdiplus::REAL)I.GetHeight());
	G.DrawImage(&I, destination, x, y, (Gdiplus::REAL)I.GetWidth(), (Gdiplus::REAL)I.GetHeight(), UnitPixel, &ImgAttr);
}


void hijack()
{
	snapshot = FindWindow(NULL,TEXT("Minesweeper"));
	if (!snapshot)
		exit(0);
}

bool colordiff(COLORREF a, COLORREF b, int k, int idx)
{
	/*int er[12] = {0, 10, 10, 13, 10, 10, 10, 10, 10, 20, 10};
	int eg[12] = {0, 10, 10, 20, 10, 10, 10, 10, 10, 13, 10};
	int eb[12] = {0, 10, 10, 20, 10, 10, 10, 10, 10, 20, 10};*/

	int dr = abs(GetRValue(a)-GetRValue(b));
	int dg = abs(GetGValue(a)-GetGValue(b));
	int db = abs(GetBValue(a)-GetBValue(b));
	if((dr<3 && dg<3) && db<3) return true;
	return false;
}
void play(UINT imsg, WPARAM wParam, LPARAM lParam, HDC MemDC)
{
	RECT rect;
	GetWindowRect(snapshot, &rect);
	window_x = rect.right - rect.left;
	window_y = rect.bottom - rect.top;
	/*window_x = 616;
	window_y = 409;
	
	
	//520 929 274 890

	rect.top = 100;//520;//0;
	rect.bottom = 300;//929;//window_y;
	rect.left = 100;//274;//0;
	rect.right = 300;//890;//window_x;*/
	//printf("%d %d %d %d\n",rect.top, rect.bottom, rect.left, rect.right);fflush(stdout);
	
	
	int x1, y1, x2, y2, w, h;

	// get screen dimensions
	x1  = rect.left;//GetSystemMetrics(SM_XVIRTUALSCREEN);
	y1  = rect.top;//GetSystemMetrics(SM_YVIRTUALSCREEN);
	x2  = rect.right;//GetSystemMetrics(SM_CXVIRTUALSCREEN);
	y2  = rect.bottom;//GetSystemMetrics(SM_CYVIRTUALSCREEN);
	w   = x2 - x1;
	h   = y2 - y1;

	// copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC     = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);


	const COLORREF num_clr[12]={RGB(0,0,0), RGB(0,0,255), RGB(0,128,0), RGB(255,0,0), RGB(0,0,128), RGB(128,0,0), RGB(0,128,128), RGB(0,0,0), RGB(128,128,128), RGB(192,192,192), RGB(0, 0, 0), RGB(0,0,0)};
	const int num_x[12] = {0, 8, 9, 8, 8, 8, 8, 4, 8, 2, 12, 5};
	const int num_y[12] = {0, 8, 8, 13, 8, 8, 8, 4, 8, 2, 12, 13};
	int board[16][30] = {};
	int delta = 1;
	COLORREF clr;
	/*for(int i=99; i<99 + 16*16+1; i+=delta) {
		for(int j=14; j<14 + 16*30+1; j+=delta) {
			clr = GetPixel(hDC, j, i);
			SetPixel(MemDC, j, i, clr);
			//printf("%d %d %d\n",GetRValue(clr),GetGValue(clr),GetBValue(clr)); fflush(stdout);
		}
	}*/

	for(int i=0; i<16; i++) {
		for(int j=0; j<30; j++) {
			int cnt = 0;
			vector<int> mem;
			for(int k=1; k<=8; k++) {
				int y = 99 + i*16 + num_y[k];
				int x = 14 + j*16 + num_x[k];
				clr = GetPixel(hDC, x, y);
				SetPixel(MemDC, x, y, clr);
				if(colordiff(num_clr[k], GetPixel(hDC, x, y), k, i+j)) {
					cnt++;
					mem.push_back(k);
				}
			}
			if(cnt == 1){
				//printf("%d  ",mem[0]); fflush(stdout);
				board[i][j] = mem[0];
			} else if(cnt>1) {
				//printf("%d%d ",mem[0],mem[1]); fflush(stdout);
			} else {
				cnt = 0;
				vector<int> mem;
				for(int k=9; k<=11; k++) {
					int y = 99 + i*16 + num_y[k];
					int x = 14 + j*16 + num_x[k];
					clr = GetPixel(hDC, x, y);
					SetPixel(MemDC, x, y, clr);
					if(colordiff(num_clr[k], GetPixel(hDC, x, y), k, i+j)) {
						cnt++;
						mem.push_back(k);
					}
				}
				if(cnt==1 && mem[0]==9) {
					//printf("   "); fflush(stdout);
					board[i][j] = 0;
				} else if(cnt==3 && mem[1]==10) {
					//printf("*  "); fflush(stdout);
				} else if(cnt==2 && mem[1]==11) {
					//printf("|> "); fflush(stdout);
					board[i][j] = -2;
				} else {
					//printf("!%d ",cnt); fflush(stdout);
					board[i][j] = -1;
				}
			}
		}
		//printf("\n"); fflush(stdout);
	}
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);

	//calc & click
	int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	int dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};
	bool flag = false;
	for(int i=0; i<16; i++) {
		for(int j=0; j<30; j++) {
			if(board[i][j]>0) {
				vector<pair<int, int> > target;
				for(int k = 0; k<8; k++) {
					int nx = j + dx[k];
					int ny = i + dy[k];
					if(nx < 0 || ny < 0 || nx >= 30 || ny >= 16)
						continue;
					if(board[ny][nx] < 0) {
						target.push_back(make_pair(nx, ny));
					}
				}
				if((int)target.size() == board[i][j]) {
					for(int k=0; k<(int)target.size(); k++) {
						if(board[target[k].second][target[k].first] != -2) {
							board[target[k].second][target[k].first] = -2;
							int tx = 14 + target[k].first*16 + 8;
							int ty = 99-48 + target[k].second*16 + 8;
							PostMessage(snapshot, WM_RBUTTONDOWN, 0, (LPARAM)(DWORD)MAKELONG(tx, ty));
							PostMessage(snapshot, WM_RBUTTONUP, 0, (LPARAM)(DWORD)MAKELONG(tx, ty));
							//click!
						}
					}
				}
			}
		}
	}

	/*for(int i=0; i<16; i++){
		for(int j=0; j<30; j++) {
			if(board[i][j]!=-1) 
				printf("%2d ",board[i][j]);
			else
				printf("!! ");
		}
		printf("\n");
	}
	fflush(stdout);*/
	int d;
	//scanf("%d",&d);
	for(int lev=0; lev<10; lev++){
		for(int i=0; i<16; i++) {
			for(int j=0; j<30; j++) {
				if(board[i][j]>0) {
					int cnt = 0;
					for(int k = 0; k<8; k++) {
						int nx = j + dx[k];
						int ny = i + dy[k];
						if(nx < 0 || ny < 0 || nx >= 30 || ny >= 16) continue;
						if(board[ny][nx]==-2)
							cnt++;
					}
					if(cnt == board[i][j]) {
						flag = true;
						for(int k = 0; k<8; k++) {
							int nx = j + dx[k];
							int ny = i + dy[k];
							if(board[ny][nx]==-1) {
								int tx = 14 + nx*16 + 8;
								int ty = 99-48 + ny*16 + 8;
								PostMessage(snapshot, WM_LBUTTONDOWN, 0, (LPARAM)(DWORD)MAKELONG(tx, ty));
								PostMessage(snapshot, WM_LBUTTONUP, 0, (LPARAM)(DWORD)MAKELONG(tx, ty));
								//click!
							}
						}
					}
				}
			}
		}
	}

	//scanf("%d",&d);

	if(!flag) {
		//random click on -1
	}

	//send click event
	//PostMessage(snapshot, imsg, wParam, lParam);
}
