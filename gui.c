#define _WIN32_IE 0x0500
#define TRAYICONMSG WM_USER+2
#define MAINSIGNALMSG WM_USER+1
#include "globalinc.h"
#include "debug.h"
#include "gui.h"
#include "main.h"
#include "sync.h"
#include <commctrl.h>
#include "FSUIPC_User.h"

HWND hDummyWindow = NULL;
static HWND hMainDlg = NULL;
static HANDLE hGUIThread = NULL;
static UINT_PTR hDrawTimer;
static SyncOptions_t PendingSettings;
static HICON hIcon;
static HICON hIconGray;
static NOTIFYICONDATA TrayIconData;
static unsigned int TrayIconState;
static HMENU TrayMenu;
static HMENU SubTrayMenu;

static LRESULT CALLBACK TraynHotkeysProc(HWND hwnd,UINT Message, WPARAM wParam, LPARAM lParam) {
	HWND hTemphwnd;
    switch(Message)
    {
		case MAINSIGNALMSG:
			if(wParam == 10) 
				GUIUpdate();
			break;	
		case TRAYICONMSG:
			switch(lParam) {
				case WM_CONTEXTMENU:
					{
						POINT CurPos;			
						GetCursorPos(&CurPos);
					
						SetMenuDefaultItem(SubTrayMenu,0,TRUE);
						
						if(GetRTVal(FST_AUTOMODE)) {
							EnableMenuItem(SubTrayMenu,2,MF_BYPOSITION | MF_GRAYED);
							CheckMenuRadioItem(SubTrayMenu,3,4,3,MF_BYPOSITION);
						} else { 
							if(Stats.SimStatus) 
								EnableMenuItem(SubTrayMenu,2,MF_BYPOSITION | MF_ENABLED);
							else					
								EnableMenuItem(SubTrayMenu,2,MF_BYPOSITION | MF_GRAYED);
							CheckMenuRadioItem(SubTrayMenu,3,4,4,MF_BYPOSITION);
						}
						
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
			switch(wParam) {
				case 443:
					GUISyncNowEvent();
					break;
				case 444: 
					/* If automatic, change to manual and vice versa. */
					if(GetRTVal(FST_AUTOMODE)) {
						SetRTVal(FST_AUTOMODE,FALSE);
						/* If the dialog exists, redraw will also update the tray,
				   		But if it doesn't, we have to update the tray ourselves */
						GUIUpdate();
					} else {
						SetRTVal(FST_AUTOMODE,TRUE);
						GUIUpdate();
					}
					break;
				default:
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
					SetRTVal(FST_QUIT,1);
					break;
				case IDM_SYNCNOW:
					GUISyncNowEvent();
					break;		
				case IDM_AUTOMATIC:
					/* If already in automatic mode, do nothing */
					if(GetRTVal(FST_AUTOMODE))
						break;

					SetRTVal(FST_AUTOMODE,TRUE);
					GUIUpdate();
									
					break;
				case IDM_MANUAL:
					/* If already in manual mode, do nothing */
					if(!GetRTVal(FST_AUTOMODE))
						break;

					SetRTVal(FST_AUTOMODE,FALSE);
					GUIUpdate();
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

static BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{
				/* Set the icon for this dialog */
				GUISetDialogIcon(hwnd,Stats.SimStatus);

				/* Create the draw timer */
				hDrawTimer = SetTimer(hwnd,4231,200,NULL);

				/* Set the hMainDlg handle for the next call */
				hMainDlg = hwnd;

				/* Draw the dialog elements */
				GUIUpdate();

				/* Update the dialog elements */
				GUIElementsUpdate();
			}
			break;
		case WM_TIMER:
			/* Updates the main window elements, such as time and sync status */
			GUIElementsUpdate();
			break;	
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDB_CLOSE:
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
						if(GetRTVal(FST_AUTOMODE)) {
							SetRTVal(FST_AUTOMODE,FALSE);
							GUIUpdate();
						} else {
							SetRTVal(FST_AUTOMODE,TRUE);
							GUIUpdate();
						}											 	
					}
					break;	
				case IDB_SYNCNOW:
					GUISyncNowEvent();					
					break;
				case IDB_ABOUT:
					{
						int nRet = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hMainDlg, AboutDlgProc);
					}
					break;
				default:
					return FALSE;
			}
			break;
		case WM_CLOSE:
			if(MessageBox(hwnd,"Are you sure you want to quit?","FS Time Sync",MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES) {
				/* Signal WM_DESTROY that we are quitting */
				SetRTVal(FST_QUIT,1);
				DestroyWindow(hwnd);
			}	
			break;
		case WM_DESTROY:
			hDrawTimer = KillTimer(hwnd,4231);
			hMainDlg = NULL; /* To mark that this window is closed */
			if(GetRTVal(FST_QUIT))
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


static BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{				
				/* TEMPORARY: Hide the reserved option (previously was Daylight Saving) */
				HWND hOpt3 = GetDlgItem(hwnd,IDC_FUTUREOPT3);
				ShowWindow(hOpt3,SW_HIDE);				
				
				/* Set the icon for this dialog */
				GUISetDialogIcon(hwnd,1);
				
				/* Copy the settings from main to here */
				EnterCriticalSection(&ProgramDataCS);
				CopySettings(&PendingSettings,&Settings);
				LeaveCriticalSection(&ProgramDataCS);
				
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
					EnterCriticalSection(&ProgramDataCS);
					CopySettings(&Settings,&PendingSettings);
					LeaveCriticalSection(&ProgramDataCS);
					
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
				case IDC_UTCCORRECTION:
					{
						HWND hEUTCCorrection = GetDlgItem(hwnd,IDE_UTCCORRECTION);		
						if(SendDlgItemMessage(hwnd,IDC_UTCCORRECTION,BM_GETCHECK,0,0) == BST_CHECKED) {
							EnableWindow(hEUTCCorrection,TRUE);
						} else {
							EnableWindow(hEUTCCorrection,FALSE);
						}
						CloseHandle(hEUTCCorrection);
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

static BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_INITDIALOG:
			{
				GUISetDialogIcon(hwnd,1);
				ShowWindow(hwnd,SW_HIDE); /* Hide the window before processing */
				char StrBuff[128];
				sprintf(StrBuff,"FS Time Sync %s",Ver.VersionString);
				SetDlgItemText(hwnd,IDT_ABOUTL1,StrBuff);
				SetDlgItemText(hwnd,IDT_ABOUTL2,"by mastertheknife");
				sprintf(StrBuff,"%s %s",Ver.compiledate,Ver.compiletime);
				SetDlgItemText(hwnd,IDT_ABOUTL3,StrBuff);
				ShowWindow(hwnd,SW_SHOW);
			}
			break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDB_CLOSE)
				EndDialog(hwnd,IDB_CLOSE);
			else
				return FALSE;
			break;
		case WM_CLOSE:
			EndDialog(hwnd,1);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

static DWORD WINAPI GUIThreadProc(LPVOID lpParameter) {
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
	wcex.lpszClassName  = "DUMMYWINDOWFORMESSAGES";
	RegisterClassEx(&wcex);
	
	/* Create dummy window for tray icon message processing */
	if(!(hDummyWindow = CreateWindowEx(0,"DUMMYWINDOWFORMESSAGES",NULL,0,0,0,0,0,HWND_MESSAGE,0,hInst,NULL))) {
		debuglog(DEBUG_ERROR,"Creating dummy window failed\n");
	}
	
	/* Setting up the tray icon properties */
	ZeroMemory(&TrayIconData,sizeof(NOTIFYICONDATA));
	TrayIconData.cbSize = sizeof(NOTIFYICONDATA);
	TrayIconData.hWnd = hDummyWindow;
	TrayIconData.uID = 120988;
	TrayIconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	TrayIconData.hIcon = hIconGray;
	TrayIconData.uCallbackMessage = TRAYICONMSG;	
	TrayIconData.uVersion = NOTIFYICON_VERSION; /* Windows 2000 or later */
	strcpy(TrayIconData.szTip,"");

	/* Creating the tray icon */
	if(!Shell_NotifyIcon(NIM_ADD,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed creating tray icon!\n");
	}
	/* Setting the tray icon version to windows 2000 or later */
	if(!Shell_NotifyIcon(NIM_SETVERSION,&TrayIconData)) {
		debuglog(DEBUG_ERROR,"Failed setting tray icon version!\n");
	}	
	TrayIconState = 1; /* Tray icon ready */

	GUITrayUpdate(); /* Update the tray icon, mainly the tooltip */
	
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
	
	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONBLUE15));
	hIconGray = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONGRAY));	
	
	if(hIcon == NULL || hIconGray == NULL)
		debuglog(DEBUG_ERROR,"Failed to load icon(s)!\n");	
	
	if(!(TrayMenu = LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDM_TRAYMENU))))
		debuglog(DEBUG_ERROR,"Failed to load tray menu!\n");
	
	SubTrayMenu = GetSubMenu(TrayMenu,0);
		
	return 1;
}


int GUIShutdown(void) {

	/* Unload the icons */
	DestroyIcon(hIcon);
	DestroyIcon(hIconGray);

	/* Unload the menus */
	DestroyMenu(SubTrayMenu);
	DestroyMenu(TrayMenu);

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
	
static void GUIElementsUpdate() {
	const char* lpstr;
	char StrBuff[128];
	DWORD dwInt;
	int TimePercent;

	EnterCriticalSection(&ProgramDataCS);

	/* Only update the elements if the main loop finished another cycle */
	if(Stats.UpdateElements) {
		/* System UTC Time */
		lpstr = ctime(&Stats.SysUTCTime);
		SetDlgItemText(hMainDlg,IDT_SYSUTC,lpstr);		

		/* FS UTC Time - only if connected to FS */
		if(Stats.SimStatus) {
			lpstr = ctime(&Stats.SimUTCTime);	
			SetDlgItemText(hMainDlg,IDT_SIMUTC,lpstr);
		}
	
		if(Stats.SyncLastModified) {
			sprintf(StrBuff,"Last synchronizated in %s",ctime(&Stats.SyncLast));
			SetDlgItemText(hMainDlg,IDT_LASTSYNC,StrBuff);
			Stats.SyncLastModified = 0;
		}
	
		/* Sync status */
		if(Stats.SimStatus) {
			if(Stats.SimUTCTime > Stats.SysUTCTime)
				TimePercent = Stats.SimUTCTime - Stats.SysUTCTime;
			else
				TimePercent = Stats.SysUTCTime - Stats.SimUTCTime;
			TimePercent = 600 - TimePercent;
			if(TimePercent > 600)
				TimePercent = 600;
			if(TimePercent < 0)
				TimePercent = 0;
			dwInt = floor(((((double)TimePercent) / ((double)600))* 100)+0.5);
			if(dwInt > 100)
				dwInt = 100;
			if(dwInt < 0)
				dwInt = 0;
			sprintf(StrBuff,"%u%%",dwInt);
			SetDlgItemText(hMainDlg,IDT_SYNCSTATUS,StrBuff);
		}
	}
	
	if(GetRTVal(FST_AUTOMODE)) {
		if(Stats.SimStatus) {		
			if(Stats.SyncNext <= time(NULL))	
				dwInt = Settings.AutoSyncInterval;
			else
				dwInt = Stats.SyncNext - time(NULL);
			sprintf(StrBuff,"Next synchronization in %u seconds.",dwInt);
			SetDlgItemText(hMainDlg,IDT_NEXTSYNC,StrBuff);
			dwInt = Stats.SyncInterval - dwInt;
			SendDlgItemMessage(hMainDlg,IDP_NEXTSYNC, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, Stats.SyncInterval)); 
			SendDlgItemMessage(hMainDlg,IDP_NEXTSYNC, PBM_SETPOS, (WPARAM)dwInt, 0); 
		}
	}
	Stats.UpdateElements = 0; /* No need to re-update the elements if nothing has changed */
	
	LeaveCriticalSection(&ProgramDataCS);
}	

static void GUIUpdate() {	
	HWND hBSync = GetDlgItem(hMainDlg,IDB_SYNCNOW);
	HWND hTNoManual = GetDlgItem(hMainDlg,IDT_NOMANUAL);
	HWND hPNextSync = GetDlgItem(hMainDlg,IDP_NEXTSYNC);		
	HWND hToSync = GetDlgItem(hMainDlg,IDT_TOSYNC);
	HWND hNextSync = GetDlgItem(hMainDlg,IDT_NEXTSYNC);
	HWND hTLastSync = GetDlgItem(hMainDlg,IDT_LASTSYNC);
		
	/* Only update the window if main window exists */	
	if(hMainDlg) {	
		
		EnterCriticalSection(&ProgramDataCS);
		
		/* Things that are related to simstatus but not mode related are here */
		if(Stats.SimStatus) {
			switch(FSUIPC_FS_Version) {
				case SIM_FS98:
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"FS98 Running");
					break;
				case SIM_FS2K:
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"FS2000 Running");
					break;
				case SIM_FS2K2:
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"FS2002 Running");
					break;
				case SIM_FS2K4:
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"FS2004 Running");
					break;
				case SIM_FSX:
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"FS X Running");
					break;
				default:					
					SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"Running");
					break;
			}					
		} else {
			SetDlgItemText(hMainDlg,IDT_SIMSTATUS,"Not Running"); 				
			SetDlgItemText(hMainDlg,IDT_SIMUTC,"N/A");			
			SetDlgItemText(hMainDlg,IDT_SYNCSTATUS,"N/A");
			SetDlgItemText(hMainDlg,IDT_TOSYNC,"No simulator detected running. Please start your Microsoft Flight Simulator 98\\2000\\2002\\2004\\FS X.\n");
		}		
	
		if(GetRTVal(FST_AUTOMODE)) {	
			if(Stats.SimStatus) {
				ShowWindow(hToSync,SW_HIDE);
				ShowWindow(hNextSync,SW_SHOW);
				ShowWindow(hPNextSync,SW_SHOW);		
			} else {
				ShowWindow(hPNextSync,SW_HIDE);
				ShowWindow(hNextSync,SW_HIDE);				
				ShowWindow(hToSync,SW_SHOW);
			}	
			EnableWindow(hBSync,FALSE);							
			ShowWindow(hTNoManual,SW_SHOW);			
			SetDlgItemText(hMainDlg,IDB_MODE,"Manual Mode");	
			SetDlgItemText(hMainDlg,IDT_OPERMODE,"Automatic");										
		} else {
			if(Stats.SimStatus) {
				EnableWindow(hBSync,TRUE);
				SetDlgItemText(hMainDlg,IDT_TOSYNC,"To synchronizate the flight simulator clock, click on the\nSync Now button or the assigned hotkey (see options).");
			} else {
				EnableWindow(hBSync,FALSE);
			}
			ShowWindow(hPNextSync,SW_HIDE);		
			ShowWindow(hNextSync,SW_HIDE);		
			ShowWindow(hTNoManual,SW_HIDE);		
			ShowWindow(hToSync,SW_SHOW);																
			SetDlgItemText(hMainDlg,IDB_MODE,"Automatic Mode");	
			SetDlgItemText(hMainDlg,IDT_OPERMODE,"Manual");
		}
				
		/* Update the main window's icon */
		GUISetDialogIcon(hMainDlg,Stats.SimStatus);

		LeaveCriticalSection(&ProgramDataCS);
	} 
	
	/* Update the GUI elements such as next sync and progress bar */
	GUIElementsUpdate();
	
	/* Update the tray icon to the new mode */
	GUITrayUpdate();
			
	CloseHandle(hBSync);
	CloseHandle(hTNoManual);
	CloseHandle(hPNextSync);	
	CloseHandle(hToSync);
	CloseHandle(hNextSync);
}

/* Loads the options from the structure into the dialog */
static void GUIOptionsDraw(HWND hwnd,SyncOptions_t* Sets) {
	HWND hEUTCCorrection = GetDlgItem(hwnd,IDE_UTCCORRECTION);	
	
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_RESETCONTENT,0,0);
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"5 seconds");				
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"10 seconds");
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"30 seconds");			
	SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_ADDSTRING,0,(LPARAM)"60 seconds");
	
	if(Sets->StartMinimized)
		SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
		
	if(Sets->SystemUTCCorrectionState) {
		SendDlgItemMessage(hwnd,IDC_UTCCORRECTION,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
		EnableWindow(hEUTCCorrection,TRUE);						
	} else {
		SendDlgItemMessage(hwnd,IDC_UTCCORRECTION,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);	
		EnableWindow(hEUTCCorrection,FALSE);		
	}
		SetDlgItemInt(hwnd,IDE_UTCCORRECTION,Sets->SystemUTCCorrection,TRUE);	

	/* Reserved for future option, previously was Daylight Saving
	if(Sets->FutureSetting)
		SendDlgItemMessage(hwnd,IDC_FUTUREOPT3,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_FUTUREOPT3,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
	*/
	
	if(Sets->AutoOnStart)
		SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_SETCHECK,(WPARAM)BST_CHECKED,0);
	else
		SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);																	
			
	switch(Sets->AutoSyncInterval) {
		case 5:
			SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)0,0);
			break;
		case 10:
			SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)1,0);
			break;
		case 30:	
			SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)2,0);
			break;
		case 60:
			SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)3,0);
			break;
		default:
			SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_SETCURSEL,(WPARAM)1,0);
			break;			
	}

	/* Set the rules on the hotkey controls */	
	SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_SETRULES,(WPARAM)(DWORD)HKCOMB_NONE,MAKELPARAM((HOTKEYF_CONTROL | HOTKEYF_SHIFT),0));
	SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_SETRULES,(WPARAM)(DWORD)HKCOMB_NONE,MAKELPARAM((HOTKEYF_CONTROL | HOTKEYF_SHIFT),0));

	/* Copy the stored hotkeys to the controls */
	SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_SETHOTKEY,PendingSettings.ManSyncHotkey,0);
	SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_SETHOTKEY,PendingSettings.ModeSwitchHotkey,0);	
	
	CloseHandle(hEUTCCorrection);	
}

/* Saves the options from the dialog into the structure */
static void GUIOptionsSave(HWND hwnd,SyncOptions_t* Sets) {
	if(SendDlgItemMessage(hwnd,IDC_STARTMINIMIZED,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->StartMinimized = 1;
	else
		Sets->StartMinimized = 0;
		
	if(SendDlgItemMessage(hwnd,IDC_UTCCORRECTION,BM_GETCHECK,0,0) == BST_CHECKED) {
		Sets->SystemUTCCorrectionState = 1;
	} else {
		Sets->SystemUTCCorrectionState = 0;	
	}
	Sets->SystemUTCCorrection = GetDlgItemInt(hwnd,IDE_UTCCORRECTION,NULL,TRUE);

	/* Reserved for future option, previously was Daylight Saving	
	if(SendDlgItemMessage(hwnd,IDC_FUTUREOPT3,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->FutureSetting = 1;
	else
		Sets->FutureSetting = 0;
	*/
			
	if(SendDlgItemMessage(hwnd,IDC_AUTOSYNCSTARTUP,BM_GETCHECK,0,0) == BST_CHECKED)
		Sets->AutoOnStart = 1;
	else
		Sets->AutoOnStart = 0;			
		
	switch(SendDlgItemMessage(hwnd,IDL_SYNCINT,CB_GETCURSEL,0,0)) {
		case 0:
			Sets->AutoSyncInterval = 5;
			break;
		case 1:
			Sets->AutoSyncInterval = 10;
			break;
		case 2:	
			Sets->AutoSyncInterval = 30;
			break;
		case 3:
			Sets->AutoSyncInterval = 60;
			break;
		default:
			Sets->AutoSyncInterval = Defaults.AutoSyncInterval;
			break;			
	}
	
	/* Copy the hotkeys from the controls to the settings */
	PendingSettings.ManSyncHotkey = SendDlgItemMessage(hwnd,IDH_MANSYNCHOTKEY,HKM_GETHOTKEY,0,0);
	PendingSettings.ModeSwitchHotkey = SendDlgItemMessage(hwnd,IDH_OPERMODEHOTKEY,HKM_GETHOTKEY,0,0);
	
}

static void GUITrayUpdate() {
	/* todo: Add code to change the tray icon to grey depending on SimStatus */
	
	if(GetRTVal(FST_AUTOMODE))
		sprintf(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Automatic\nInterval: %u seconds",Settings.AutoSyncInterval);
	else
		strcpy(TrayIconData.szTip,"FS Time Sync v1.0\nMode: Manual");
	
	if(Stats.SimStatus)
		TrayIconData.hIcon = hIcon;
	else
		TrayIconData.hIcon = hIconGray;
		
	/* Only modify the tray icon if it exists. */
	if(TrayIconState == 1) {
		if(!Shell_NotifyIcon(NIM_MODIFY,&TrayIconData)) {
			debuglog(DEBUG_ERROR,"Failed modifying tray icon!\n");
		}
	}
}

static void GUIOpenMain() {
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

static void GUISetDialogIcon(HWND hwnd, DWORD Color) {
	if(Color) {
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	} else {
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconGray);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconGray);	
	}	
}

static void GUISyncNowEvent() {
	/* Checking for manual mode first */
	if(!GetRTVal(FST_AUTOMODE)) {
		/* Sync now only if there's no sync in progress */	
		if(!GetRTVal(FST_SYNCNOW))
			SetRTVal(FST_SYNCNOW,TRUE);
	}
}

