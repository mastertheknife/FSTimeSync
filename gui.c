#define _WIN32_IE 0x0500
#define TRAYICONMSG WM_USER+2
#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"
#include "sync.h"
#include <commctrl.h>

static HWND hMainDlg = NULL;
static HWND hDummyWindow = NULL;
static HANDLE hGUIThread = NULL;
static UINT_PTR hDrawTimer;
static SyncOptions PendingSettings;
static HICON hIcon;
static NOTIFYICONDATA TrayIconData;
static unsigned int TrayIconState;
static HMENU TrayMenu;
static HMENU SubTrayMenu;

LRESULT CALLBACK TraynHotkeysProc(HWND hwnd,UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hTemphwnd;
    switch(Message)
    {
		case TRAYICONMSG:
			switch(lParam) {
				case WM_CONTEXTMENU:
					{
						POINT CurPos;			
						GetCursorPos(&CurPos);
					
						SetMenuDefaultItem(SubTrayMenu,0,TRUE);
						
						if(GetOperMode())
							CheckMenuRadioItem(SubTrayMenu,2,3,2,MF_BYPOSITION);
						else
							CheckMenuRadioItem(SubTrayMenu,2,3,3,MF_BYPOSITION);
						
						SetForegroundWindow(hwnd);
						if(GetSystemMetrics(SM_MENUDROPALIGNMENT)) 
							TrackPopupMenu(SubTrayMenu,TPM_LEFTALIGN | TPM_BOTTOMALIGN,CurPos.x,CurPos.y,0,hwnd,NULL);
						else
							TrackPopupMenu(SubTrayMenu,TPM_RIGHTALIGN | TPM_BOTTOMALIGN,CurPos.x,CurPos.y,0,hwnd,NULL);
						PostMessage(hwnd, WM_NULL, 0, 0);
						
					}
					break;
				case WM_USER:
					/* Left click on the icon. Create the window
					   Or bring it to the foreground if it already exists
					*/
					GUIOpenMain();
					break;	
				default:
					return DefWindowProc(hwnd,Message,wParam,lParam);				
			}
        	break;
        case WM_HOTKEY:
			if(wParam == 443)
				MessageBox(hMainDlg,"Received 443 HOTKEY (MANUAL SYNC)","FS Time Sync",MB_OK);
			else if(wParam == 444) {
				/* If automatic, change to manual and vice versa. */
				if(GetOperMode()) {
					SetOperMode(FALSE);
					/* If the dialog exists, redraw will also update the tray,
				   	But if it doesn't, we have to update the tray ourselves */
					if(hMainDlg)
						GUIOperModeDraw(hMainDlg,0);
					else
						GUITrayUpdate();
				} else {
					SetOperMode(TRUE);					
					if(hMainDlg)
						GUIOperModeDraw(hMainDlg,1);
					else
						GUITrayUpdate();	
				}
			} else {
				return DefWindowProc(hwnd,Message,wParam,lParam);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDM_RESTORE:
					GUIOpenMain();
					break;
				case IDM_QUIT:
					PostQuitMessage(0);
					bQuit = 1;
					break;	
				case IDM_AUTOMATIC:
					/* If already in automatic mode, do nothing */
					if(GetOperMode())
						break;

					SetOperMode(TRUE);
					if(hMainDlg)
						GUIOperModeDraw(hMainDlg,1);
					else
						GUITrayUpdate();
									
					break;
				case IDM_MANUAL:
					/* If already in manual mode, do nothing */
					if(!GetOperMode())
						break;

					SetOperMode(FALSE);
					if(hMainDlg)
						GUIOperModeDraw(hMainDlg,0);
					else
						GUITrayUpdate();
									
					break;
				default:
					return DefWindowProc(hwnd,Message,wParam,lParam);	
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
				
				/* Create the draw timer */
				hDrawTimer = SetTimer(hwnd,4231,250,NULL);
								
				/* Draw the dialog operating modee (Auto or manual) */
				GUIOperModeDraw(hwnd,GetOperMode());

				/* Draw the other elements */
				GUIElementsDraw(hwnd);
							
				SendDlgItemMessage(hwnd,IDP_NEXTSYNC, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 4)); 
				SendDlgItemMessage(hwnd,IDP_NEXTSYNC, PBM_SETPOS, (WPARAM)3, 0); 
				SendDlgItemMessage(hwnd,IDP_SYNCSTATUS, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 100)); 
				SendDlgItemMessage(hwnd,IDP_SYNCSTATUS, PBM_SETPOS, (WPARAM)100, 0); 
			
			}
			break;
		case WM_TIMER:
			/* Updates the main window elements, such as time and sync status */
			GUIElementsDraw(hwnd);
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
						if(GetOperMode()) {
							SetOperMode(FALSE);
							GUIOperModeDraw(hwnd,0);
						} else {
							SetOperMode(TRUE);
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
				/* Signal WM_DESTROY that we are quitting */
				bQuit = 1;
				DestroyWindow(hwnd);
			}	
			break;
		case WM_DESTROY:
			hDrawTimer = KillTimer(hwnd,4231);
			hMainDlg = NULL; /* To mark that this window is closed */
			if(bQuit == 1)
				PostQuitMessage(0); /* Quit */
			break;
		case WM_SYSCOMMAND:
			if(wParam == SC_MINIMIZE) {
				DestroyWindow(hwnd);
			} else {
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
				/* Set the icon for this dialog */
				GUISetDialogIcon(hwnd);
				
				/* Copy the settings from main to here */
				EnterCriticalSection(&SettingsCS);
				CopySettings(&PendingSettings,&Settings);
				LeaveCriticalSection(&SettingsCS);
				
				/* Load the options from the structure into the dialog */
				GUIOptionsDraw(hwnd,&PendingSettings);
			}
			break;		
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDB_OK:
					/* Save the options from the dialog into the structure */
					GUIOptionsSave(hwnd,&PendingSettings);
					
					/* Update the hotkeys by re-registering them */
					HotkeysUnregister(hDummyWindow);
					HotkeysRegister(hDummyWindow,PendingSettings.ManSyncHotkey,PendingSettings.ModeSwitchHotkey);					

					/* Copy them to main settings */
					EnterCriticalSection(&SettingsCS);
					CopySettings(&Settings,&PendingSettings);
					LeaveCriticalSection(&SettingsCS);
					
					/* Save to registry */
					RegistryWriteSettings(&PendingSettings);

					/* Close the options dialog */
					EndDialog(hwnd,IDB_OK);
					break;
				case IDB_CANCEL:
					EndDialog(hwnd,IDB_OK);
					break;
				case IDB_DEFAULTS:
					CopySettings(&PendingSettings,&Defaults);
					GUIOptionsDraw(hwnd,&PendingSettings);
					break;
				case IDC_UTCOFFSET:
					{
						HWND hEUTCOffset = GetDlgItem(hwnd,IDE_UTCOFFSET);		
						if(SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_GETCHECK,0,0) == BST_CHECKED) {
							EnableWindow(hEUTCOffset,TRUE);
						} else {
							EnableWindow(hEUTCOffset,FALSE);
						}
						CloseHandle(hEUTCOffset);
					}
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
	wcex.lpfnWndProc    = TraynHotkeysProc;
	wcex.hInstance      = hInst;
	wcex.lpszClassName  = "DUMMYWINDOWFORTRAYNOTIFICATIONS";
	RegisterClassEx(&wcex);
	
	/* Create dummy window for tray icon message processing */
	if(!(hDummyWindow = CreateWindowEx(0,"DUMMYWINDOWFORTRAYNOTIFICATIONS",NULL,0,0,0,0,0,HWND_MESSAGE,0,hInst,NULL))) {
		debuglog(DEBUG_ERROR,"Creating dummy window failed\n");
	}
	
	/* Setting up the tray icon properties */
	ZeroMemory(&TrayIconData,sizeof(NOTIFYICONDATA));
	TrayIconData.cbSize = sizeof(NOTIFYICONDATA);
	TrayIconData.hWnd = hDummyWindow;
	TrayIconData.uID = 120988;
	TrayIconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	TrayIconData.hIcon = hIcon;
	TrayIconData.uCallbackMessage = TRAYICONMSG;	
	TrayIconData.uVersion = NOTIFYICON_VERSION; /* Windows 2000 or later */
	
	if(GetOperMode())
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
	
	/* Register the hotkeys */
	HotkeysRegister(hDummyWindow, Settings.ManSyncHotkey, Settings.ModeSwitchHotkey);

	/* Creating the main window, unless start minimized is enabled */
	if((int)lpParameter == SW_MINIMIZE) {
		hMainDlg = NULL; 
	} else {
		hMainDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
		ShowWindow(hMainDlg,(int)lpParameter);
	}
			
	/* Message Loop! */
	/* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&Message, NULL, 0, 0)) {
		if(!IsDialogMessage(hMainDlg, &Message)) {	
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&Message);
			/* Send message to WindowProcedure */
			DispatchMessage(&Message);
		}
    }
    
    /* if we reached the end of the loop, 
	   It means the program is shutting down.. */
    
    /* Unregistering the hotkeys */
    HotkeysUnregister(hDummyWindow);
    
	/* Removing the tray icon */
	TrayIconState = 0;
	if(!Shell_NotifyIcon(NIM_DELETE,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed removing tray icon!\n");
	}	
		
	/* Kill all windows and return */
	if(hDummyWindow != NULL) {
		DestroyWindow(hDummyWindow);
		hDummyWindow = NULL;
	}
	if(hMainDlg != NULL) {
		DestroyWindow(hMainDlg);
		hMainDlg = NULL;
	}
		
	return 1;	
}

int GUIStartup(void) {
	/* Initialize the common controls */
	InitCommonControls();	
	
	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
		
	if(hIcon == NULL)
		debuglog(DEBUG_ERROR,"Failed to load icon!\n");	
	
	if(!(TrayMenu = LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDM_TRAYMENU))))
		debuglog(DEBUG_ERROR,"Failed to load tray menu!\n");
	
	SubTrayMenu = GetSubMenu(TrayMenu,0);
		
	return 1;
}


int GUIShutdown(void) {

	/* Close the handle to the icon */
	CloseHandle(hIcon);

	/* Close the handles to the menus */
	CloseHandle(SubTrayMenu);
	CloseHandle(TrayMenu);

	return 1;
}

int GUIStartThread(int nCmdShow) {

	hGUIThread = CreateThread(NULL,0,GUIThreadProc,(LPVOID)nCmdShow,0,NULL);
	if(hGUIThread) {
		debuglog(DEBUG_INFO,"Successfully created GUI thread\n");
	} else {
		debuglog(DEBUG_ERROR,"Failed creating GUI thread\n");
		return 0;
	}	
	return 1;
}

int GUIStopThread() {
	DWORD nRet;
	
	debuglog(DEBUG_INFO,"Stopping GUI Thread\n");
	
	/* Wait for the GUI thread to finish, up to 1 second */
	nRet = WaitForSingleObject(hGUIThread,1000);
	
	if(nRet == WAIT_FAILED) {
		debuglog(DEBUG_NOTICE,"GUI thread didn't close with WaitForSingleObject, Terminating it..\n");
		TerminateThread(hGUIThread,200);
	}
	return 1;
}
	
void GUIElementsDraw(HWND hwnd) {
	debuglog(DEBUG_CAPTURE,"Timer event!\n");
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
	/* Update the tray icon to the new mode */
	GUITrayUpdate();
	
	CloseHandle(hBSync);
	CloseHandle(hTNoManual);
	CloseHandle(hPNextSync);	
	
}

/* Loads the options from the structure into the dialog */
void GUIOptionsDraw(HWND hwnd,SyncOptions* Sets) {
	HWND hEUTCOffset = GetDlgItem(hwnd,IDE_UTCOFFSET);	
	
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_RESETCONTENT,0,0);
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"3 seconds");				
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"5 seconds");
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"10 seconds");			
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"30 seconds");
	
	if(Sets->StartMinimized)
		SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
		
	if(Sets->SystemUTCOffsetState) {
		SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
		SetDlgItemInt(hwnd,IDE_UTCOFFSET,Sets->SystemUTCOffset,TRUE);
		EnableWindow(hEUTCOffset,TRUE);						
	} else {
		SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
		SetDlgItemInt(hwnd,IDE_UTCOFFSET,Sets->SystemUTCOffset,TRUE);	
		EnableWindow(hEUTCOffset,FALSE);		
	}
	
	if(Sets->DaylightSaving)
		SendDlgItemMessage(hwnd,IDC_DAYLIGHTSAVING,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_DAYLIGHTSAVING,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
	
	if(Sets->AutoOnStart)
		SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);																	
			
	switch(Sets->AutoSyncInterval) {
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

	/* Set the rules on the hotkey controls */	
	SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_SETRULES,(WPARAM)(DWORD)HKCOMB_NONE,MAKELPARAM((HOTKEYF_CONTROL | HOTKEYF_SHIFT),0));
	SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_SETRULES,(WPARAM)(DWORD)HKCOMB_NONE,MAKELPARAM((HOTKEYF_CONTROL | HOTKEYF_SHIFT),0));

	/* Copy the stored hotkeys to the controls */
	SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_SETHOTKEY,PendingSettings.ManSyncHotkey,0);
	SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_SETHOTKEY,PendingSettings.ModeSwitchHotkey,0);	
	
	CloseHandle(hEUTCOffset);	
}

/* Saves the options from the dialog into the structure */
void GUIOptionsSave(HWND hwnd,SyncOptions* Sets) {
	if(SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->StartMinimized = 1;
	else
		Sets->StartMinimized = 0;
		
	if(SendDlgItemMessage(hwnd,IDC_UTCOFFSET,BM_GETCHECK,0,0) == BST_CHECKED) {
		Sets->SystemUTCOffsetState = 1;
		Sets->SystemUTCOffset = GetDlgItemInt(hwnd,IDE_UTCOFFSET,NULL,TRUE);
	} else {
		Sets->SystemUTCOffsetState = 0;
		Sets->SystemUTCOffset = GetDlgItemInt(hwnd,IDE_UTCOFFSET,NULL,TRUE);	
	}
	
	if(SendDlgItemMessage(hwnd,IDC_DAYLIGHTSAVING,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->DaylightSaving = 1;
	else
		Sets->DaylightSaving = 0;	
			
	if(SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->AutoOnStart = 1;
	else
		Sets->AutoOnStart = 0;			
		
	switch(SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_GETCURSEL,0,0)) {
		case 0:
			Sets->AutoSyncInterval = 3;
			break;
		case 1:
			Sets->AutoSyncInterval = 5;
			break;
		case 2:	
			Sets->AutoSyncInterval = 10;
			break;
		case 3:
			Sets->AutoSyncInterval = 30;
			break;
		default:
			Sets->AutoSyncInterval = Defaults.AutoSyncInterval;
			break;			
	}
	
	/* Copy the hotkeys from the controls to the settings */
	PendingSettings.ManSyncHotkey = SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_GETHOTKEY,0,0);
	PendingSettings.ModeSwitchHotkey = SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_GETHOTKEY,0,0);
	
}

void GUITrayUpdate() {
	if(GetOperMode())
		sprintf(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Automatic\nInterval: %u seconds",10);
	else
		strcpy(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Manual");
		
	/* Only modify the tray icon if it exists. */
	if(TrayIconState == 1) {
		if(!Shell_NotifyIcon(NIM_MODIFY,&TrayIconData)) {
			debuglog(DEBUG_ERROR,"Failed modifying tray icon!\n");
		}
	}
}

void GUIOpenMain() {
	HWND hTemphwnd;
	if(hMainDlg == NULL) {
		/* Window doesn't exist, let's create it */
		hTemphwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
			if(hTemphwnd) {
				hMainDlg = hTemphwnd;
				ShowWindow(hMainDlg,SW_SHOW);
			}
	} else {
		/* Window is open, let's make it more clear to the user */
		SetForegroundWindow(hMainDlg);
	}
}

void GUISetDialogIcon(HWND hwnd) {
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}
