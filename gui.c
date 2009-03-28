#define _WIN32_IE 0x0500
#define TRAYICONMSG 4231
#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"
#include "sync.h"
#include <commctrl.h>

static HWND hMainDlg = NULL;
static HWND hDummyWindow = NULL;

static HANDLE hThread = NULL;
static SyncOptions PendingSettings;
static HICON hIcon;
static HICON hIconSm;
static NOTIFYICONDATA TrayIconData;
static unsigned int TrayIconState;

LRESULT CALLBACK TrayProc(HWND hwnd,UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hTemphwnd;
    switch(Message)
    {
		case TRAYICONMSG:
			switch(lParam) {
				case WM_CONTEXTMENU:
					MessageBox(hwnd,"Right click detected\n","FS Time Sync",MB_OK);
					break;
				case WM_USER:
					debuglog(DEBUG_CAPTURE,"Left click detected on icon\n");
					if(hMainDlg == NULL) {
						/* Window doesn't exist, let's create it */
						hTemphwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
						if(hTemphwnd) {
							hMainDlg = hTemphwnd;
							ShowWindow(hMainDlg,SW_SHOW);
						}
					} else {
						/* Window is open, let's make it more clear to the user */
						return SetForegroundWindow(hMainDlg);
					}
					break;				
			}
        	break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
    }
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{
				
				/* Set the icon for this dialog */
				GUISetDialogIcon(hwnd);
				
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
				default:
					return FALSE;			
			}
			break;
		case WM_CLOSE:
			if(MessageBox(hwnd,"Are you sure you want to quit?","FS Time Sync",MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
				bQuit = 1;
				DestroyWindow(hwnd);
			}	
			break;
		case WM_DESTROY:
			hMainDlg = NULL; /* To mark that this window is closed */
			if(bQuit == 1)
				PostQuitMessage(0); /* Quit */
			break;
		case WM_SYSCOMMAND:
			switch(wParam) {
				case SC_MINIMIZE:
					DestroyWindow(hwnd);
					break;
				default:
					return FALSE;
			}			
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
				
				/* Set the icon for this dialog */
				GUISetDialogIcon(hwnd);
				
				memcpy(&PendingSettings,&Settings,sizeof(SyncOptions));
								
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
				default:
					return FALSE;					
			}
			break;		
		case WM_CLOSE:
			EndDialog(hwnd,IDB_CANCEL);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

DWORD WINAPI GUIThreadProc(LPVOID lpParameter) {
	MSG Message;
	HINSTANCE hInst;
	WNDCLASSEX wcex;
	
	/* Register dummy class for the tray icon (dummy window) */		
	hInst = GetModuleHandle(NULL);
	ZeroMemory(&wcex, sizeof wcex);
	wcex.cbSize         = sizeof(wcex);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.style          = 0;
	wcex.lpfnWndProc    = TrayProc;
	wcex.hInstance      = hInst;
	wcex.lpszClassName  = "DUMMYWINDOWFORTRAYNOTIFICATIONS";
	RegisterClassEx(&wcex);
	
	/* Create dummy window for tray icon message processing */
	if(!(hDummyWindow = CreateWindowEx(0,"DUMMYWINDOWFORTRAYNOTIFICATIONS",NULL,0,0,0,0,0,HWND_MESSAGE,0,hInst,NULL))) {
		debuglog(DEBUG_ERROR,"Creating dummy window failed\n");
	}
	
	/* Find out if to start minimized or not. */
	
	/* Creating the main window */
	hMainDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	ShowWindow(hMainDlg,(int)lpParameter);
	
	/* Setting up the tray icon properties */
	ZeroMemory(&TrayIconData,sizeof(NOTIFYICONDATA));
	TrayIconData.cbSize = sizeof(NOTIFYICONDATA);
	TrayIconData.hWnd = hDummyWindow;
	TrayIconData.uID = 120988;
	TrayIconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	TrayIconData.hIcon = hIcon;
	TrayIconData.uCallbackMessage = TRAYICONMSG;	
	TrayIconData.uVersion = NOTIFYICON_VERSION; /* Windows 2000 or later */
	/* TrayIconData.dwStateMask = NIS_HIDDEN; */
	
	if(AutoSync == 1)
		sprintf(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Automatic\nInterval: %u seconds",10);
	else
		strcpy(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Manual");	
	
	/* Creating the tray icon */
	if(!Shell_NotifyIcon(NIM_ADD,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed creating tray icon!\n");
	}
	/* Setting the tray icon version to windows 2000 or later */
	if(!Shell_NotifyIcon(NIM_SETVERSION,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed setting tray icon version!\n");
	}	
	TrayIconState = 1; /* Tray icon ready */

			
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
	
	/* Removing the tray icon */
	TrayIconState = 0;
	if(!Shell_NotifyIcon(NIM_DELETE,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed removing tray icon!\n");
	}	
		
	/* Kill all windows and return */
	ExitProcess(0);
}

int GUIStartup(void) {
	/* Initialize the common controls */
	InitCommonControls();	
	
	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
	hIconSm  = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
	
	if(hIcon == NULL)
		debuglog(DEBUG_ERROR,"Failed to load big 32x32 icon!\n");
		
	if(hIconSm == NULL)
		debuglog(DEBUG_ERROR,"Failed to load small 16x16 icon!\n");	
			
	return 1;	
}

int GUIShutdown(void) {

	/* Close the handles to the icons */
	CloseHandle(hIcon);
	CloseHandle(hIconSm);

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
		sprintf(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Automatic\nInterval: %u seconds",10);								
		EnableWindow(hBSync,FALSE);
		SetDlgItemText(hwnd,IDB_MODE,"Manual Mode");	
		SetDlgItemText(hwnd,IDT_OPERMODE,"Automatic");
		SetDlgItemText(hwnd,IDT_NEXTSYNC,NextSyncMsg);							
		ShowWindow(hPNextSync,SW_SHOW);								
		ShowWindow(hTNoManual,SW_SHOW);							
	} else {
		strcpy(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Manual");
		EnableWindow(hBSync,TRUE);
		SetDlgItemText(hwnd,IDB_MODE,"Automatic Mode");	
		SetDlgItemText(hwnd,IDT_OPERMODE,"Manual");
		SetDlgItemText(hwnd,IDT_NEXTSYNC,"To synchronizate the flight simulator clock, click on the\nSync Now button or the assigned hotkey (see options).");								
		ShowWindow(hPNextSync,SW_HIDE);														
		ShowWindow(hTNoManual,SW_HIDE);							
	}
	
	/* Only modify the tray icon if it exists. */
	if(TrayIconState == 1) {
		if(!Shell_NotifyIcon(NIM_MODIFY,&TrayIconData)) {
			debuglog(DEBUG_ERROR,"Failed modifying tray icon!\n");
		}
	}

	CloseHandle(hBSync);
	CloseHandle(hTNoManual);
	CloseHandle(hPNextSync);	
	
}

void GUISetDialogIcon(HWND hwnd) {
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
}
