#pragma once
#include "TerminalHeader.h"

int GetWidthOfString(TerminalEditInfo* pTI, TCHAR* pcStart1)
{
	// get charecter width of string
	HDC hdc;
	int j = 0;
	int iLen = 0;
	int iTotalLen = 0;
	hdc = GetDC(pTI->hwnd);

	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(0, 255, 0));
	SelectObject(hdc, pTI->hFont);
	int bl = (int)_tcslen(pcStart1);
	while (j < bl)
	{
		GetCharWidth32(hdc, pcStart1[j], pcStart1[j], &iLen);
		iTotalLen += iLen;
		j++;
	}
	ReleaseDC(pTI->hwnd, hdc);
	return iTotalLen;
}

int GetWidthOfChar(TerminalEditInfo* pTI, TCHAR ch)
{
	HDC hdc;
	int iLen = 0;

	hdc = GetDC(pTI->hwnd);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(0, 255, 0));
	SelectObject(hdc, pTI->hFont);

	GetCharWidth32(hdc, ch, ch, &iLen);

	ReleaseDC(pTI->hwnd, hdc);
	return iLen;
}

void HandleKeyDownMessage(TerminalEditInfo* pTI)
{
	int iLen=0;
	TCHAR *pcStart = NULL;
	TCHAR *pcEnd = NULL;
	HideCaret(pTI->hwnd);
	switch (pTI->wParam)
	{
		case VK_TAB:
		{
			Handle_TAB_Key(pTI);
		}break;
		case VK_DELETE:
		{
			HideCaret(pTI->hwnd);
			Handle_Delete_Key(pTI);
			ShowCaret(pTI->hwnd);
		}break;
		case VK_LEFT:
		{
			MoveCaretLeftRight(pTI, 0);
		}break;
		case VK_RIGHT:
		{
			MoveCaretLeftRight(pTI, 1);
		}break;
		case  VK_HOME:
		{
			MoveCaretLeftRight(pTI, 2);
		}break;
		case  VK_END:
		{
			MoveCaretLeftRight(pTI, 3);
		}break;

		case VK_BACK:
			if (1 != pTI->iCharPos && GetKeyState(VK_CONTROL) & 0x8000)
			{
				pcEnd = &pTI->szMainBuffer[pTI->iCaretPos];
				
				while (pTI->iCaretPos >= 1)
				{
					pTI->iCaretPos--;
					if (
						((pTI->szCommandBuffer[pTI->iCaretPos] >= 65) && (pTI->szCommandBuffer[pTI->iCaretPos] <= 90)) ||
						((pTI->szCommandBuffer[pTI->iCaretPos] >= 97) && (pTI->szCommandBuffer[pTI->iCaretPos] <= 122))
						)
					{
						iLen += GetWidthOfChar(pTI, pTI->szCommandBuffer[pTI->iCaretPos]);
						pTI->szCommandBuffer[pTI->iCaretPos] = _T('\0');
					}
					else
						break;
				}
				pTI->iCaretPos++;
				pTI->iCharPos = pTI->iCaretPos;
				
				pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
				_tcscpy(pcStart, pcEnd);
				
				pTI->iCaretPosLen -= iLen;

				BufferCopyCommandToMain(pTI);
			}
			break;

		/*
		case VK_UP:
		{
			Handle_UP_Key(hwnd);
		}break;
		case VK_DOWN:
		{
			Handle_Down_Key(hwnd);
		}break;
		*/
	}
	ShowCaret(pTI->hwnd);
#if ENABLE_UI_DEBUG
	InvalidateRect(pTI->hwnd, NULL, TRUE);
#endif
}

void Handle_Delete_Key(TerminalEditInfo* pTI)
{
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szSelectedText[MAX_SIZE] = {0};

	int iLen = 0;
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		memset(pTI->szCommandBuffer, 0x00, sizeof(pTI->szCommandBuffer));
		memset(pTI->szMainBuffer, 0x00, sizeof(pTI->szMainBuffer));
		
		pTI->szCommandBuffer[0] = _T('#');
		pTI->szMainBuffer[0] = _T('#');

		pTI->iCaretPos = 1;
		pTI->iCharPos = 1;

		pTI->iCaretPosLen = GetWidthOfChar(pTI, pTI->szMainBuffer[0]);

		SetCaretPos(pTI->iCaretPosLen, 0);
	}
	else
	{
		if (pTI->iSelectStart > 0 && pTI->iSelectEnd > 0)
		{
			pcStart = &pTI->szCommandBuffer[pTI->iSelectStart];
			pcEnd = &pTI->szCommandBuffer[pTI->iSelectEnd];
			memset(szSelectedText, 0x00, sizeof(szSelectedText));
			_tcscpy(szSelectedText, pcEnd);
			
			#ifdef UNICODE
				memset(pcStart, 0x00, 2 * (pTI->iSelectEnd - pTI->iSelectStart));//due to unicode
			#else
				memset(pcStart, 0x00, (pTI->iSelectEnd - pTI->iSelectStart));
			#endif

			_tcscpy(pcStart, szSelectedText);
			BufferCopyCommandToMain(pTI);

			pTI->iCaretPos = pTI->iSelectStart;
			pTI->iCharPos = pTI->iSelectStart;

			memset(szSelectedText,0x00,sizeof(szSelectedText));
			_tcsncpy(szSelectedText, pTI->szMainBuffer, pTI->iSelectStart);

			pTI->iSelectEnd = 0;
			pTI->iSelectStart = 0;

			HideCaret(pTI->hwnd);
			pTI->iCaretPosLen = GetWidthOfString(pTI, szSelectedText);
			SetCaretPos(pTI->iCaretPosLen, 0);
			ShowCaret(pTI->hwnd);
		}
		else
		{
			if ((_tcslen(pTI->szMainBuffer) == 1) || (pTI->iCaretPos == _tcslen(pTI->szMainBuffer)))
				return;
			pTI->szMainBuffer[pTI->iCaretPos] = _T('\0');
			pTI->szCommandBuffer[pTI->iCaretPos] = _T('\0');
			pcStart = &pTI->szMainBuffer[pTI->iCaretPos];
			pcEnd = &pTI->szMainBuffer[pTI->iCaretPos + 1];
			iLen = _tcslen(pcEnd) + _tcslen(pTI->szMainBuffer);
			_tcscat(pcStart, pcEnd);

			pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
			pcEnd = &pTI->szCommandBuffer[pTI->iCaretPos + 1];
			iLen = _tcslen(pcEnd) + _tcslen(pTI->szCommandBuffer);
			_tcscat(pcStart, pcEnd);
		}
	}
	InvalidateRect(pTI->hwnd, NULL, TRUE);
}

void MoveCaretLeftRight(TerminalEditInfo* pTI, int iFlag)
{
	int iLen = 0;

	TCHAR* pcStart = NULL, * pcEnd = NULL;

	HideCaret(pTI->hwnd);
	int iCharPos = pTI->iCaretPos;
	switch (iFlag)
	{
		case 0://left
		{
			if (iCharPos == 1)
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}
				ShowCaret(pTI->hwnd);
				return;
			}

			if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
			{
				if (pTI->iSelectStart == 0 && pTI->iSelectEnd == 0)
				{
					pTI->iSelectEnd = pTI->iCaretPos;
					
					while (iCharPos > 1)
					{
						if (
							((pTI->szMainBuffer[iCharPos - 1] >= 65) && (pTI->szMainBuffer[iCharPos - 1] <= 90)) ||
							((pTI->szMainBuffer[iCharPos - 1] >= 97) && (pTI->szMainBuffer[iCharPos - 1] <= 122))
							)
						{
							iLen += GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos - 1]);
							iCharPos--;
						}
						else
							break;
					}
					pTI->iSelectStart = iCharPos;
				}
				ShowSelectedText(pTI);
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}
				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;

				while (iCharPos > 1)
				{
					if (
						((pTI->szMainBuffer[iCharPos-1] >= 65) && (pTI->szMainBuffer[iCharPos-1] <= 90)) ||
						((pTI->szMainBuffer[iCharPos-1] >= 97) && (pTI->szMainBuffer[iCharPos-1] <= 122))
						)
					{
						iLen += GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos-1]);
						iCharPos--;
					}
					else
						break;
				}
			}
			else if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				if (pTI->iSelectStart == 0 && pTI->iSelectEnd == 0)
				{
					pTI->iSelectEnd = pTI->iCaretPos;
					pTI->iSelectStart = pTI->iCaretPos - 1;
				}
				else
				{
					if (pTI->iCaretPos > 1 )
						pTI->iSelectStart = pTI->iCaretPos - 1;
				}
				
				ShowSelectedText(pTI);
				iCharPos--;
				iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
			}
			else
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}

				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;
				iCharPos--;
				iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
			}

			pTI->iCaretPosLen -= iLen;
		}break;
		case 1://right
		{
			if (iCharPos == _tcslen(pTI->szMainBuffer))
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}
				ShowCaret(pTI->hwnd);
				return;
			}
			
			if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
			{
				if (pTI->iSelectStart == 0 && pTI->iSelectEnd == 0)
				{
					pTI->iSelectStart = pTI->iCaretPos;
					
					while (iCharPos >= 1)
					{
						if (
							((pTI->szMainBuffer[iCharPos] >= 65) && (pTI->szMainBuffer[iCharPos] <= 90)) ||
							((pTI->szMainBuffer[iCharPos] >= 97) && (pTI->szMainBuffer[iCharPos] <= 122))
							)
						{
							iLen += GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
							iCharPos++;
						}
						else
							break;
					}
					pTI->iSelectEnd = iCharPos;
				}
				ShowSelectedText(pTI);
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}
				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;

				while (iCharPos < _tcslen(pTI->szMainBuffer))
				{
					if (
						((pTI->szMainBuffer[iCharPos] >= 65) && (pTI->szMainBuffer[iCharPos] <= 90)) ||
						((pTI->szMainBuffer[iCharPos] >= 97) && (pTI->szMainBuffer[iCharPos] <= 122))
						)
					{
						iLen += GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
						iCharPos++;
					}
					else
						break;
				}
			}
			else if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				if (pTI->iSelectStart == 0 && pTI->iSelectEnd == 0)
				{
					pTI->iSelectEnd = pTI->iCaretPos + 1;
					pTI->iSelectStart = pTI->iCaretPos;
				}
				else
				{
					if ( pTI->iCaretPos < (int)_tcslen(pTI->szMainBuffer) )
						pTI->iSelectEnd = pTI->iCaretPos + 1;
				}
				ShowSelectedText(pTI);
				iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
				iCharPos++;
			}
			else
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}

				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;
				iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[iCharPos]);
				iCharPos++;
			}

			pTI->iCaretPosLen += iLen;
		}break;
		case 2://Home
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				pTI->iSelectEnd = pTI->iCaretPos;
				pTI->iSelectStart = 1;

				ShowSelectedText(pTI);
			}
			else
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}

				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;
			}

			if (iCharPos == 1)
			{
				ShowCaret(pTI->hwnd);
				return;
			}

			iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[0]);
			iCharPos = 1;

			pTI->iCaretPosLen = iLen;
		}break;
		case 3://End
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				pTI->iSelectStart = pTI->iCaretPos;
				pTI->iSelectEnd = _tcslen(pTI->szMainBuffer + pTI->iSelectStart) + pTI->iCaretPos;

				ShowSelectedText(pTI);
			}
			else
			{
				if (pTI->iSelectEnd != 0 || pTI->iSelectStart != 0)
				{
					InvalidateRect(pTI->hwnd, NULL, TRUE);
				}

				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;
			}

			if (iCharPos == _tcslen(pTI->szMainBuffer))
			{
				ShowCaret(pTI->hwnd);
				return;
			}

			iLen = GetWidthOfString(pTI, pTI->szMainBuffer);
			iCharPos = _tcslen(pTI->szMainBuffer);
			pTI->iCaretPosLen = iLen;
		}break;
	}
	SetCaretPos(pTI->iCaretPosLen, 0);
	ShowCaret(pTI->hwnd);
	pTI->iCaretPos = iCharPos;
	pTI->iCharPos = iCharPos;
}

void HandleCharKeyPress(TerminalEditInfo* pTI)
{
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szTemp[MAX_SIZE] = {0};
	int iLen=0;

	switch (pTI->wParam)
	{
	case 8://BackSpace
		if (1 != pTI->iCharPos || (pTI->iSelectStart > 0 && pTI->iSelectEnd > 0))
		{
			if (pTI->iSelectStart > 0 && pTI->iSelectEnd > 0)
			{
				pcStart = &pTI->szCommandBuffer[pTI->iSelectStart];
				pcEnd = &pTI->szCommandBuffer[pTI->iSelectEnd];

				#ifdef UNICODE
				memset(pcStart, 0x00, 2 * (pTI->iSelectEnd - pTI->iSelectStart));//due to unicode
				#else
				memset(pcStart, 0x00, (pTI->iSelectEnd - pTI->iSelectStart));
				#endif

				memset(szTemp, 0x00, sizeof(szTemp));
				_tcscpy(szTemp, pcEnd);
				_tcscpy(pcStart, szTemp);
				BufferCopyCommandToMain(pTI);

				pTI->iCaretPos = pTI->iSelectStart;
				pTI->iCharPos = pTI->iSelectStart;

				memset(szTemp, 0x00, sizeof(szTemp));
				_tcsncpy(szTemp, pTI->szMainBuffer, pTI->iSelectStart);

				pTI->iSelectEnd = 0;
				pTI->iSelectStart = 0;

				HideCaret(pTI->hwnd);
				pTI->iCaretPosLen = GetWidthOfString(pTI, szTemp);
				SetCaretPos(pTI->iCaretPosLen, 0);
				ShowCaret(pTI->hwnd);
			}
			else
			{
				pTI->iCharPos--;
				pTI->iCaretPos--;

				iLen = GetWidthOfChar(pTI, pTI->szMainBuffer[pTI->iCaretPos]);

				pTI->iCaretPosLen -= iLen;

				if (pTI->szMainBuffer[pTI->iCaretPos + 1] != _T('\0'))
				{
					pTI->szMainBuffer[pTI->iCaretPos] = _T('\0');
					pTI->szCommandBuffer[pTI->iCaretPos] = _T('\0');
					pcStart = &pTI->szMainBuffer[pTI->iCaretPos];
					pcEnd = &pTI->szMainBuffer[pTI->iCaretPos + 1];
					iLen = _tcslen(pcEnd) + _tcslen(pTI->szMainBuffer);
					_tcscat(pcStart, pcEnd);

					pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
					pcEnd = &pTI->szCommandBuffer[pTI->iCaretPos + 1];
					iLen = _tcslen(pcEnd) + _tcslen(pTI->szCommandBuffer);
					_tcscat(pcStart, pcEnd);
				}
				else
				{
					pTI->szMainBuffer[pTI->iCaretPos] = _T('\0');
					pTI->szCommandBuffer[pTI->iCaretPos] = _T('\0');
				}
			}
			InvalidateRect(pTI->hwnd, NULL, TRUE);
		}
		break;// BackSpace end

	case 32://Space
	{
			if (pTI->szMainBuffer[pTI->iCaretPos] != _T('\0'))
			{
				pcStart = &pTI->szMainBuffer[pTI->iCaretPos];
				_tcscpy(szTemp, pcStart);
				pTI->szMainBuffer[pTI->iCaretPos] = pTI->wParam;
				pcStart++;
				_tcscpy(pcStart, szTemp);

				memset(szTemp, 0x00, sizeof(szTemp));

				pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
				iLen = _tcslen(pcStart) + 1;
				_tcscpy(szTemp, pcStart);
				pTI->szCommandBuffer[pTI->iCaretPos] = _T('$');
				pcStart++;
				_tcscpy(pcStart, szTemp);
			}
			else
			{
				pTI->szMainBuffer[pTI->iCaretPos] = pTI->wParam;
				pTI->szCommandBuffer[pTI->iCaretPos] = _T('$');
			}
			pTI->iCharPos++;
			pTI->iCaretPos++;

			iLen = GetWidthOfChar(pTI, pTI->wParam);
			
			pTI->iCaretPosLen += iLen;
			SetCaretPos(pTI->iCaretPosLen, 0);
			MyInvalidate(pTI);
	}break; //Space end

	case 13:
	{
		//EnterKeyPress(pTI);
		memset(pTI->szCommandBuffer, 0x00, sizeof(pTI->szCommandBuffer));
		memset(pTI->szMainBuffer, 0x00, sizeof(pTI->szMainBuffer));

		pTI->szCommandBuffer[0] = _T('#');
		pTI->szMainBuffer[0] = _T('#');

		pTI->iCaretPosLen = GetWidthOfChar(pTI, _T('#'));
		
		SetCaretPos(pTI->iCaretPosLen, 0);
		pTI->iCaretPos = 1;
		pTI->iCharPos = 1;

		InvalidateRect(pTI->hwnd, NULL, TRUE);
	}break;
	case 127:
		break;
	default:
		if (pTI->szMainBuffer[pTI->iCaretPos] != _T('\0'))
		{
			pcStart = &pTI->szMainBuffer[pTI->iCaretPos];
			_tcscpy(szTemp, pcStart);
			pTI->szMainBuffer[pTI->iCaretPos] = pTI->wParam;
			pcStart++;
			_tcscpy(pcStart, szTemp);

			memset(szTemp, 0x00, sizeof(szTemp));

			pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
			iLen = _tcslen(pcStart) + 1;
			_tcscpy(szTemp, pcStart);
			pTI->szCommandBuffer[pTI->iCaretPos] = pTI->wParam;
			pcStart++;
			_tcscpy(pcStart, szTemp);
		}
		else
		{
			pTI->szCommandBuffer[pTI->iCaretPos] = pTI->wParam;
			pTI->szMainBuffer[pTI->iCaretPos] = pTI->wParam;
		}
		pTI->iCharPos++;
		pTI->iCaretPos++;

		iLen = GetWidthOfChar(pTI, pTI->wParam);
		pTI->iCaretPosLen += iLen;

		MyInvalidate(pTI);
		break;
	}
	SetCaretPos(pTI->iCaretPosLen, 0);
	InvalidateRect(pTI->hwnd, NULL, TRUE);
}

void MyInvalidate(TerminalEditInfo* pTI)
{
	HDC hdc;
	GetClientRect(pTI->hwnd, &pTI->rc);
	hdc = GetDC(pTI->hwnd);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(0, 255, 0));
	SelectObject(hdc, pTI->hFont);
	DrawText(hdc, pTI->szMainBuffer, -1, &pTI->rc, DT_LEFT);
	ReleaseDC(pTI->hwnd, hdc);
}

void ShowSelectedText(TerminalEditInfo* pTI)
{
	TCHAR szSelectedText[MAX_SIZE] = { 0 };

	TCHAR szFromStarting[MAX_SIZE] = { 0 };
	TCHAR szToEnding[MAX_SIZE] = { 0 };

	HDC hdc;
	int iLen = 0;

	hdc = GetDC(pTI->hwnd);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(0, 255, 0));
	SelectObject(hdc, pTI->hFont);
	DrawText(hdc, pTI->szMainBuffer, -1, &pTI->rc, DT_LEFT);

	SetBkColor(hdc, RGB(255,255,0));
	SetTextColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, pTI->hFont);

	_tcsncpy(szSelectedText, &pTI->szMainBuffer[pTI->iSelectStart], pTI->iSelectEnd - pTI->iSelectStart);
	
	_tcsncpy(szFromStarting, pTI->szMainBuffer, pTI->iSelectStart);

	iLen = GetWidthOfString(pTI, szFromStarting);
	
	TextOut(hdc, iLen, 0, szSelectedText, pTI->iSelectEnd - pTI->iSelectStart);

	ReleaseDC(pTI->hwnd, hdc);
}

void BufferCopyCommandToMain(TerminalEditInfo* pTI)
{
	int i = 0;

	memset(pTI->szMainBuffer, 0x0, sizeof(pTI->szMainBuffer));
	for (i = 0; i < (int)_tcslen(pTI->szCommandBuffer); i++)
	{
		if (pTI->szCommandBuffer[i] == _T('$'))
			pTI->szMainBuffer[i] = _T(' ');
		else
			pTI->szMainBuffer[i] = pTI->szCommandBuffer[i];
	}
}

void Handle_TAB_Key(TerminalEditInfo* pTI)
{
	TCHAR* szCommands[] = {
		_T("cat"),
		_T("grep"),
		_T("ls"),
		_T("clear"),
		_T("exit"),
		_T("cd"),
		_T("cp"),
		_T("mv"),
		_T("rm"),
		_T("exec"),
		_T("mkdir"),
		_T("mkfl"),
		_T("find") ,
		_T("grep"),
		_T("rmdir"),
		0
	};

	DWORD dw;
	int iBackChar = 0;
	int iLen = 0;
	int iTotalLen = 0;
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szBuffer[MAX_SIZE] = { 0 };
	TCHAR szTemp[MAX_SIZE] = {0};
	TCHAR szPichliString[MAX_SIZE] = { 0 };
	TCHAR szAgliString[MAX_SIZE] = { 0 };
	TCHAR szPath[MAX_SIZE] = { 0 };

	TCHAR szCommand[MAX_SIZE] = { 0 };
	TCHAR szParam[MAX_SIZE] = { 0 };
	int iCommandCode = -1;
	int iCounter = 0;

	HANDLE hFile;
	WIN32_FIND_DATA ffd;

	HideCaret(pTI->hwnd);

	GetCommandNameAndParameters( pTI, szCommand, szParam);
	iCommandCode = GetCommandCode ( szCommand );
	
	/*
	if (-1 != iCommandCode)
	{
		_tcscat(szCommand,_T(" "));
	}
	*/

	if(-1 == iCommandCode)
	{
		pcStart = GetStartingEndPosForTab(pTI, _T('$'), 0);
		pcEnd = GetStartingEndPosForTab(pTI, _T('$'), 1);
		if ( NULL == pcEnd )
			_tcscpy(szPichliString, pcStart);
		else
		{
			_tcsncpy(szPichliString, pcStart, pcEnd - pcStart);
			_tcscpy(szAgliString, pcEnd);
		}

		iLen = _tcslen(szPichliString);

		for (iCounter = 0; iCounter < COMMANDS_COUNT; iCounter++)
		{
			if (NULL != szCommands[iCounter] && _tcsncmp(szCommands[iCounter], szPichliString, iLen) == 0)
			{
				memset(szPichliString, 0x00, MAX_SIZE);
				_tcscpy(szPichliString, szCommands[iCounter]);
				memset(pTI->szCommandBuffer,0x00,sizeof(pTI->szCommandBuffer));
				if (_tcslen(szAgliString) > 0)
				{
					_stprintf(pTI->szCommandBuffer, _T("#%s%s"), szPichliString, szAgliString);
					pTI->iCharPos = _tcslen(szPichliString)+2;
				}
				else
				{
					_stprintf(pTI->szCommandBuffer, _T("#%s$"), szPichliString);
					pTI->iCharPos = _tcslen(pTI->szCommandBuffer);
				}
				pTI->iCaretPos = pTI->iCharPos;

				BufferCopyCommandToMain(pTI);

				iLen = GetWidthOfChar(pTI, _T('#'));
				iLen += GetWidthOfString(pTI, szPichliString);
				iLen += GetWidthOfChar(pTI, _T(' '));
				pTI->iCaretPosLen = iLen;
				SetCaretPos(pTI->iCaretPosLen, 0);
				InvalidateRect(pTI->hwnd, NULL, TRUE);
				break;
			}
		}
	}
	else
	{
		pcStart = GetStartingEndPosForTab(pTI, _T('$'), 0);
		pcEnd = GetStartingEndPosForTab(pTI, _T('$'), 1);

		if (NULL == pcEnd)
		{
			_tcscpy (szPichliString, pcStart+1);
			memset(szAgliString, 0x00, sizeof(szAgliString));

			if (_tcsrchr(szPichliString, _T('\\')))
			{
				pcEnd = _tcschr(szPichliString, _T('\\'));

				_tcscpy(szBuffer, pcEnd + 1);

				if (szPichliString[0] == _T('~'))
				{
					pTI->iCaretPos -= _tcslen(szPichliString);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szPichliString);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					dw = MAX_SIZE;
					GetUserName(szPath, &dw);
					if (1 == _tcslen(szBuffer))
						_stprintf(szPichliString, _T("C:\\Users\\%s\\"), szPath);
					else
					{
						_stprintf(szPichliString, _T("C:\\Users\\%s\\%s"), szPath, szBuffer);
						FetchTheFileName(szPichliString);
					}

					_tcscpy(pcStart, szPichliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
				else
				{
					pTI->iCaretPos -= _tcslen(szPichliString);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szPichliString);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					FetchTheFileName(szPichliString);

					_tcscpy(pcStart, szPichliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
			}
			else
			{
				_tcscpy(szBuffer, szPichliString);

				if (szBuffer[0] == _T('~'))
				{
					pTI->iCaretPos -= _tcslen(szBuffer);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szBuffer);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					dw = MAX_SIZE;
					GetUserName(szPath, &dw);
					if (1 == _tcslen(szBuffer))
						_stprintf(szPichliString, _T("C:\\Users\\%s\\"), szPath);
					else
					{
						_stprintf(szPichliString, _T("C:\\Users\\%s\\%s"), szPath, szBuffer);
						FetchTheFileName(szPichliString);
					}

					_tcscpy(pcStart, szPichliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
				else
				{
					pTI->iCaretPos -= _tcslen(szBuffer);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szBuffer);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					FetchTheFileName(szPichliString);

					_tcscpy(pcStart, szPichliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
			}
		}
		else
		{
			_tcsncpy(szPichliString, pcStart+1, pcEnd - pcStart - 1);
			_tcscpy(szAgliString, pcEnd);

			if (_tcsrchr(szPichliString, _T('\\')))
			{
				pcEnd = _tcschr(szPichliString, _T('\\'));

				_tcscpy(szBuffer, pcEnd + 1);

				if (szPichliString[0] == _T('~'))
				{
					pTI->iCaretPos -= _tcslen(szPichliString);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szPichliString);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					dw = MAX_SIZE;
					GetUserName(szPath, &dw);
					if (1 == _tcslen(szBuffer))
						_stprintf(szPichliString, _T("C:\\Users\\%s\\"), szPath);
					else
					{
						_stprintf(szPichliString, _T("C:\\Users\\%s\\%s"), szPath, szBuffer);
						FetchTheFileName(szPichliString);
					}

					_tcscpy(pcStart, szPichliString);
					_tcscat(pcStart, szAgliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
				else
				{
					pTI->iCaretPos -= _tcslen(szPichliString);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szPichliString);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					FetchTheFileName(szPichliString);

					_tcscpy(pcStart, szPichliString);
					_tcscat(pcStart, szAgliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
			}
			else
			{
				_tcscpy(szBuffer, szPichliString);

				if (szBuffer[0] == _T('~'))
				{
					pTI->iCaretPos -= _tcslen(szBuffer);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szBuffer);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					dw = MAX_SIZE;
					GetUserName(szPath, &dw);
					if (1 == _tcslen(szBuffer))
						_stprintf(szPichliString, _T("C:\\Users\\%s\\"), szPath);
					else
					{
						_stprintf(szPichliString, _T("C:\\Users\\%s\\%s"), szPath, szBuffer);
						FetchTheFileName(szPichliString);
					}

					_tcscpy(pcStart, szPichliString);
					_tcscat(pcStart, szAgliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
				else
				{
					pTI->iCaretPos -= _tcslen(szBuffer);
					pTI->iCharPos = pTI->iCaretPos;
					iTotalLen = GetWidthOfString(pTI, szBuffer);
					pTI->iCaretPosLen -= iTotalLen;

					pcStart++;
					memset(pcStart, 0x00, _tcslen(szBuffer));

					memset(szPath, 0x00, sizeof(szPath));

					FetchTheFileName(szPichliString);
					_tcscpy(pcStart, szPichliString);
					_tcscat(pcStart, szAgliString);

					pTI->iCharPos += _tcslen(szPichliString);
					pTI->iCaretPos = pTI->iCharPos;
					pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
				}
			}
		}
		SetCaretPos(pTI->iCaretPosLen, 0);
		BufferCopyCommandToMain(pTI);
	}
	/**
	else
	{
		pcStart = GetStartingEndPosForTab(pTI, _T('$'), 0);
		pcEnd = GetStartingEndPosForTab(pTI, _T('$'), 1);

		if (NULL == pcEnd)
		{
			pcEnd = _tcschr(pTI->szCommandBuffer, _T('~'));
			if (NULL != pcEnd)
			{
				pcEnd++;

				if (pcEnd[0] == 0)
					pcEnd = NULL;
			}
		}

		if (pcStart[1] == _T('~'))
		{
			GetCurrentDirectory(sizeof(szPath), szPath);

			if (pcEnd != NULL)
			{
				pTI->iCaretPosLen -= GetWidthOfChar(pTI, _T('~'));

				pTI->iCharPos--;
				pTI->iCaretPos--;

				_tcscpy(szTemp, pcEnd);

				memset(szBuffer, 0x00, sizeof(szBuffer));

				dw = MAX_SIZE;
				GetUserName(szBuffer, &dw);

				pTI->iCharPos -= _tcslen(szTemp);
				iTotalLen = GetWidthOfString(pTI, szTemp);
				pTI->iCaretPosLen -= iTotalLen;

				pcStart++;
				memset(pcStart, 0x00, pcEnd-pcStart);
				_stprintf(szPichliString, _T("C:\\Users\\%s%s"), szBuffer, szTemp);

				memset(szTemp, 0x00, sizeof(szTemp));
				FetchTheFileName(szPichliString);

				//_stprintf(pcStart, _T("C:\\Users\\%s%s"), szBuffer, szTemp);
				_tcscpy(pcStart, szPichliString);

				pTI->iCharPos += _tcslen(szPichliString);
				pTI->iCaretPos = pTI->iCharPos;
				pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
			}
			else
			{
				pTI->iCaretPosLen -= GetWidthOfChar(pTI, _T('~'));

				pTI->iCharPos--;
				pTI->iCaretPos--;

				memset(szBuffer, 0x00, sizeof(szBuffer));

				dw = MAX_SIZE;
				GetUserName(szBuffer, &dw);

				_stprintf(szPichliString, _T("C:\\Users\\%s\\"), szBuffer);
				_stprintf(pcStart+1, _T("C:\\Users\\%s\\"), szBuffer);
				pTI->iCharPos += _tcslen(pcStart+1);
				pTI->iCaretPos = pTI->iCharPos;
				pTI->iCaretPosLen += GetWidthOfString(pTI, szPichliString);
			}
			SetCaretPos(pTI->iCaretPosLen, 0);
			BufferCopyCommandToMain(pTI);
		}
		else
		{
		}
	}
	/**/
	BufferCopyCommandToMain(pTI);
	ShowCaret(pTI->hwnd);
	MyInvalidate(pTI);
}

int GetCommandCode(TCHAR* szTemp)
{
	if (_tcscmp(szTemp, _T("cd")) == 0)
		return CMD_CD;
	else if (_tcscmp(szTemp, _T("cat")) == 0)
		return CMD_CAT;
	else if (_tcscmp(szTemp, _T("ls")) == 0)
		return CMD_LS;
	else if (_tcscmp(szTemp, _T("clear")) == 0)
		return CMD_CLEAR;
	else if (_tcscmp(szTemp, _T("exit")) == 0)
		return CMD_EXIT;
	else if (_tcscmp(szTemp, _T("grep")) == 0)
		return CMD_GREP;
	else if (_tcscmp(szTemp, _T("rm")) == 0)
		return CMD_RM;
	else if (_tcscmp(szTemp, _T("mv")) == 0)
		return CMD_MV;
	else if (_tcscmp(szTemp, _T("cp")) == 0)
		return CMD_CP;
	else if (_tcscmp(szTemp, _T("mkdir")) == 0)
		return CMD_MKDIR;
	else if (_tcscmp(szTemp, _T("mkfl")) == 0)
		return CMD_MKFL;
	else if (_tcscmp(szTemp, _T("find")) == 0)
		return CMD_FIND;
	else if (_tcscmp(szTemp, _T("exec")) == 0)
		return CMD_EXEC;
	else if (_tcscmp(szTemp, _T("rmdir")) == 0)
		return CMD_RMDIR;

	return -1;
}

void GetCommandNameAndParameters(TerminalEditInfo* pTI, TCHAR* szCommand, TCHAR* szParams)
{
	TCHAR* szCommands[] = {
		_T("cat"),
		_T("grep"),
		_T("ls"),
		_T("clear"),
		_T("exit"),
		_T("cd"),
		_T("cp"),
		_T("mv"),
		_T("rm"),
		_T("exec"),
		_T("mkdir"),
		_T("mkfl"),
		_T("find") ,
		_T("grep"),
		_T("rmdir"),
		0
	};

	TCHAR* pcSeperator = NULL;
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;

	pcSeperator = _tcschr(pTI->szCommandBuffer, _T('|'));

	pcStart = pTI->szCommandBuffer+1;
	pcEnd = _tcschr(pTI->szCommandBuffer, _T('$'));

	if (pcSeperator)
	{
		if ( pcEnd )
		{
			_tcsncpy(szCommand, pcStart, pcEnd - pcStart);
			pcStart = pcEnd + 1;

			pcEnd = pcSeperator-1;
			if (pcEnd[0] == _T('$'))
				pcEnd = pcSeperator - 1;
			else
				pcEnd = pcSeperator;
			
			_tcsncpy(szParams, pcStart, pcEnd - pcStart);
			ReplaceDollerWithSpace(szParams);
		}
		else
		{
			pcEnd = pcSeperator;
			_tcsncpy(szCommand, pcStart, pcEnd - pcStart);
			return;
		}
	}
	else
	{
		if (pcEnd)
		{
			_tcsncpy(szCommand, pcStart, pcEnd - pcStart);
			pcStart = pcEnd + 1;

			_tcscpy(szParams, pcStart);
			ReplaceDollerWithSpace(szParams);
		}
		else
		{
			_tcscpy(szCommand, pcStart);
			return;
		}
	}
}

void ReplaceDollerWithSpace(TCHAR* szParams)
{
	int i = 0;
	while (i<_tcslen(szParams))
	{
		if (szParams[i] == _T('$'))
			szParams[i] = _T(' ');
		i++;
	}
}

void ShowCommandError(TerminalEditInfo* pTI)
{
	
}

TCHAR* GetStartingEndPosForTab(TerminalEditInfo* pTI, char ch, int iForwardBackward)
{
	TCHAR* pcStart = &pTI->szCommandBuffer[pTI->iCaretPos];
	TCHAR* pcEnd = NULL;
	int iPos = pTI->iCaretPos-1;
	if (iForwardBackward)
	{
		pcEnd = _tcschr(&pTI->szCommandBuffer[pTI->iCaretPos], ch);
	}
	else
	{
		while (pTI->szCommandBuffer[iPos] != ch && iPos > 1)
			iPos--;
		pcEnd = &pTI->szCommandBuffer[iPos];
		
	}
	return pcEnd;
}

void FetchTheFileName(TCHAR *szPath)
{
	WIN32_FIND_DATA fd = {0};
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szFileName[260] = {0};
	TCHAR szFilePath[260] = { 0 };
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	int iLen = 0;

	pcStart = _tcsrchr(szPath, _T('\\'));

	if (NULL != pcStart)
	{
		pcStart++;
		pcEnd = _tcschr(pcStart, _T('$'));
		if (NULL != pcEnd)
		{
			_tcsncpy(szFileName, pcStart, pcEnd - pcStart);
			memset(pcStart, 0x00, pcEnd - pcStart);
		}
		else
		{
			_tcscpy(szFileName, pcStart);
			iLen = _tcslen(szFileName);
			memset(pcStart, 0x00, iLen);
		}

		_stprintf(szFilePath, _T("%s*.*"), szPath);
	}
	else
	{
		GetCurrentDirectory(sizeof(szFileName), szFileName);
		_stprintf(szFilePath, _T("%s\\*.*"), szFileName);
		memset(szFileName, 0x00,sizeof(szFileName));
		_tcscpy(szFileName, szPath);
		memset(szPath, 0x00, _tcslen(szPath));
	}

	iLen = 0;
	hFile = FindFirstFile(szFilePath, &fd);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBeep(-1);
		return;
	}
	
	do{
		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( 0 == _tcscmp(fd.cFileName, _T(".")) || 0 == _tcscmp(fd.cFileName, _T("..")) )
				continue;
		}

		if (_tcsstr(fd.cFileName, szFileName))
		{
			_tcscat(szPath, fd.cFileName);
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				_tcscat(szPath, _T("\\"));

			iLen = 1;
			break;
		}
	} while (FindNextFile( hFile, &fd) != 0 );

	FindClose(hFile);

	if (0 == iLen)
		_tcscat ( szPath, szFileName);
}
