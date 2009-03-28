#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"
#include "sync.h"
#include <commctrl.h>

static HANDLE hThread = NULL;
static HWND hMainDlg = NULL;
static SyncOptions PendingSettings;


BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{
				/* Lock settings */			
				EnterCriticalSection(&SettingsCS);
				
				/* Modify the dialog for the current operating mode (Auto or manual) */
				if(AutoSync == 1) {
					GUIOperModeDraw(hwnd,1);
				} else {
					GUIOperModeDraw(hwnd,0);
				}	
							
				SendDlgItemMessage(hwnd,IDP_NEXTSYNC, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 4)); 
				SendDlgItemMessage(hwnd,IDP_NEXTSYNC, PBM_SETPOS, (WPARAM)3, 0); 
				SendDlgItemMessage(hwnd,IDP_SYNCSTATUS, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 100)); 
				SendDlgItemMessage(hwnd,IDP_SYNCSTATUS, PBM_SETPOS, (WPARAM)100, 0); 
				
				/* Release settings */
				LeaveCriticalSection(&SettingsCS);
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
						int nRet = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPTIONS), hMainDlg, OptionsDlgProc);
					}
					break;
				case IDB_MODE:
					{
						/* Operating Mode changed. Update the runtime setting and modify the dialog depending on the new mode */
						if(AutoSync == 1) {
							AutoSync = 0;
							GUIOperModeDraw(hwnd,0);
						} else {
							AutoSync = 1;
							GUIOperModeDraw(hwnd,1);
						}											 	
					}
					break;	
				case IDB_SYNCNOW:
					MessageBox(hwnd,"Not available yet.\n","FS Time Sync",MB_OK);
					break;	
	
			}
			break;
		
		case WM_CLOSE:
			if(MessageBox(hwnd,"Are you sure you want to quit?","FS Time Sync",MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
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
				HWND hEUTCOffset = GetDlgItem(hwnd,IDE_UTCOFFSET);	
						
				memcpy(&PendingSettings,&Settings,sizeof(SyncOptions)); /* Copy the settings in main to pending */
				
				SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"3 seconds");				
				SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"5 seconds");
				SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"10 seconds");			
				SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"30 seconds");
				switch(PendingSettings.AutoSyncInterval) {
					case 3:
						SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)0,0);
						break;
					case 5:
						SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)1,0);
						break;
					case 10:	
						SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)2,0);
						break;
					case 30:
						SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)3,0);
						break;
					default:
						SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)2,0);
						break;			
				}
				
				if(PendingSettings.StartMinimized == 1)
					SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
				else
					SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);																											
				
				if(PendingSettings.UTCOffsetState == 1) {
					SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
					SetDlgItemInt(hwnd,IDE_UTCOFFSET,PendingSettings.UTCOffset,TRUE);
					EnableWindow(hEUTCOffset,TRUE);						
				} else {
					SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
					SetDlgItemInt(hwnd,IDE_UTCOFFSET,PendingSettings.UTCOffset,TRUE);	
					EnableWindow(hEUTCOffset,FALSE);		
				}	
				CloseHandle(hEUTCOffset);									
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
	ShowWindow(hMainDlg,(int)lpParameter);
			
	/* Message Loop! */
	/* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&Message, NULL, 0, 0)) {
		if(!IsDialogMessage(hMainDlg, &Message)) {	
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

int GUIStartThread(int nCmdShow) {
	hThread = CreateThread(NULL,0,GUIThreadProc,(LPVOID)nCmdShow,0,NULL);
	debuglog(DEBUG_CAPTURE,"Thread created\n");
	return 1;
}

void GUIOperModeDraw(HWND hwnd, unsigned int AutoMode) {
	HWND hBSync = GetDlgItem(hwnd,IDB_SYNCNOW);
	HWND hTNoManual = GetDlgItem(hwnd,IDT_NOMANUAL);
	HWND hPNextSync = GetDlgItem(hwnd,IDP_NEXTSYNC);		
	
	if(AutoMode == 1) {
		char NextSyncMsg[96];
		sprintf(NextSyncMsg,"Next synchronization in %u seconds.",5);							
		EnableWindow(hBSync,FALSE);
		SetDlgItemText(hwnd,IDB_MODE,"Manual Mode");	
		SetDlgItemText(hwnd,IDT_OPERMODE,"Automatic");
		SetDlgItemText(hwnd,IDT_NEXTSYNC,NextSyncMsg);							
		ShowWindow(hPNextSync,SW_SHOW);								
		ShowWindow(hTNoManual,SW_SHOW);								
	} else {
		EnableWindow(hBSync,TRUE);
		SetDlgItemText(hwnd,IDB_MODE,"Automatic Mode");	
		SetDlgItemText(hwnd,IDT_OPERMODE,"Manual");
		SetDlgItemText(hwnd,IDT_NEXTSYNC,"To synchronizate the flight simulator clock, click on the\nSync Now button or the assigned hotkey (see options).");								
		ShowWindow(hPNextSync,SW_HIDE);														
		ShowWindow(hTNoManual,SW_HIDE);							
	}
	
	CloseHandle(hBSync);
	CloseHandle(hTNoManual);
	CloseHandle(hPNextSync);	
	
}
