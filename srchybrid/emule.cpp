//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "stdafx.h"
#include <locale.h>
#include "emule.h"
#include "version.h"
#include "opcodes.h"
#ifdef _DUMP
#include "mdump.h"
#endif
#include "Scheduler.h"
#include "SearchList.h"
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/kademlia/Error.h"
#include "kademlia/utils/UInt128.h"
#include "PerfLog.h"
#include <..\src\mfc\sockimpl.h>
#include <..\src\mfc\afximpl.h>
#include "LastCommonRouteFinder.h"
#include "UploadBandwidthThrottler.h"
#include "ClientList.h"
#include "FriendList.h"
#include "ClientUDPSocket.h"
#include "DownloadQueue.h"
#include "IPFilter.h"
#include "MMServer.h"
#include "Statistics.h"
#include "OtherFunctions.h"
#include "WebServer.h"
#include "UploadQueue.h"
#include "SharedFileList.h"
#include "ServerList.h"
#include "Sockets.h"
#include "ListenSocket.h"
#include "ClientCredits.h"
#include "KnownFileList.h"
#include "Server.h"
#include "UpDownClient.h"
#include "ED2KLink.h"
#ifndef _CONSOLE
#include "emuleDlg.h"
#include "SearchDlg.h"
#include "enbitmap.h"
#endif

CLog theLog;
CLog theVerboseLog;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW


static CMemoryState oldMemState, newMemState, diffMemState;

_CRT_ALLOC_HOOK g_pfnPrevCrtAllocHook = NULL;
CMap<const unsigned char*, const unsigned char*, UINT, UINT> g_allocations;
int eMuleAllocHook(int mode, void* pUserData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* pszFileName, int nLine);

//CString _strCrtDebugReportFilePath(_T("eMule CRT Debug Log.txt"));
// don't use a CString for that memory - it will not be available on application termination!
#define APP_CRT_DEBUG_LOG_FILE _T("eMule CRT Debug Log.txt")
static TCHAR _szCrtDebugReportFilePath[MAX_PATH] = APP_CRT_DEBUG_LOG_FILE;
#endif

void CALLBACK myErrHandler(Kademlia::CKademliaError *error)
{
	CString msg;
	msg.Format(_T("\r\nError 0x%08X : %hs\r\n"), error->m_ErrorCode, error->m_ErrorDescription);
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.emuledlg->QueueDebugLogLine(false, msg);
}

void CALLBACK myDebugAndLogHandler(LPCSTR lpMsg)
{
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.emuledlg->QueueDebugLogLine(false, _T("%hs"), lpMsg);
}

void CALLBACK myLogHandler(LPCSTR lpMsg)
{
	if(theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.emuledlg->QueueLogLine(false, _T("%hs"), lpMsg);
}

const static UINT UWM_ARE_YOU_EMULE=RegisterWindowMessage(EMULE_GUID);

// CemuleApp

BEGIN_MESSAGE_MAP(CemuleApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


CemuleApp::CemuleApp(LPCTSTR lpszAppName)
	:CWinApp(lpszAppName)
{
	m_ullComCtrlVer = MAKEDLLVERULL(4,0,0,0);
	m_hSystemImageList = NULL;
	m_sizSmallSystemIcon.cx = 16;
	m_sizSmallSystemIcon.cy = 16;
	m_iDfltImageListColorFlags = ILC_COLOR;

// MOD Note: Do not change this part - Merkur

	// this is the "base" version number <major>.<minor>.<update>.<build>
	m_dwProductVersionMS = MAKELONG(VERSION_MIN, VERSION_MJR);
	m_dwProductVersionLS = MAKELONG(VERSION_BUILD, VERSION_UPDATE);

	// create a string version (e.g. "0.30a")
	ASSERT( VERSION_UPDATE + 'a' <= 'f' );
#ifdef _DEBUG
	m_strCurVersionLong.Format(_T("%u.%u%c.%u"), VERSION_MJR, VERSION_MIN, _T('a') + VERSION_UPDATE, VERSION_BUILD);
#else
	m_strCurVersionLong.Format(_T("%u.%u%c"), VERSION_MJR, VERSION_MIN, _T('a') + VERSION_UPDATE);
#endif

#ifdef _DUMP
	m_strCurVersionLong += _T(" DEBUG");
#endif

	// create the protocol version number
	CString strTmp;
	strTmp.Format(_T("0x%u"), m_dwProductVersionMS);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionShort) == 1 );
	ASSERT( m_uCurVersionShort < 0x99 );

	// create the version check number
	strTmp.Format(_T("0x%u%c"), m_dwProductVersionMS, _T('A') + VERSION_UPDATE);
	VERIFY( _stscanf(strTmp, _T("0x%x"), &m_uCurVersionCheck) == 1 );
	ASSERT( m_uCurVersionCheck < 0x999 );
// MOD Note: end

	m_bGuardClipboardPrompt = false;
}


CemuleApp theApp(_T("eMule"));


// Workaround for buggy 'AfxSocketTerm' (needed at least for MFC 7.0)
#if _MFC_VER==0x0700 || _MFC_VER==0x0710
void __cdecl __AfxSocketTerm()
{
#if defined(_AFXDLL) && (_MFC_VER==0x0700 || _MFC_VER==0x0710)
	VERIFY( WSACleanup() == 0 );
#else
	_AFX_SOCK_STATE* pState = _afxSockState.GetData();
	if (pState->m_pfnSockTerm != NULL){
		VERIFY( WSACleanup() == 0 );
		pState->m_pfnSockTerm = NULL;
	}
#endif
}
#else
#error "You are using an MFC version which may require a special version of the above function!"
#endif

// CemuleApp Initialisierung

BOOL CemuleApp::InitInstance()
{
	TCHAR szAppDir[MAX_PATH];
	VERIFY( GetModuleFileName(m_hInstance, szAppDir, ARRSIZE(szAppDir)) );
	VERIFY( PathRemoveFileSpec(szAppDir) );
	TCHAR szPrefFilePath[MAX_PATH];
	PathCombine(szPrefFilePath, szAppDir, CONFIGFOLDER _T("preferences.ini"));
	if (m_pszProfileName)
		free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(szPrefFilePath);

#ifdef _DEBUG
	oldMemState.Checkpoint();
	// Installing that memory debug code works fine in Debug builds when running within VS Debugger,
	// but some other test applications don't like that all....
	//g_pfnPrevCrtAllocHook = _CrtSetAllocHook(&eMuleAllocHook);
#endif
	//afxMemDF = allocMemDF | delayFreeMemDF;

#ifdef _DUMP
	MiniDumper dumper(m_strCurVersionLong);
#endif

	_tsetlocale(LC_ALL, _T(""));
	_tsetlocale(LC_NUMERIC, _T("C"));
	AfxOleInit();

	pendinglink = 0;
	if (ProcessCommandline())
		return false;
	// InitCommonControls() ist f�r Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder h�her zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	InitCommonControls();
	DWORD dwComCtrlMjr = 4;
	DWORD dwComCtrlMin = 0;
	AtlGetCommCtrlVersion(&dwComCtrlMjr, &dwComCtrlMin);
	m_ullComCtrlVer = MAKEDLLVERULL(dwComCtrlMjr,dwComCtrlMin,0,0);
	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);

	m_iDfltImageListColorFlags = GetAppImageListColorFlag();

	// don't use 32bit color resources if not supported by commctl
	if (m_iDfltImageListColorFlags == ILC_COLOR32 && m_ullComCtrlVer < MAKEDLLVERULL(6,0,0,0))
		m_iDfltImageListColorFlags = ILC_COLOR16;
	// don't use >8bit color resources with OSs with restricted memory for GDI resources
	if (afxData.bWin95)
		m_iDfltImageListColorFlags = ILC_COLOR8;

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(GetResString(IDS_SOCKETS_INIT_FAILED));
		return FALSE;
	}
#if _MFC_VER==0x0700 || _MFC_VER==0x0710
	atexit(__AfxSocketTerm);
#else
#error "You are using an MFC version which may require a special version of the above function!"
#endif
	AfxEnableControlContainer();
	if (!AfxInitRichEdit2()){
		if (!AfxInitRichEdit())
			AfxMessageBox(_T("No Rich Edit control library found!")); // should never happen..
	}
	// remove viruses which ddos us
	if (!CheckForSomeFoolVirus())
		return FALSE;

	// create & initalize all the important stuff 
	thePrefs.Init();
#ifdef _DEBUG
	_sntprintf(_szCrtDebugReportFilePath, ARRSIZE(_szCrtDebugReportFilePath), "%s\\%s", thePrefs.GetAppDir(), APP_CRT_DEBUG_LOG_FILE);
#endif
	VERIFY( theLog.SetFilePath(thePrefs.GetAppDir() + _T("eMule.log")) );
	VERIFY( theVerboseLog.SetFilePath(thePrefs.GetAppDir() + _T("eMule_Verbose.log")) );
	theLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	theVerboseLog.SetMaxFileSize(thePrefs.GetMaxLogFileSize());
	if (thePrefs.GetLog2Disk())
		theLog.Open();
	if (thePrefs.GetDebug2Disk())
		theVerboseLog.Open();

	CemuleDlg dlg;
	emuledlg = &dlg;
	m_pMainWnd = &dlg;

	// Barry - Auto-take ed2k links
	if (thePrefs.AutoTakeED2KLinks())
		Ask4RegFix(false, true);

	// ZZ:UploadSpeedSense -->
    lastCommonRouteFinder = new LastCommonRouteFinder();
    uploadBandwidthThrottler = new UploadBandwidthThrottler();
	// ZZ:UploadSpeedSense <--

	clientlist = new CClientList();
	friendlist = new CFriendList();
	searchlist = new CSearchList();
	knownfiles = new CKnownFileList();
	serverlist = new CServerList();
	serverconnect = new CServerConnect(serverlist);
	sharedfiles = new CSharedFileList(serverconnect);
	listensocket = new CListenSocket();
	clientudp	= new CClientUDPSocket();
	clientcredits = new CClientCreditsList();
	downloadqueue = new CDownloadQueue(sharedfiles);	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	mmserver = new CMMServer();
	scheduler = new CScheduler();
	statistics = new CStatistics();
	
	// reset statistic values
	theApp.stat_sessionReceivedBytes=0;
	theApp.stat_sessionSentBytes=0;
	theApp.stat_reconnects=0;
	theApp.stat_transferStarttime=0;
	theApp.stat_serverConnectTime=0;
	theApp.stat_filteredclients=0;
	thePerfLog.Startup();
	dlg.DoModal();

	::CloseHandle(m_hMutexOneInstance);
#ifdef _DEBUG
	if (g_pfnPrevCrtAllocHook)
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);

	newMemState.Checkpoint();
	if (diffMemState.Difference(oldMemState, newMemState))
	{
		TRACE("Memory usage:\n");
		diffMemState.DumpStatistics();
	}
	//_CrtDumpMemoryLeaks();
#endif //_DEBUG

	emuledlg = NULL;
	return FALSE;
}

#ifdef _DEBUG
int CrtDebugReportCB(int reportType, char* message, int* returnValue)
{
	FILE* fp = fopen(_szCrtDebugReportFilePath, "a");
	if (fp){
		time_t tNow = time(NULL);
		char szTime[40];
		strftime(szTime, ARRSIZE(szTime), "%H:%M:%S", localtime(&tNow));
		fprintf(fp, "%s  %u  %s", szTime, reportType, message);
		fclose(fp);
	}
	*returnValue = 0; // avoid invokation of 'AfxDebugBreak' in ASSERT macros
	return TRUE; // avoid further processing of this debug report message by the CRT
}

// allocation hook - for memory statistics gatering
int eMuleAllocHook(int mode, void* pUserData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* pszFileName, int nLine)
{
	UINT count = 0;
	g_allocations.Lookup(pszFileName, count);
	if (mode == _HOOK_ALLOC) {
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);
		g_allocations.SetAt(pszFileName, count + 1);
		_CrtSetAllocHook(&eMuleAllocHook);
	}
	else if (mode == _HOOK_FREE){
		_CrtSetAllocHook(g_pfnPrevCrtAllocHook);
		g_allocations.SetAt(pszFileName, count - 1);
		_CrtSetAllocHook(&eMuleAllocHook);
	}
	return g_pfnPrevCrtAllocHook(mode, pUserData, nSize, nBlockUse, lRequest, pszFileName, nLine);
}
#endif

bool CemuleApp::ProcessCommandline()
{
	bool bIgnoreRunningInstances = (GetProfileInt(_T("eMule"), _T("IgnoreInstances"), 0) != 0);

	for (int i = 1; i < __argc; i++){
		LPCTSTR pszParam = __targv[i];
		if (pszParam[0] == _T('-') || pszParam[0] == _T('/')){
			pszParam++;
#ifdef _DEBUG
			if (_tcscmp(pszParam, _T("assertfile")) == 0)
				_CrtSetReportHook(CrtDebugReportCB);
#endif
			if (_tcscmp(pszParam, _T("ignoreinstances")) == 0)
				bIgnoreRunningInstances = true;
		}
	}

	CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    
	// If we create our TCP listen socket with SO_REUSEADDR, we have to ensure that there are
	// not 2 emules are running using the same port.
	// NOTE: This will not prevent from some other application using that port!
	UINT uTcpPort = GetProfileInt(_T("eMule"), _T("Port"), DEFAULT_TCP_PORT);
	CString strMutextName;
	strMutextName.Format(_T("%s:%u"), EMULE_GUID, uTcpPort);
	m_hMutexOneInstance = ::CreateMutex(NULL, FALSE, strMutextName);
	
	HWND maininst = NULL;
	bool bAlreadyRunning = false;
	if (!bIgnoreRunningInstances){
		bAlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS ||::GetLastError() == ERROR_ACCESS_DENIED);
    	if ( bAlreadyRunning ) EnumWindows(SearchEmuleWindow, (LPARAM)&maininst);
	}

    if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
		CString command = cmdInfo.m_strFileName;
		if (command.Find("://")>0) {
			sendstruct.cbData = command.GetLength()+1; 
			sendstruct.dwData = OP_ED2KLINK; 
			sendstruct.lpData = command.GetBuffer(); 
    		if (maininst){
      			SendMessage(maininst,WM_COPYDATA,(WPARAM)0,(LPARAM) (PCOPYDATASTRUCT) &sendstruct); 
      			return true; 
			} 
    		else 
      			pendinglink = new CString(command);
		} else {
			sendstruct.cbData = command.GetLength()+1; 
			sendstruct.dwData = OP_CLCOMMAND;
			sendstruct.lpData = command.GetBuffer(); 
    		if (maininst){
      			SendMessage(maininst,WM_COPYDATA,(WPARAM)0,(LPARAM) (PCOPYDATASTRUCT) &sendstruct); 
      			return true; 
			}
		}
    }
    return (maininst || bAlreadyRunning);
}

BOOL CALLBACK CemuleApp::SearchEmuleWindow(HWND hWnd, LPARAM lParam){
	DWORD dwMsgResult;
	LRESULT res = ::SendMessageTimeout(hWnd,UWM_ARE_YOU_EMULE,0, 0,SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);
	if(res == 0)
		return TRUE;
	if(dwMsgResult == UWM_ARE_YOU_EMULE){ 
		HWND * target = (HWND *)lParam;
		*target = hWnd;
		return FALSE; 
	} 
	return TRUE; 
} 


void CemuleApp::UpdateReceivedBytes(uint32 bytesToAdd) {
	SetTimeOnTransfer();
	stat_sessionReceivedBytes+=bytesToAdd;
}

void CemuleApp::UpdateSentBytes(uint32 bytesToAdd) {
	SetTimeOnTransfer();
	stat_sessionSentBytes+=bytesToAdd;
}

void CemuleApp::SetTimeOnTransfer() {
	if (stat_transferStarttime>0) return;
	
	stat_transferStarttime=GetTickCount();
}

CString CemuleApp::CreateED2kSourceLink(const CAbstractFile* f)
{
	if (!IsConnected() || IsFirewalled()){
		AddLogLine(true,GetResString(IDS_SOURCELINKFAILED));
		return CString("");
	}
	uint32 dwID = GetID();

	CString strLink;
	strLink.Format("ed2k://|file|%s|%u|%s|/|sources,%i.%i.%i.%i:%i|/",
		StripInvalidFilenameChars(f->GetFileName(), false),	// spaces to dots
		f->GetFileSize(),
		EncodeBase16(f->GetFileHash(),16),
		(uint8)dwID,(uint8)(dwID>>8),(uint8)(dwID>>16),(uint8)(dwID>>24), thePrefs.GetPort() );
	return strLink;
}

CString CemuleApp::CreateED2kHostnameSourceLink(const CAbstractFile* f)
{
	CString strLink;
	strLink.Format("ed2k://|file|%s|%u|%s|/|sources,%s:%i|/",
		StripInvalidFilenameChars(f->GetFileName(), false),	// spaces to dots
		f->GetFileSize(),
		EncodeBase16(f->GetFileHash(),16),
		thePrefs.GetYourHostname(), thePrefs.GetPort() );
	return strLink;
}

//TODO: Move to emule-window
bool CemuleApp::CopyTextToClipboard( CString strText )
{
	if (strText.IsEmpty())
		return false;

	//allocate global memory & lock it
	HGLOBAL hGlobal = GlobalAlloc(GHND|GMEM_SHARE,strText.GetLength() + 1);
	if(hGlobal == NULL)
		return false;

	PTSTR pGlobal = static_cast<PTSTR>(GlobalLock(hGlobal));
	if( pGlobal == NULL ){
		GlobalFree(hGlobal);
		return false;
	}

	//copy the text
	strcpy(pGlobal,(LPCTSTR)strText);
	GlobalUnlock(hGlobal);

	//Open the Clipboard and insert the handle into the global memory
	bool bResult = false;
	if( OpenClipboard(NULL) )
	{
		if( EmptyClipboard() )
			bResult = (SetClipboardData(CF_TEXT,hGlobal) != NULL);
		CloseClipboard();
	}
	if (bResult)
		IgnoreClipboardLinks(strText); // this is so eMule won't think the clipboard has ed2k links for adding
	else
		GlobalFree(hGlobal);
	return bResult;
}

//TODO: Move to emule-window
CString CemuleApp::CopyTextFromClipboard() 
{
	HGLOBAL	hglb; 
	LPTSTR  lptstr; 
	CString	retstring;

	if (!IsClipboardFormatAvailable(CF_TEXT)) 
		return ""; 
	if (!OpenClipboard(NULL)) 
		return ""; 

	hglb = GetClipboardData(CF_TEXT); 
	if (hglb != NULL) 
	{ 
		lptstr = (LPTSTR)GlobalLock(hglb); 
		if (lptstr != NULL)
			retstring = lptstr;
	} 
	CloseClipboard();

	return retstring;
}

void CemuleApp::OnlineSig() // Added By Bouc7 
{ 
	if (!thePrefs.IsOnlineSignatureEnabled()) return;

    char* fullpath = new char[strlen(thePrefs.GetAppDir())+MAX_PATH]; 
    sprintf(fullpath,"%sonlinesig.dat",thePrefs.GetAppDir()); 
    CFile file; 
    if (!file.Open(fullpath,CFile::modeCreate|CFile::modeReadWrite)){ 
		AddLogLine(true,GetResString(IDS_ERROR_SAVEFILE)+CString(" OnlineSig.dat"));
	    delete[] fullpath; 
		return;
    } 
    char buffer[20]; 
	CString Kad;
	if (IsConnected()){ 
		file.Write("1",1); 
		file.Write("|",1);
		if(serverconnect->IsConnected())
			file.Write(serverconnect->GetCurrentServer()->GetListName(),strlen(serverconnect->GetCurrentServer()->GetListName())); 
      // Not : file.Write(serverconnect->GetCurrentServer()->GetListName(),strlen(serverconnect->GetCurrentServer()- >GetRealName())); 
		else{
			Kad = "Kademlia";
			file.Write(Kad,Kad.GetLength()); 
		}

		file.Write("|",1); 
		if(serverconnect->IsConnected())
			file.Write(serverconnect->GetCurrentServer()->GetFullIP(),strlen(serverconnect->GetCurrentServer()->GetFullIP())); 
		else{
			Kad = "0.0.0.0";
			file.Write(Kad,Kad.GetLength()); 
		}
		file.Write("|",1); 
		if(serverconnect->IsConnected()){
			itoa(serverconnect->GetCurrentServer()->GetPort(),buffer,10); 
			file.Write(buffer,strlen(buffer));
		}
		else{
			Kad = "0";
			file.Write(Kad,Kad.GetLength());
		}
	} 
    else 
      file.Write("0",1); 

    file.Write("\n",1); 
    sprintf(buffer,"%.1f",(float)downloadqueue->GetDatarate()/1024); 
    file.Write(buffer,strlen(buffer)); 
    file.Write("|",1); 
    sprintf(buffer,"%.1f",(float)uploadqueue->GetDatarate()/1024); 
    file.Write(buffer,strlen(buffer)); 
    file.Write("|",1); 
    itoa(uploadqueue->GetWaitingUserCount(),buffer,10); 
    file.Write(buffer,strlen(buffer)); 

    file.Close(); 
    delete[] fullpath; 
    fullpath=NULL;
} //End Added By Bouc7

void CemuleApp::OnHelp() {

	// Change extension for help file
	CString strHelpFile = m_pszHelpFilePath;
	CFileFind ff;

	if (thePrefs.GetLanguageID()!=MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT)) {
		int pos=strHelpFile.ReverseFind('.');
		CString temp;
		temp.Format("%s.%u.chm",strHelpFile.Left(pos),thePrefs.GetLanguageID());
		if (pos>0) strHelpFile=temp;
		
		// if not exists, use original help (english)
		if (!ff.FindFile(strHelpFile, 0)) strHelpFile = m_pszHelpFilePath;
	}
	ff.Close();
	strHelpFile.Replace(".HLP", ".chm");

	// lets just open the helpfile by associated program, instead of more windows-dependency :)
	if (ff.FindFile(strHelpFile, 0)) ShellOpenFile(strHelpFile); 
	else
		if (IDYES==AfxMessageBox(strHelpFile + _T("\n\n") + GetResString(IDS_ERR_NOHELP), MB_YESNO | MB_ICONERROR) ){
			CString strUrl = thePrefs.GetHomepageBaseURL() + CString("/home/perl/help.cgi");
			ShellExecute(NULL, NULL, strUrl, NULL, thePrefs.GetAppDir(), SW_SHOWDEFAULT);

		}
}

int CemuleApp::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength /* = -1 */)
{
	//TODO: This has to be MBCS aware..
	DWORD dwFileAttributes;
	LPCTSTR pszCacheExt = NULL;
	if (iLength == -1)
		iLength = _tcslen(pszFilePath);
	if (iLength > 0 && (pszFilePath[iLength - 1] == _T('\\') || pszFilePath[iLength - 1] == _T('/'))){
		// it's a directory
		pszCacheExt = _T("\\");
		dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	}
	else{
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		// search last '.' character *after* the last '\\' character
		for (int i = iLength - 1; i >= 0; i--){
			if (pszFilePath[i] == _T('\\') || pszFilePath[i] == _T('/'))
				break;
			if (pszFilePath[i] == _T('.')) {
				// point to 1st character of extension (skip the '.')
				pszCacheExt = &pszFilePath[i+1];
				break;
			}
		}
		if (pszCacheExt == NULL)
			pszCacheExt = _T("");	// empty extension
	}

	// Search extension in "ext->idx" cache.
	LPVOID vData;
	if (!m_aExtToSysImgIdx.Lookup(pszCacheExt, vData)){
		// Get index for the system's small icon image list
		SHFILEINFO sfi;
		DWORD dwResult = SHGetFileInfo(pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
									   SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		if (dwResult == 0)
			return 0;
		ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
		m_hSystemImageList = (HIMAGELIST)dwResult;

		// Store icon index in local cache
		m_aExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);
		return sfi.iIcon;
	}
	// Return already cached value
	// Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}

bool CemuleApp::IsConnected()
{
	//Once we can handle lowID users in Kad, we remove the IsFirewalled!
	return (theApp.serverconnect->IsConnected() || (Kademlia::CKademlia::isConnected() && !Kademlia::CKademlia::isFirewalled()));
}

uint32 CemuleApp::GetID(){
	uint32 ID;
	if( Kademlia::CKademlia::isConnected() && !Kademlia::CKademlia::isFirewalled() )
		ID = ntohl(Kademlia::CKademlia::getIPAddress());
	else if( theApp.serverconnect->IsConnected() )
		ID = theApp.serverconnect->GetClientID();
	else 
		ID = 0; //Once we can handle lowID users in Kad, this may change.
	return ID;
}

bool CemuleApp::IsFirewalled()
{
	if (theApp.serverconnect->IsConnected() && !theApp.serverconnect->IsLowID())
		return false; // we have an eD2K HighID -> not firewalled

	if (Kademlia::CKademlia::isConnected() && !Kademlia::CKademlia::isFirewalled())
		return false; // we have an Kad HighID -> not firewalled

	return true; // firewalled
}

bool CemuleApp::DoCallback( CUpDownClient *client )
{
	if(Kademlia::CKademlia::isConnected())
	{
		if(theApp.serverconnect->IsConnected())
		{
			if(theApp.serverconnect->IsLowID())
			{
				if(Kademlia::CKademlia::isFirewalled())
				{
					//Both Connected - Both Firewalled
					return false;
				}
				else
				{
					if(client->GetServerIP() == theApp.serverconnect->GetCurrentServer()->GetIP() && client->GetServerPort() == theApp.serverconnect->GetCurrentServer()->GetPort())
					{
						//Both Connected - Server lowID, Kad Open - Client on same server
						return false;
					}
					else
					{
						//Both Connected - Server lowID, Kad Open - Client on remote server
						return true;
					}
				}
			}
			else
			{
				//Both Connected - Server HighID, Kad don't care
				return true;
			}
		}
		else
		{
			if(Kademlia::CKademlia::isFirewalled())
			{
				//Only Kad Connected - Kad Firewalled
				return false;
			}
			else
			{
				//Only Kad Conected - Kad Open
				return true;
			}
		}
	}
	else
	{
		if( theApp.serverconnect->IsConnected() )
		{
			if( theApp.serverconnect->IsLowID() )
			{
				//Only Server Connected - Server LowID
				return false;
			}
			else
			{
				//Only Server Connected - Server HighID
				return true;
			}
		}
		else
		{
			//We are not connected at all!
			return false;
		}
	}
}

HICON CemuleApp::LoadIcon(UINT nIDResource) const
{
	// use string resource identifiers!!
	ASSERT(0);
	return CWinApp::LoadIcon(nIDResource);
}

HICON CemuleApp::LoadIcon(LPCTSTR lpszResourceName, int cx, int cy, UINT uFlags) const
{
	HICON hIcon = NULL;
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		// load icon resource file specification from skin profile
		TCHAR szSkinResource[MAX_PATH];
		GetPrivateProfileString(_T("Icons"), lpszResourceName, _T(""), szSkinResource, ARRSIZE(szSkinResource), pszSkinProfile);
		if (szSkinResource[0] != _T('\0'))
		{
			// expand any optional available environment strings
			TCHAR szExpSkinRes[MAX_PATH];
			if (ExpandEnvironmentStrings(szSkinResource, szExpSkinRes, ARRSIZE(szExpSkinRes)) != 0)
			{
				_tcsncpy(szSkinResource, szExpSkinRes, ARRSIZE(szSkinResource));
				szSkinResource[ARRSIZE(szSkinResource)-1] = _T('\0');
			}

			// create absolute path to icon resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, ARRSIZE(szSkinResFolder));
				szSkinResFolder[ARRSIZE(szSkinResFolder)-1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepath(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, ARRSIZE(szFullResPath));
				szFullResPath[ARRSIZE(szFullResPath)-1] = _T('\0');
			}

			// check for optional icon index or resource identifier within the icon resource file
			bool bExtractIcon = false;
			CString strFullResPath = szFullResPath;
			int iIconIndex = 0;
			int iComma = strFullResPath.ReverseFind(_T(','));
			if (iComma != -1){
				if (_stscanf((LPCTSTR)strFullResPath + iComma + 1, _T("%d"), &iIconIndex) == 1)
					bExtractIcon = true;
				strFullResPath = strFullResPath.Left(iComma);
			}

			if (bExtractIcon)
			{
				HICON aIconsLarge[1] = {0};
				HICON aIconsSmall[1] = {0};
				int iExtractedIcons = ExtractIconEx(strFullResPath, iIconIndex, aIconsLarge, aIconsSmall, 1);
				if (iExtractedIcons > 0) // 'iExtractedIcons' is 2(!) if we get a large and a small icon
				{
					// alway try to return the icon size which was requested
					if (cx == 16 && aIconsSmall[0] != NULL)
					{
						hIcon = aIconsSmall[0];
						aIconsSmall[0] = NULL;
					}
					else if (cx == 32 && aIconsLarge[0] != NULL)
					{
						hIcon = aIconsLarge[0];
						aIconsLarge[0] = NULL;
					}
					else
					{
						if (aIconsSmall[0] != NULL)
						{
							hIcon = aIconsSmall[0];
							aIconsSmall[0] = NULL;
						}
						else if (aIconsLarge[0] != NULL)
						{
							hIcon = aIconsLarge[0];
							aIconsLarge[0] = NULL;
						}
					}

					for (int i = 0; i < ARRSIZE(aIconsLarge); i++)
					{
						if (aIconsLarge[i] != NULL)
							VERIFY( DestroyIcon(aIconsLarge[i]) );
						if (aIconsSmall[i] != NULL)
							VERIFY( DestroyIcon(aIconsSmall[i]) );
					}
				}
			}
			else
			{
				// WINBUG???: 'ExtractIcon' does not work well on ICO-files when using the color 
				// scheme 'Windows-Standard (extragro�)' -> always try to use 'LoadImage'!
				//
				// If the ICO file contains a 16x16 icon, 'LoadImage' will though return a 32x32 icon,
				// if LR_DEFAULTSIZE is specified! -> always specify the requested size!
				hIcon = (HICON)::LoadImage(NULL, szFullResPath, IMAGE_ICON, cx, cy, uFlags | LR_LOADFROMFILE);
			}
		}
	}

	if (hIcon == NULL)
	{
		if (cx != LR_DEFAULTSIZE || cy != LR_DEFAULTSIZE || uFlags != LR_DEFAULTCOLOR)
			hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), lpszResourceName, IMAGE_ICON, cx, cy, uFlags);
		if (hIcon == NULL)
			hIcon = CWinApp::LoadIcon(lpszResourceName);
	}
	return hIcon;
}

HBITMAP CemuleApp::LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const
{
	HBITMAP hBmp = NULL;
	LPCTSTR pszSkinProfile = thePrefs.GetSkinProfile();
	if (pszSkinProfile != NULL && pszSkinProfile[0] != _T('\0'))
	{
		// load resource file specification from skin profile
		TCHAR szSkinResource[MAX_PATH];
		GetPrivateProfileString(_T("Bitmaps"), lpszResourceName, _T(""), szSkinResource, ARRSIZE(szSkinResource), pszSkinProfile);
		if (szSkinResource[0] != _T('\0'))
		{
			// expand any optional available environment strings
			TCHAR szExpSkinRes[MAX_PATH];
			if (ExpandEnvironmentStrings(szSkinResource, szExpSkinRes, ARRSIZE(szExpSkinRes)) != 0)
			{
				_tcsncpy(szSkinResource, szExpSkinRes, ARRSIZE(szSkinResource));
				szSkinResource[ARRSIZE(szSkinResource)-1] = _T('\0');
			}

			// create absolute path to resource file
			TCHAR szFullResPath[MAX_PATH];
			if (PathIsRelative(szSkinResource))
			{
				TCHAR szSkinResFolder[MAX_PATH];
				_tcsncpy(szSkinResFolder, pszSkinProfile, ARRSIZE(szSkinResFolder));
				szSkinResFolder[ARRSIZE(szSkinResFolder)-1] = _T('\0');
				PathRemoveFileSpec(szSkinResFolder);
				_tmakepath(szFullResPath, NULL, szSkinResFolder, szSkinResource, NULL);
			}
			else
			{
				_tcsncpy(szFullResPath, szSkinResource, ARRSIZE(szFullResPath));
				szFullResPath[ARRSIZE(szFullResPath)-1] = _T('\0');
			}

			CEnBitmap bmp;
			if (bmp.LoadImage(szFullResPath))
				return (HBITMAP)bmp.Detach();
		}
	}

	CEnBitmap bmp;
	if (bmp.LoadImage(lpszResourceName, pszResourceType))
		return (HBITMAP)bmp.Detach();
	return NULL;
}

void CemuleApp::ApplySkin(LPCTSTR pszSkinProfile)
{
	thePrefs.SetSkinProfile(pszSkinProfile);
	AfxGetMainWnd()->SendMessage(WM_SYSCOLORCHANGE);
}

CTempIconLoader::CTempIconLoader(LPCTSTR pszResourceID, int cx, int cy, UINT uFlags)
{
	m_hIcon = theApp.LoadIcon(pszResourceID, cx, cy, uFlags);
}

CTempIconLoader::~CTempIconLoader()
{
	if (m_hIcon)
		VERIFY( DestroyIcon(m_hIcon) );
}

void CemuleApp::AddEd2kLinksToDownload(CString strlink, uint8 cat)
{
	int curPos = 0;
	CString resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
	while (resToken != _T(""))
	{
		if (resToken.Right(1) != _T("/"))
			resToken += _T("/");
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(resToken.Trim());
			if (pLink)
			{
				if (pLink->GetKind() == CED2KLink::kFile)
				{
					downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat);
				}
				else
				{
					delete pLink;
					throw CString(_T("bad link"));
				}
				delete pLink;
			}
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_snprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			AddLogLine(true, GetResString(IDS_ERR_LINKERROR), szBuffer);
		}
		resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
	}
}

void CemuleApp::SearchClipboard()
{
	if (m_bGuardClipboardPrompt)
		return;

	CString strLinks = CopyTextFromClipboard();
	if (strLinks.IsEmpty())
		return;

	if (strLinks.Compare(m_strLastClipboardContents) == 0)
		return;

	if (strLinks.Left(13).CompareNoCase(_T("ed2k://|file|")) == 0)
	{
		m_bGuardClipboardPrompt = true;
		if (AfxMessageBox(GetResString(IDS_ED2KLINKFIX) + _T("\r\n\r\n") + GetResString(IDS_ADDDOWNLOADSFROMCB)+_T("\r\n") + strLinks, MB_YESNO | MB_TOPMOST) == IDYES)
			AddEd2kLinksToDownload(strLinks, 0);
	}
	m_strLastClipboardContents = strLinks;
	m_bGuardClipboardPrompt = false;
}

void CemuleApp::PasteClipboard()
{
	CString strLinks = CopyTextFromClipboard();
	strLinks.Trim();
	if (strLinks.IsEmpty())
		return;

	AddEd2kLinksToDownload(strLinks, 0);
}

bool CemuleApp::IsEd2kLinkInClipboard(LPCTSTR pszLinkType, int iLinkTypeLen)
{
	bool bFoundLink = false;
	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if (OpenClipboard(NULL))
		{
			HGLOBAL	hText = GetClipboardData(CF_TEXT);
			if (hText != NULL)
			{ 
				LPCTSTR pszText = (LPCTSTR)GlobalLock(hText);
				if (pszText != NULL)
				{
					while (*pszText == _T(' ') || *pszText == _T('\t') || *pszText == _T('\r') || *pszText == _T('\n'))
						pszText++;
					bFoundLink = (_tcsncmp(pszText, pszLinkType, iLinkTypeLen) == 0);
				}
			}
			CloseClipboard();
		}
	}

	return bFoundLink;
}

bool CemuleApp::IsEd2kFileLinkInClipboard()
{
	static const TCHAR _szEd2kFileLink[] = _T("ed2k://|file|");
	return IsEd2kLinkInClipboard(_szEd2kFileLink, ARRSIZE(_szEd2kFileLink)-1);
}

bool CemuleApp::IsEd2kServerLinkInClipboard()
{
	static const TCHAR _szEd2kServerLink[] = _T("ed2k://|server|");
	return IsEd2kLinkInClipboard(_szEd2kServerLink, ARRSIZE(_szEd2kServerLink)-1);
}