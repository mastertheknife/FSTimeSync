#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"
#include <commctrl.h>

HANDLE hThread = NULL;
HWND hOptionsDlg = NULL;
HWND hMainDlg = NULL;	

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{
				HWND hPNextSync = GetDlgItem(hwnd,IDP_NEXTSYNC);
				SendMessage(hPNextSync, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 4)); 
				SendMessage(hPNextSync, PBM_SETPOS, (WPARAM)3, 0); 
				
				HWND hPSyncStatus = GetDlgItem(hwnd,IDP_SYNCSTATUS);
				SendMessage(hPSyncStatus, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 100)); 
				SendMessage(hPSyncStatus, PBM_SETPOS, (WPARAM)100, 0); 
			}
			break;
		case WM_COMMAND:

			switch(LOWORD(wParam)) {
				case IDB_OK:
					/* Close the window, back to tray */
					DestroyWindow(hwnd);
					break;
				case IDB_OPTIONS:
					{
						int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS), hMainDlg, OptionsDlgProc);
					}
					break;
				case IDB_MODE:
					{
						HWND hBMode = GetDlgItem(hwnd,IDB_MODE);
						HWND hBSync = GetDlgItem(hwnd,IDB_SYNCNOW);
						if(AutoSync == 1) {
							EnableWindow(hBSync,TRUE);
							SendMessage(hBMode, WM_SETTEXT, 0, (LPARAM)"Automatic Mode");
							AutoSync = 0; /* Manual mode */
						} else {
							EnableWindow(hBSync,FALSE);
							SendMessage(hBMode, WM_SETTEXT, 0, (LPARAM)"Manual Mode");
							AutoSync = 1; /* Auto mode */	
						}
						CloseHandle(hBMode);
						CloseHandle(hBSync);					 	
					}
					break;		
	
			}
			break;
		
		case WM_CLOSE:
			if(MessageBox(hMainDlg,"Are you sure you want to quit?","FS Time Sync",MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
				DestroyWindow(hwnd);
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
		case WM_INITDIALOG:
			{
				HWND hPNextSync = GetDlgItem(hwnd,IDP_NEXTSYNC);
				SendMessage(hPNextSync, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 4)); 
				SendMessage(hPNextSync, PBM_SETPOS, (WPARAM)3, 0); 
				
				HWND hPSyncStatus = GetDlgItem(hwnd,IDP_SYNCSTATUS);
				SendMessage(hPSyncStatus, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 100)); 
				SendMessage(hPSyncStatus, PBM_SETPOS, (WPARAM)100, 0); 
			}
			break;		
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDB_OK:
					/* Save Settings here! */
					EndDialog(hwnd,IDB_OK);
					break;
				case IDB_CANCEL:
					EndDialog(hwnd,IDB_OK);
					break;					
			}
			break;		
		case WM_CLOSE:
			EndDialog(hwnd,IDB_CANCEL);
		default:
			return FALSE;
	}
	return TRUE;
}

DWORD WINAPI GUIThreadProc(LPVOID lpParameter) {
	MSG Message;	
	
	/* Find out if to start minimized or not. */
	
	/* Create the main window and tray icon */
	hMainDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
		
	/* Message Loop! */
	/* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&Message, NULL, 0, 0)) {
		if(!IsDialogMessage(hOptionsDlg, &Message) && !IsDialogMessage(hMainDlg, &Message)) {	
			debuglog(DEBUG_CAPTURE,"This part got called!\n");	
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&Message);
			/* Send message to WindowProcedure */
			DispatchMessage(&Message);
		}
    }
	
	
	/* Kill all windows and return */
	ExitProcess(0);
}

int GUIStartup(void) {
	/* Initialize the common controls */
	InitCommonControls();	
	return 1;	
}

int GUIShutdown(void) {
	/* unallocate and stuff */
	return 1;
}

int GUIStartThread(BOOL bStartMinimized) {
	hThread = CreateThread(NULL,0,GUIThreadProc,(LPVOID)bStartMinimized,0,NULL);
	debuglog(DEBUG_CAPTURE,"Thread created\n");
	return 1;
}
