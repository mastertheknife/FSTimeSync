#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>
#include "resource.h"

HWND hNoobdlg;
/*  This function is called by the Windows function DispatchMessage()  */
BOOL CALLBACK NoobDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
 					
	/* Initialize the windows common controls */
	InitCommonControls();

	MSG messages;

	hNoobdlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS), NULL, NoobDlgProc);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
		if(!IsDialogMessage(hNoobdlg, &messages))
		{	
      	  /* Translate virtual-key messages into character messages */
      	  TranslateMessage(&messages);
      	  /* Send message to WindowProcedure */
      	  DispatchMessage(&messages);
		}
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */
BOOL CALLBACK NoobDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDB_OK:
					MessageBox(hNoobdlg, "OK was pressed.","Noob Dialog",MB_OK | MB_ICONINFORMATION);
					break;
				case IDB_CANCEL:
					PostMessage(hNoobdlg,WM_CLOSE,0,0);
					break;
			}
			break;
		case WM_CLOSE:
			if(MessageBox(hNoobdlg,"Are you sure you want to close the window?","Noob Dialog",MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(hNoobdlg);
			}	
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
