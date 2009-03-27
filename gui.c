#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
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

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
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

DWORD WINAPI GUIThreadProc(LPVOID lpParameter) {
	HWND hOptionsDlg = NULL;
	HWND hMainDlg = NULL;	
	MSG Message;	
	
	/* Find out if to start minimized or not. */
	
	/* Create the main window and tray icon */
	hNoobdlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS), NULL, OptionsDlgProc);
		
	/* Message Loop! */
	/* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&Message, NULL, 0, 0)) {
		if(!IsDialogMessage(hOptionsDlg, &Message)) {	
			debuglog(DEBUG_CAPTURE,"This part got called!\n");	
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&Message);
			/* Send message to WindowProcedure */
			DispatchMessage(&Message);
		}
    }
	
	
	/* Kill all windows and return */
	
}

int GUIStartup(void) {
	/* Initialize the common controls */
	InitCommonControls();
}

int GUIShutdown(void) {
	/* unallocate and stuff */
}
