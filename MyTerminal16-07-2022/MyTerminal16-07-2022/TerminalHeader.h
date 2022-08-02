#include <stdio.h>
#include<windows.h>
#include<tchar.h>

#pragma warning(disable:4996)

#define ENABLE_UI_DEBUG	0

#define MAX_SIZE	260
#define BUFFERSIZE	65535

#define COMMANDS_COUNT	14

enum COMMANDS
{
	CMD_CAT=0,
	CMD_GREP,
	CMD_LS,
	CMD_CLEAR,
	CMD_EXIT,
	CMD_CD,
	CMD_CP,
	CMD_MV,
	CMD_RM,
	CMD_EXEC,
	CMD_MKDIR,
	CMD_MKFL,
	CMD_FIND,
	CMD_RMDIR
};

typedef struct
{
	HWND hwnd;
	HFONT hFont;
	int iCharPos;
	int iCaretPos;
	int iSelectEnd;
	int iCaretPosLen;
	int iSelectStart;
	TCHAR szMainBuffer[260];
	TCHAR szCommandBuffer[260];

	COLORREF prevCol;
	COLORREF prevTextCol;

	WPARAM wParam;
	LPARAM lParam;
	PAINTSTRUCT ps;
	RECT rc;
	TCHAR *pchOutputBuffer;
}TerminalEditInfo;

typedef struct
{
	BOOL bASysnch;
	HANDLE hInEvent;
	HANDLE hOutEvent;
	HANDLE hInPipe;
	HANDLE hOutPipe;
	TCHAR szOptions[260];
	TCHAR szParameters[260];
}THREADPARAM;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int GetWidthOfString(TerminalEditInfo*, TCHAR* pcStart1);
int GetWidthOfChar(TerminalEditInfo* pTI, TCHAR ch);
/*
void Handle_UP_Key(HWND hwnd);
void Handle_Down_Key(HWND hwnd);
*/

void Handle_TAB_Key(TerminalEditInfo*);
void ShowSelectedText(TerminalEditInfo*);
void BufferCopyCommandToMain(TerminalEditInfo*);

void MoveCaretLeftRight(TerminalEditInfo*, int iFlag);
void HandleKeyDownMessage(TerminalEditInfo*);
void Handle_Delete_Key(TerminalEditInfo*);

void HandleCharKeyPress(TerminalEditInfo *);
void MyInvalidate(TerminalEditInfo*);

int GetCommandCode(TCHAR* szTemp);
void GetCommandNameAndParameters(TerminalEditInfo*, TCHAR* szCommand, TCHAR* szParams);
void ReplaceDollerWithSpace(TCHAR* szParams);
void ShowCommandError(TerminalEditInfo* pTI);
TCHAR* GetStartingEndPosForTab(TerminalEditInfo* pTI, char ch, int iForwardBackward);
void FetchTheFileName(TCHAR* szPath);

void EnterKeyPress(TerminalEditInfo* pTI);
