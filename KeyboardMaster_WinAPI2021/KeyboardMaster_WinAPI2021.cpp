// KeyboardMaster_WinAPI2021.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "KeyboardMaster_WinAPI2021.h"
#include <ctime>
#include <vector>
#include <windowsx.h>
#include <commdlg.h>
using namespace std;
const int squareSize = 25;
const int minTime = 300;
const int maxTime = 1000;
const int fallDistMin = 2;
const int fallDistMax = 3;
const int windowLimit = 100;
int mainHeight;
int mainWidth;
#define BLACK RGB(0,0,0)
#define WHITE RGB(255,255,255)
#define RAND(MIN,MAX) (rand()%(MAX-MIN+1)+MIN)
#define FALLTIME 40
#define WINDOWTIMER 7
#define FALLTIMER 8
#define DEFAULT_BG 0
#define TILED_BG 1
#define STREACHED_BG 2
#define COLOR_BG 3


#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

class keyWindow
{
public:
	HWND hWnd;
	char letter;
	int speed;
	int left;
	int top;

	keyWindow(HWND hWnd, char letter, int speed, int left, int top = 0) : hWnd(hWnd), letter(letter), speed(speed), left(left), top(top) {}

};
vector<keyWindow> windows;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_KEYBOARDMASTERWINAPI2021, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYBOARDMASTERWINAPI2021));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

int findWindow(HWND hWnd, int& windowsCount)
{
	for (int i = 0; i < windowsCount; ++i)
	{
		if (windows[i].hWnd == hWnd) return i;
	}

	return 0;
}

void refreshTitle(HWND hWnd, int missed, int wrong)
{
	TCHAR s[256];
	_stprintf_s(s, 256, _T("%s, Missed: %d, Wrong keys: %d"), szTitle, missed, wrong);
	SetWindowText(hWnd, s);
}

void destroyTile(int i, int& missed, int wrong, int& windowsCount)
{
	if (i < 0 || i >= (int)windows.size()-1) return;
	DestroyWindow(windows[i].hWnd);
	windows.erase(windows.begin() + i);
	--windowsCount;
	refreshTitle(GetParent(windows[i].hWnd), missed, wrong);
}

void moveChildWindow(HWND hWndMain, int i, int& missed, int& wrong, int& windowsCount)
{
	windows[i].top += windows[i].speed;
	MoveWindow(windows[i].hWnd, windows[i].left, windows[i].top, squareSize, squareSize, TRUE);


	if (mainHeight < windows[i].top + squareSize)
	{
		++missed;
		destroyTile(i, missed, wrong, windowsCount);
	}
	else InvalidateRect(windows[i].hWnd, NULL, true);
}

void generateTile(HWND hWnd, int& windowsCount)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	int l = RAND(0, rc.right - rc.left - squareSize + 1);

	HWND newhWnd = CreateWindowW(szWindowClass, 0, WS_CHILDWINDOW | WS_VISIBLE, l, 0, squareSize, squareSize, hWnd, nullptr, hInst, nullptr);

	keyWindow win(newhWnd, RAND('a', 'z'), RAND(fallDistMin, fallDistMax), l);
	windows.push_back(win);

	windowsCount++;

	SetTimer(hWnd, WINDOWTIMER, RAND(minTime, maxTime), NULL);
}

void deleteLowestTile(char c, int& missed, int& wrong, int& windowsCount)
{
	c = tolower(c);
	if (c < 'a' || c>'z') return;

	int ind = -1, maxDist = 0;

	for (unsigned int i = 0; i < windows.size(); ++i)
	{
		if (windows[i].letter == c && windows[i].top > maxDist)
		{
			maxDist = windows[i].top;
			ind = i;
		}
	}

	if (ind >= 0 && ind<(int)windows.size()) destroyTile(ind, missed, wrong, windowsCount);
	else
	{
		++wrong;
		refreshTitle(GetParent(windows[0].hWnd), missed, wrong);
	}
	
}

void chpause(HWND hWnd, bool pnp)
{
	if (pnp)
	{
		KillTimer(hWnd, WINDOWTIMER);
		KillTimer(hWnd, FALLTIMER);
	}
	else
	{
		SetTimer(hWnd, WINDOWTIMER, RAND(minTime, maxTime), NULL);
		SetTimer(hWnd, FALLTIMER, FALLTIME, NULL);
	}
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMASTERWINAPI2021));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = GetSysColorBrush(COLOR_INACTIVECAPTION);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_KEYBOARDMASTERWINAPI2021);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	mainWidth = (rc.left + rc.right + 1) / 2;
	mainHeight = (rc.top + rc.bottom + 1) / 2;


	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_CLIPCHILDREN,
		mainWidth / 2, mainHeight / 2, mainWidth, mainHeight, nullptr, nullptr, hInstance, nullptr);


	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


	GetClientRect(hWnd, &rc);
	mainHeight = rc.bottom;

	if (!hWnd)
	{
		return FALSE;
	}

	srand((unsigned int)time(NULL));

	refreshTitle(hWnd, 0, 0);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int wrong = 0;
	static int windowsCount = 0;
	static int missed = 0;
	static bool pause = false;
	static int bg = DEFAULT_BG;
	static bool bitmapPresent = false;
	static OPENFILENAME ofn;
	static char szFile[260];
	static COLORREF col = GetSysColor(COLOR_INACTIVECAPTION);

	switch (message)
	{
	case WM_SETFOCUS:
	case WM_CREATE:
		if (GetParent(hWnd) == nullptr)
			chpause(hWnd, false);
		break;

	case WM_KILLFOCUS:
		if (GetParent(hWnd) == nullptr)
			chpause(hWnd, true);
		break;

	case WM_ERASEBKGND: return 1;


	case WM_TIMER:
		if (wParam == WINDOWTIMER && windowsCount < windowLimit)
			generateTile(hWnd, windowsCount);

		if (wParam == FALLTIMER)
		{
			for (unsigned int i = 0; i < windows.size(); ++i)
				moveChildWindow(hWnd, i, missed, wrong, windowsCount);
		}
		break;

	case WM_PAINT:
		if (GetParent(hWnd) != NULL)
		{
			int i = findWindow(hWnd, windowsCount);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			TCHAR s[2];
			s[0] = windows[i].letter;
			s[1] = '\0';
			RECT rc;
			GetClientRect(hWnd, &rc);

			HBRUSH brush = CreateSolidBrush(BLACK);
			FillRect(hdc, &ps.rcPaint, brush);
			SetTextColor(hdc, WHITE);
			SetBkColor(hdc, BLACK);
			DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			DeleteObject(brush);
			EndPaint(hWnd, &ps);
			
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			if (bg == COLOR_BG || bg == DEFAULT_BG)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				HBRUSH brush = CreateSolidBrush(col);	
				FillRect(hdc, &ps.rcPaint, brush);
				DeleteObject(brush);
			}
			else
			{
				HBITMAP bitmap = (HBITMAP)LoadImage(NULL, ofn.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				HDC memDC = CreateCompatibleDC(hdc);
				HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
				BitBlt(hdc, 0, 0, 0, 0, memDC, 0, 0, SRCCOPY);		
				
				BITMAP* bm = new BITMAP();
				GetObjectW(bitmap, sizeof(BITMAP), bm);

				if(bg == STREACHED_BG)
					StretchBlt(hdc, 0, 0, mainWidth, mainHeight, memDC, 0, 0, bm->bmWidth, bm->bmHeight, SRCCOPY);
				else
				{
					RECT rc;
					GetClientRect(hWnd, &rc);
					HBRUSH brush = CreatePatternBrush(bitmap);
					FillRect(hdc, &ps.rcPaint, brush);
					DeleteObject(brush);
				}
				SelectObject(memDC, oldBitmap);
				DeleteObject(bitmap);
				DeleteObject(bm);
				DeleteDC(memDC);	
			}

			EndPaint(hWnd, &ps);
		}
		break;

	case WM_CHAR:
		if(GetParent(hWnd) == nullptr) deleteLowestTile((char)wParam, missed, wrong, windowsCount);
		break;

	case WM_CONTEXTMENU:
		if (GetParent(hWnd) == nullptr)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			HMENU hMenu = CreatePopupMenu();
			UINT flags = MF_BYPOSITION | MF_STRING;
			UINT flagsB = flags;
			if (!bitmapPresent) flagsB = flags | MF_GRAYED;
			InsertMenu(hMenu, 0, flags, ID_COLOR, _T("Color...\tCtrl+C"));
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(hMenu, 0, flagsB, ID_STREACH, _T("Streach\tCtrl+S"));
			InsertMenu(hMenu, 0, flagsB, ID_TILE, _T("Tile\tCtrl+T"));
			InsertMenu(hMenu, 0, flags, ID_BITMAP, _T("Bitmap\tCtrl+B"));
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, xPos, yPos, 0, hWnd, NULL);
		}

		break;


	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);

		switch (wmId)
		{
		case ID_BITMAP:
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = (LPWSTR)szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"Bitmap(*.bmp)\0*.bmp\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn))
				bitmapPresent = true;
			else break;
			
		case ID_TILE:
			if (!bitmapPresent) break;
			bg = TILED_BG;
			InvalidateRect(hWnd, NULL, true);
			break;

		case ID_STREACH:
			if (!bitmapPresent) break;
			bg = STREACHED_BG;
			InvalidateRect(hWnd, NULL, true);
			break;
			
		case ID_COLOR:
			CHOOSECOLOR cc;
			static COLORREF tab[16];
			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hWnd;
			cc.lpCustColors = (LPDWORD)tab;
			cc.rgbResult = col;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			if (ChooseColor(&cc)) bg = COLOR_BG;
			col = cc.rgbResult;
			InvalidateRect(hWnd, NULL, true);
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case ID_FILE_NEWGAME:
			for (int i = windows.size() - 1; i >= 0; --i)
				DestroyWindow(windows[i].hWnd);
			windows.erase(windows.begin(), windows.end());
			missed = 0;
			windowsCount = 0;
			refreshTitle(hWnd, missed, wrong);
			break;

		case ID_FILE_PAUSE:
			chpause(hWnd, !pause);
			pause = !pause;
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_DESTROY:
		if (GetParent(hWnd) == NULL) PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
