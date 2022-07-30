#pragma once
#include "TerminalHeader.h"

/*
gre(TAB) FAUZAN --> Not Working
*/

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPCSTR szCmdLine,
	int iCmdShow
)
{
	TCHAR szAppName[] = {_T("MyTerminal")};
	MSG msg;
	WNDCLASSEX wnd;
	HWND hwnd;

	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInstance;
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.lpfnWndProc = WndProc;
	wnd.lpszClassName = szAppName;
	wnd.lpszMenuName = NULL;
	wnd.style = CS_HREDRAW | CS_VREDRAW;

	if (RegisterClassEx(&wnd) == NULL)
	{
		MessageBox(NULL, _T("Register Class Failed!!!"), _T("Error"), MB_OK);
		return 0;
	}

	hwnd = CreateWindow(
		szAppName,
		_T("MyTerminal"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if ( NULL == hwnd)
	{
		MessageBox(NULL, _T("Create Window Failed!!!"), _T("Error"), MB_OK);
		return;
	}

	ShowWindow(hwnd, SW_MAXIMIZE);
	UpdateWindow(hwnd);

	while ( GetMessage(&msg, NULL, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static TerminalEditInfo objTI = {0};

	HDC hdc;
	TCHAR szTitle[MAX_SIZE] = { 0 };
	int iLen=0;

	objTI.wParam = wParam;
	objTI.lParam = lParam;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		memset(objTI.szMainBuffer, 0x00, MAX_SIZE);
		memset(objTI.szCommandBuffer, 0x00, MAX_SIZE);

		objTI.szMainBuffer[0] = _T('#');
		objTI.szCommandBuffer[0] = _T('#');
		objTI.iCharPos = 1;
		objTI.iCaretPos = 1;
		objTI.hwnd = hwnd;

		//objTI.pchOutputBuffer = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, BUFFERSIZE*sizeof(TCHAR));
		
		ShowScrollBar(hwnd, SB_HORZ, TRUE);
		ShowScrollBar(hwnd, SB_VERT, TRUE);

		objTI.hFont = CreateFont(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Consolas"));
#ifndef _DEBUG
		GetCurrentDirectory(sizeof(szTitle), szTitle);
		SetWindowText(hwnd, szTitle);
#else
		SetCurrentDirectory(_T("C:\\Users\\fauza\\Desktop\\testing\\"));
		SetWindowText(hwnd, _T("C:\\Users\\fauza\\Desktop\\testing\\"));
#endif
	}break;//WM_CREATE end

	case WM_SETFOCUS:
	{
		hdc = GetDC(hwnd);
		SelectObject(hdc, objTI.hFont);
		iLen = GetWidthOfString(&objTI, objTI.szMainBuffer);
		ReleaseDC(hwnd, hdc);
		CreateCaret(hwnd, (HBITMAP)NULL, 0, 18);
		objTI.iCaretPosLen = iLen;
		objTI.iCaretPos = _tcslen(objTI.szMainBuffer);
		objTI.iCharPos = objTI.iCaretPos;
		SetCaretPos(objTI.iCaretPosLen, 0);
		ShowCaret(hwnd);
	}break; //WM_SETFOCUS end

	case WM_KILLFOCUS:
		HideCaret(hwnd);
		DestroyCaret();
		break;//WM_KILLFOCUS end

	case WM_PAINT:
	{
		int i = _tcslen(objTI.szCommandBuffer);
		_stprintf(szTitle, _T("iCaretPos=%d , iCharPos=%d , iSelectStart=%d, iSelectEnd=%d , Buffer(%d) : %s "), objTI.iCaretPos, objTI.iCharPos, objTI.iSelectStart, objTI.iSelectEnd, _tcslen(objTI.szCommandBuffer), objTI.szCommandBuffer);
		iLen = _tcslen(szTitle);
		GetClientRect(hwnd, &objTI.rc);
		hdc = BeginPaint(hwnd, &objTI.ps);
		SetBkColor(hdc, RGB(0,0,0));
		SetTextColor(hdc, RGB(0, 255, 0));
		SelectObject(hdc, objTI.hFont);
		DrawText(hdc, objTI.szMainBuffer, -1, &objTI.rc, DT_SINGLELINE | DT_LEFT);
		
	#if ENABLE_UI_DEBUG
		DrawText(hdc, szTitle, -1, &objTI.rc, DT_SINGLELINE | DT_VCENTER);
	#endif
		EndPaint(hwnd, &objTI.ps);

		/*
		HideCaret(hwnd);
		objTI.iCaretPosLen = GetWidthOfString(&objTI, objTI.szMainBuffer);
		SetCaretPos(objTI.iCaretPosLen, 0);
		ShowCaret(hwnd);
		*/

	}break;//WM_PAINT end

	case WM_CHAR:
	{
		if (9 == wParam)
			return;
		HideCaret(hwnd);
		HandleCharKeyPress(&objTI);
		ShowCaret(hwnd);
	}break;//WM_CHAR

	case WM_KEYDOWN:
		HandleKeyDownMessage(&objTI);
		break;
	case WM_DESTROY:
		//GlobalFree(objTI.pchOutputBuffer);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
