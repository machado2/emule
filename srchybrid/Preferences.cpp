//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

#include "stdafx.h"
#include <io.h>
#include <share.h>
#include "emule.h"
#include "Preferences.h"
#include "Opcodes.h"
#include "OtherFunctions.h"
#include "Ini2.h"
#include "DownloadQueue.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "MD5Sum.h"
#include "PartFile.h"
#include "Sockets.h"
#include "ListenSocket.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "SafeFile.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "Log.h"
#include "MuleToolbarCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPreferences thePrefs;

int		CPreferences::m_iDbgHeap;
CString	CPreferences::strNick;
uint16	CPreferences::minupload;
uint16	CPreferences::maxupload;
uint16	CPreferences::maxdownload;
uint16	CPreferences::port;
uint16	CPreferences::udpport;
uint16	CPreferences::nServerUDPPort;
uint16	CPreferences::maxconnections;
uint16	CPreferences::maxhalfconnections;
bool	CPreferences::m_bConditionalTCPAccept;
bool	CPreferences::reconnect;
bool	CPreferences::m_bUseServerPriorities;
TCHAR	CPreferences::incomingdir[MAX_PATH];
CStringArray CPreferences::tempdir;
bool	CPreferences::ICH;
bool	CPreferences::m_bAutoUpdateServerList;
bool	CPreferences::updatenotify;
bool	CPreferences::mintotray;
bool	CPreferences::autoconnect;
bool	CPreferences::m_bAutoConnectToStaticServersOnly;
bool	CPreferences::autotakeed2klinks;
bool	CPreferences::addnewfilespaused;
uint8	CPreferences::depth3D;
bool	CPreferences::m_bEnableMiniMule;
int		CPreferences::m_iStraightWindowStyles;
bool	CPreferences::m_bRTLWindowsLayout;
CString	CPreferences::m_strSkinProfile;
CString	CPreferences::m_strSkinProfileDir;
bool	CPreferences::m_bAddServersFromServer;
bool	CPreferences::m_bAddServersFromClients;
uint16	CPreferences::maxsourceperfile;
uint16	CPreferences::trafficOMeterInterval;
uint16	CPreferences::statsInterval;
uchar	CPreferences::userhash[16];
WINDOWPLACEMENT CPreferences::EmuleWindowPlacement;
int		CPreferences::maxGraphDownloadRate;
int		CPreferences::maxGraphUploadRate;
uint32	CPreferences::maxGraphUploadRateEstimated = 0;
bool	CPreferences::beepOnError;
bool	CPreferences::confirmExit;
DWORD	CPreferences::m_adwStatsColors[15];
bool	CPreferences::splashscreen;
bool	CPreferences::filterLANIPs;
bool	CPreferences::m_bAllocLocalHostIP;
bool	CPreferences::onlineSig;
uint64	CPreferences::cumDownOverheadTotal;
uint64	CPreferences::cumDownOverheadFileReq;
uint64	CPreferences::cumDownOverheadSrcEx;
uint64	CPreferences::cumDownOverheadServer;
uint64	CPreferences::cumDownOverheadKad;
uint64	CPreferences::cumDownOverheadTotalPackets;
uint64	CPreferences::cumDownOverheadFileReqPackets;
uint64	CPreferences::cumDownOverheadSrcExPackets;
uint64	CPreferences::cumDownOverheadServerPackets;
uint64	CPreferences::cumDownOverheadKadPackets;
uint64	CPreferences::cumUpOverheadTotal;
uint64	CPreferences::cumUpOverheadFileReq;
uint64	CPreferences::cumUpOverheadSrcEx;
uint64	CPreferences::cumUpOverheadServer;
uint64	CPreferences::cumUpOverheadKad;
uint64	CPreferences::cumUpOverheadTotalPackets;
uint64	CPreferences::cumUpOverheadFileReqPackets;
uint64	CPreferences::cumUpOverheadSrcExPackets;
uint64	CPreferences::cumUpOverheadServerPackets;
uint64	CPreferences::cumUpOverheadKadPackets;
uint32	CPreferences::cumUpSuccessfulSessions;
uint32	CPreferences::cumUpFailedSessions;
uint32	CPreferences::cumUpAvgTime;
uint64	CPreferences::cumUpData_EDONKEY;
uint64	CPreferences::cumUpData_EDONKEYHYBRID;
uint64	CPreferences::cumUpData_EMULE;
uint64	CPreferences::cumUpData_MLDONKEY;
uint64	CPreferences::cumUpData_AMULE;
uint64	CPreferences::cumUpData_EMULECOMPAT;
uint64	CPreferences::cumUpData_SHAREAZA;
uint64	CPreferences::sesUpData_EDONKEY;
uint64	CPreferences::sesUpData_EDONKEYHYBRID;
uint64	CPreferences::sesUpData_EMULE;
uint64	CPreferences::sesUpData_MLDONKEY;
uint64	CPreferences::sesUpData_AMULE;
uint64	CPreferences::sesUpData_EMULECOMPAT;
uint64	CPreferences::sesUpData_SHAREAZA;
uint64	CPreferences::cumUpDataPort_4662;
uint64	CPreferences::cumUpDataPort_OTHER;
uint64	CPreferences::cumUpDataPort_PeerCache;
uint64	CPreferences::sesUpDataPort_4662;
uint64	CPreferences::sesUpDataPort_OTHER;
uint64	CPreferences::sesUpDataPort_PeerCache;
uint64	CPreferences::cumUpData_File;
uint64	CPreferences::cumUpData_Partfile;
uint64	CPreferences::sesUpData_File;
uint64	CPreferences::sesUpData_Partfile;
uint32	CPreferences::cumDownCompletedFiles;
uint32	CPreferences::cumDownSuccessfulSessions;
uint32	CPreferences::cumDownFailedSessions;
uint32	CPreferences::cumDownAvgTime;
uint64	CPreferences::cumLostFromCorruption;
uint64	CPreferences::cumSavedFromCompression;
uint32	CPreferences::cumPartsSavedByICH;
uint32	CPreferences::sesDownSuccessfulSessions;
uint32	CPreferences::sesDownFailedSessions;
uint32	CPreferences::sesDownAvgTime;
uint32	CPreferences::sesDownCompletedFiles;
uint64	CPreferences::sesLostFromCorruption;
uint64	CPreferences::sesSavedFromCompression;
uint32	CPreferences::sesPartsSavedByICH;
uint64	CPreferences::cumDownData_EDONKEY;
uint64	CPreferences::cumDownData_EDONKEYHYBRID;
uint64	CPreferences::cumDownData_EMULE;
uint64	CPreferences::cumDownData_MLDONKEY;
uint64	CPreferences::cumDownData_AMULE;
uint64	CPreferences::cumDownData_EMULECOMPAT;
uint64	CPreferences::cumDownData_SHAREAZA;
uint64	CPreferences::cumDownData_URL;
uint64	CPreferences::sesDownData_EDONKEY;
uint64	CPreferences::sesDownData_EDONKEYHYBRID;
uint64	CPreferences::sesDownData_EMULE;
uint64	CPreferences::sesDownData_MLDONKEY;
uint64	CPreferences::sesDownData_AMULE;
uint64	CPreferences::sesDownData_EMULECOMPAT;
uint64	CPreferences::sesDownData_SHAREAZA;
uint64	CPreferences::sesDownData_URL;
uint64	CPreferences::cumDownDataPort_4662;
uint64	CPreferences::cumDownDataPort_OTHER;
uint64	CPreferences::cumDownDataPort_PeerCache;
uint64	CPreferences::sesDownDataPort_4662;
uint64	CPreferences::sesDownDataPort_OTHER;
uint64	CPreferences::sesDownDataPort_PeerCache;
float	CPreferences::cumConnAvgDownRate;
float	CPreferences::cumConnMaxAvgDownRate;
float	CPreferences::cumConnMaxDownRate;
float	CPreferences::cumConnAvgUpRate;
float	CPreferences::cumConnMaxAvgUpRate;
float	CPreferences::cumConnMaxUpRate;
uint64	CPreferences::cumConnRunTime;
uint32	CPreferences::cumConnNumReconnects;
uint32	CPreferences::cumConnAvgConnections;
uint32	CPreferences::cumConnMaxConnLimitReached;
uint32	CPreferences::cumConnPeakConnections;
uint32	CPreferences::cumConnTransferTime;
uint32	CPreferences::cumConnDownloadTime;
uint32	CPreferences::cumConnUploadTime;
uint32	CPreferences::cumConnServerDuration;
uint32	CPreferences::cumSrvrsMostWorkingServers;
uint32	CPreferences::cumSrvrsMostUsersOnline;
uint32	CPreferences::cumSrvrsMostFilesAvail;
uint32	CPreferences::cumSharedMostFilesShared;
uint64	CPreferences::cumSharedLargestShareSize;
uint64	CPreferences::cumSharedLargestAvgFileSize;
uint64	CPreferences::cumSharedLargestFileSize;
__int64 CPreferences::stat_datetimeLastReset;
uint8	CPreferences::statsConnectionsGraphRatio;
UINT	CPreferences::statsSaveInterval;
TCHAR	CPreferences::statsExpandedTreeItems[256];
bool	CPreferences::m_bShowVerticalHourMarkers;
uint64	CPreferences::totalDownloadedBytes;
uint64	CPreferences::totalUploadedBytes;
WORD	CPreferences::m_wLanguageID;
bool	CPreferences::transferDoubleclick;
EViewSharedFilesAccess CPreferences::m_iSeeShares;
UINT	CPreferences::m_iToolDelayTime;
bool	CPreferences::bringtoforeground;
UINT	CPreferences::splitterbarPosition;
UINT	CPreferences::splitterbarPositionSvr;
UINT	CPreferences::splitterbarPositionStat;
UINT	CPreferences::splitterbarPositionStat_HL;
UINT	CPreferences::splitterbarPositionStat_HR;
UINT	CPreferences::splitterbarPositionFriend;
UINT	CPreferences::splitterbarPositionIRC;
UINT	CPreferences::splitterbarPositionShared;
UINT	CPreferences::m_uTransferWnd1;
UINT	CPreferences::m_uTransferWnd2;
UINT	CPreferences::m_uDeadServerRetries;
DWORD	CPreferences::m_dwServerKeepAliveTimeout;
uint16	CPreferences::statsMax;
uint8	CPreferences::statsAverageMinutes;
CString	CPreferences::notifierConfiguration;
bool	CPreferences::notifierOnDownloadFinished;
bool	CPreferences::notifierOnNewDownload;
bool	CPreferences::notifierOnChat;
bool	CPreferences::notifierOnLog;
bool	CPreferences::notifierOnImportantError;
bool	CPreferences::notifierOnEveryChatMsg;
bool	CPreferences::notifierOnNewVersion;
ENotifierSoundType CPreferences::notifierSoundType = ntfstNoSound;
CString	CPreferences::notifierSoundFile;
TCHAR	CPreferences::m_sircserver[50];
TCHAR	CPreferences::m_sircnick[30];
TCHAR	CPreferences::m_sircchannamefilter[50];
bool	CPreferences::m_bircaddtimestamp;
bool	CPreferences::m_bircusechanfilter;
uint16	CPreferences::m_iircchanneluserfilter;
TCHAR	CPreferences::m_sircperformstring[255];
bool	CPreferences::m_bircuseperform;
bool	CPreferences::m_birclistonconnect;
bool	CPreferences::m_bircacceptlinks;
bool	CPreferences::m_bircacceptlinksfriends;
bool	CPreferences::m_bircsoundevents;
bool	CPreferences::m_bircignoremiscmessage;
bool	CPreferences::m_bircignorejoinmessage;
bool	CPreferences::m_bircignorepartmessage;
bool	CPreferences::m_bircignorequitmessage;
bool	CPreferences::m_bircignoreemuleprotoaddfriend;
bool	CPreferences::m_bircallowemuleprotoaddfriend;
bool	CPreferences::m_bircignoreemuleprotosendlink;
bool	CPreferences::m_birchelpchannel;
bool	CPreferences::m_bRemove2bin;
bool	CPreferences::m_bShowCopyEd2kLinkCmd;
bool	CPreferences::m_bpreviewprio;
bool	CPreferences::m_bSmartServerIdCheck;
uint8	CPreferences::smartidstate;
bool	CPreferences::m_bSafeServerConnect;
bool	CPreferences::startMinimized;
bool	CPreferences::m_bAutoStart;
bool	CPreferences::m_bRestoreLastMainWndDlg;
int		CPreferences::m_iLastMainWndDlgID;
bool	CPreferences::m_bRestoreLastLogPane;
int		CPreferences::m_iLastLogPaneID;
uint16	CPreferences::MaxConperFive;
bool	CPreferences::checkDiskspace;
UINT	CPreferences::m_uMinFreeDiskSpace;
bool	CPreferences::m_bSparsePartFiles;
CString	CPreferences::m_strYourHostname;
bool	CPreferences::m_bEnableVerboseOptions;
bool	CPreferences::m_bVerbose;
bool	CPreferences::m_bFullVerbose;
bool	CPreferences::m_bDebugSourceExchange;
bool	CPreferences::m_bLogBannedClients;
bool	CPreferences::m_bLogRatingDescReceived;
bool	CPreferences::m_bLogSecureIdent;
bool	CPreferences::m_bLogFilteredIPs;
bool	CPreferences::m_bLogFileSaving;
bool	CPreferences::m_bLogA4AF; // ZZ:DownloadManager
bool	CPreferences::m_bLogUlDlEvents;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
bool	CPreferences::m_bUseDebugDevice = true;
#else
bool	CPreferences::m_bUseDebugDevice = false;
#endif
int		CPreferences::m_iDebugServerTCPLevel;
int		CPreferences::m_iDebugServerUDPLevel;
int		CPreferences::m_iDebugServerSourcesLevel;
int		CPreferences::m_iDebugServerSearchesLevel;
int		CPreferences::m_iDebugClientTCPLevel;
int		CPreferences::m_iDebugClientUDPLevel;
int		CPreferences::m_iDebugClientKadUDPLevel;
int		CPreferences::m_iDebugSearchResultDetailLevel;
bool	CPreferences::m_bupdatequeuelist;
bool	CPreferences::m_bManualAddedServersHighPriority;
bool	CPreferences::m_btransferfullchunks;
int		CPreferences::m_istartnextfile;
bool	CPreferences::m_bshowoverhead;
bool	CPreferences::m_bDAP;
bool	CPreferences::m_bUAP;
bool	CPreferences::m_bDisableKnownClientList;
bool	CPreferences::m_bDisableQueueList;
bool	CPreferences::m_bExtControls;
bool	CPreferences::m_bTransflstRemain;
uint8	CPreferences::versioncheckdays;
/*
int		CPreferences::tableSortItemDownload;
int		CPreferences::tableSortItemUpload;
int		CPreferences::tableSortItemQueue;
int		CPreferences::tableSortItemSearch;
int		CPreferences::tableSortItemShared;
int		CPreferences::tableSortItemServer;
int		CPreferences::tableSortItemClientList;
int		CPreferences::tableSortItemFilenames;
int		CPreferences::tableSortItemIrcMain;
int		CPreferences::tableSortItemIrcChannels;
int		CPreferences::tableSortItemDownloadClients;
bool	CPreferences::tableSortAscendingDownload;
bool	CPreferences::tableSortAscendingUpload;
bool	CPreferences::tableSortAscendingQueue;
bool	CPreferences::tableSortAscendingSearch;
bool	CPreferences::tableSortAscendingShared;
bool	CPreferences::tableSortAscendingServer;
bool	CPreferences::tableSortAscendingClientList;
bool	CPreferences::tableSortAscendingFilenames;
bool	CPreferences::tableSortAscendingIrcMain;
bool	CPreferences::tableSortAscendingIrcChannels;
bool	CPreferences::tableSortAscendingDownloadClients;
*/
bool	CPreferences::showRatesInTitle;
TCHAR	CPreferences::TxtEditor[256];
TCHAR	CPreferences::VideoPlayer[256];
bool	CPreferences::moviePreviewBackup;
int		CPreferences::m_iPreviewSmallBlocks;
bool	CPreferences::m_bPreviewCopiedArchives;
int		CPreferences::m_iInspectAllFileTypes;
bool	CPreferences::m_bPreviewOnIconDblClk;
bool	CPreferences::indicateratings;
bool	CPreferences::watchclipboard;
bool	CPreferences::filterserverbyip;
bool	CPreferences::m_bFirstStart;
bool	CPreferences::m_bCreditSystem;
bool	CPreferences::log2disk;
bool	CPreferences::debug2disk;
int		CPreferences::iMaxLogBuff;
UINT	CPreferences::uMaxLogFileSize;
ELogFileFormat CPreferences::m_iLogFileFormat = Unicode;
bool	CPreferences::scheduler;
bool	CPreferences::dontcompressavi;
bool	CPreferences::msgonlyfriends;
bool	CPreferences::msgsecure;
uint8	CPreferences::filterlevel;
UINT	CPreferences::m_iFileBufferSize;
UINT	CPreferences::m_iQueueSize;
int		CPreferences::m_iCommitFiles;
uint16	CPreferences::maxmsgsessions;
uint32	CPreferences::versioncheckLastAutomatic;
TCHAR	CPreferences::messageFilter[512];
CString	CPreferences::commentFilter;
TCHAR	CPreferences::filenameCleanups[512];
TCHAR	CPreferences::datetimeformat[64];
TCHAR	CPreferences::datetimeformat4log[64];
LOGFONT CPreferences::m_lfHyperText;
LOGFONT CPreferences::m_lfLogText;
COLORREF CPreferences::m_crLogError = RGB(255, 0, 0);
COLORREF CPreferences::m_crLogWarning = RGB(128, 0, 128);
COLORREF CPreferences::m_crLogSuccess = RGB(0, 0, 255);
int		CPreferences::m_iExtractMetaData;
bool	CPreferences::m_bAdjustNTFSDaylightFileTime = true;
TCHAR	CPreferences::m_sWebPassword[256];
TCHAR	CPreferences::m_sWebLowPassword[256];
CUIntArray CPreferences::m_aAllowedRemoteAccessIPs;
uint16	CPreferences::m_nWebPort;
bool	CPreferences::m_bWebEnabled;
bool	CPreferences::m_bWebUseGzip;
int		CPreferences::m_nWebPageRefresh;
bool	CPreferences::m_bWebLowEnabled;
TCHAR	CPreferences::m_sWebResDir[MAX_PATH];
int		CPreferences::m_iWebTimeoutMins;
int		CPreferences::m_iWebFileUploadSizeLimitMB;
bool	CPreferences::m_bAllowAdminHiLevFunc;
TCHAR	CPreferences::m_sTemplateFile[MAX_PATH];
ProxySettings CPreferences::proxy;
bool	CPreferences::m_bIsASCWOP;
bool	CPreferences::m_bShowProxyErrors;
bool	CPreferences::showCatTabInfos;
bool	CPreferences::resumeSameCat;
bool	CPreferences::dontRecreateGraphs;
bool	CPreferences::autofilenamecleanup;
bool	CPreferences::m_bUseAutocompl;
bool	CPreferences::m_bShowDwlPercentage;
bool	CPreferences::m_bRemoveFinishedDownloads;
uint16	CPreferences::m_iMaxChatHistory;
bool	CPreferences::m_bShowActiveDownloadsBold;
int		CPreferences::m_iSearchMethod;
bool	CPreferences::m_bAdvancedSpamfilter;
bool	CPreferences::m_bUseSecureIdent;
TCHAR	CPreferences::m_sMMPassword[256];
bool	CPreferences::m_bMMEnabled;
uint16	CPreferences::m_nMMPort;
bool	CPreferences::networkkademlia;
bool	CPreferences::networked2k;
EToolbarLabelType CPreferences::m_nToolbarLabels;
CString	CPreferences::m_sToolbarBitmap;
CString	CPreferences::m_sToolbarBitmapFolder;
CString	CPreferences::m_sToolbarSettings;
bool	CPreferences::m_bReBarToolbar;
CSize	CPreferences::m_sizToolbarIconSize;
bool	CPreferences::m_bPreviewEnabled;
bool	CPreferences::m_bDynUpEnabled;
int		CPreferences::m_iDynUpPingTolerance;
int		CPreferences::m_iDynUpGoingUpDivider;
int		CPreferences::m_iDynUpGoingDownDivider;
int		CPreferences::m_iDynUpNumberOfPings;
int		CPreferences::m_iDynUpPingToleranceMilliseconds;
bool	CPreferences::m_bDynUpUseMillisecondPingTolerance;

// ZZ:DownloadManager -->
bool    CPreferences::m_bA4AFSaveCpu;
// ZZ:DownloadManager <--

CStringList CPreferences::shareddir_list;
CStringList CPreferences::addresses_list;
CString CPreferences::appdir;
CString CPreferences::configdir;
CString CPreferences::m_strWebServerDir;
CString CPreferences::m_strLangDir;
CString CPreferences::m_strFileCommentsFilePath;
CString	CPreferences::m_strLogDir;
Preferences_Ext_Struct* CPreferences::prefsExt;
WORD	CPreferences::m_wWinVer;
bool	CPreferences::m_UseProxyListenPort;
uint16	CPreferences::ListenPort;
CArray<Category_Struct*,Category_Struct*> CPreferences::catMap;
uint8	CPreferences::m_nWebMirrorAlertLevel;
bool	CPreferences::m_bRunAsUser;
bool	CPreferences::m_bPreferRestrictedOverUser;
bool	CPreferences::m_bUseOldTimeRemaining;
uint32	CPreferences::m_uPeerCacheLastSearch;
bool	CPreferences::m_bPeerCacheWasFound;
bool	CPreferences::m_bPeerCacheEnabled;
uint16	CPreferences::m_nPeerCachePort;
bool	CPreferences::m_bPeerCacheShow;

bool	CPreferences::m_bOpenPortsOnStartUp;
uint8	CPreferences::m_byLogLevel;
bool	CPreferences::m_bTrustEveryHash;
bool	CPreferences::m_bRememberCancelledFiles;
bool	CPreferences::m_bRememberDownloadedFiles;

bool	CPreferences::m_bNotifierSendMail;
CString	CPreferences::m_strNotifierMailServer;
CString	CPreferences::m_strNotifierMailSender;
CString	CPreferences::m_strNotifierMailReceiver;

bool	CPreferences::m_bWinaTransToolbar;

CPreferences::CPreferences()
{
#ifdef _DEBUG
	m_iDbgHeap = 1;
#endif
}

CPreferences::~CPreferences()
{
	delete prefsExt;
}

LPCTSTR CPreferences::GetConfigFile()
{
	return theApp.m_pszProfileName;
}

void CPreferences::Init()
{
	srand((uint32)time(0)); // we need random numbers sometimes

	prefsExt = new Preferences_Ext_Struct;
	memset(prefsExt, 0, sizeof *prefsExt);

	//get application start directory
	TCHAR buffer[490];
	::GetModuleFileName(0, buffer, 490);
	LPTSTR pszFileName = _tcsrchr(buffer, _T('\\')) + 1;
	*pszFileName = _T('\0');

	appdir = buffer;
	configdir = appdir + CONFIGFOLDER;
	m_strWebServerDir = appdir + _T("webserver\\");
	m_strLangDir = appdir + _T("lang\\");
	m_strFileCommentsFilePath = configdir + _T("fileinfo.ini");
	m_strLogDir = appdir + _T("logs\\");

	///////////////////////////////////////////////////////////////////////////
	// Create 'config' directory (and optionally move files from application directory)
	//
	::CreateDirectory(GetConfigDir(), 0);

	///////////////////////////////////////////////////////////////////////////
	// Create 'logs' directory (and optionally move files from application directory)
	//
	::CreateDirectory(GetLogDir(), 0);
	CFileFind ff;
	BOOL bFoundFile = ff.FindFile(GetAppDir() + _T("eMule*.log"), 0);
	while (bFoundFile)
	{
		bFoundFile = ff.FindNextFile();
		if (ff.IsDots() || ff.IsSystem() || ff.IsDirectory() || ff.IsHidden())
			continue;
		MoveFile(ff.GetFilePath(), GetLogDir() + ff.GetFileName());
	}


	CreateUserHash();

	// load preferences.dat or set standart values
	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+16];
	_stprintf(fullpath,_T("%spreferences.dat"),configdir);
	FILE* preffile = _tfsopen(fullpath,_T("rb"), _SH_DENYWR);
	delete[] fullpath;

	LoadPreferences();

	if (!preffile){
		SetStandartValues();
	}
	else{
		fread(prefsExt,sizeof(Preferences_Ext_Struct),1,preffile);
		if (ferror(preffile))
			SetStandartValues();

		md4cpy(userhash, prefsExt->userhash);
		EmuleWindowPlacement = prefsExt->EmuleWindowPlacement;

		fclose(preffile);
		smartidstate = 0;
	}

	// shared directories
	fullpath = new TCHAR[_tcslen(configdir) + MAX_PATH];
	_stprintf(fullpath, _T("%sshareddir.dat"), configdir);
	CStdioFile* sdirfile = new CStdioFile();
	bool bIsUnicodeFile = IsUnicodeFile(fullpath); // check for BOM
	// open the text file either in ANSI (text) or Unicode (binary), this way we can read old and new files
	// with nearly the same code..
	if (sdirfile->Open(fullpath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
	{
		try {
			if (bIsUnicodeFile)
				sdirfile->Seek(sizeof(WORD), SEEK_CUR); // skip BOM

			CString toadd;
			while (sdirfile->ReadString(toadd))
			{
				toadd.Trim(_T("\r\n")); // need to trim '\r' in binary mode
				TCHAR szFullPath[MAX_PATH];
				if (PathCanonicalize(szFullPath, toadd))
					toadd = szFullPath;

				if (!IsShareableDirectory(toadd) )
					continue;

				if (_taccess(toadd, 0) == 0) { // only add directories which still exist
					if (toadd.Right(1) != _T('\\'))
						toadd.Append(_T("\\"));
					shareddir_list.AddHead(toadd);
				}
			}
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
		sdirfile->Close();
	}
	delete sdirfile;
	delete[] fullpath;

	// serverlist addresses
	// filename update to reasonable name
	if (PathFileExists( configdir + _T("adresses.dat")) ) {
		if (PathFileExists( configdir + _T("addresses.dat")) )
			DeleteFile( configdir + _T("adresses.dat"));
		else 
			MoveFile( configdir + _T("adresses.dat"), configdir + _T("addresses.dat"));
	}

	fullpath = new TCHAR[_tcslen(configdir) + 20];
	_stprintf(fullpath, _T("%saddresses.dat"), configdir);
	sdirfile = new CStdioFile();
	bIsUnicodeFile = IsUnicodeFile(fullpath);
	if (sdirfile->Open(fullpath, CFile::modeRead | CFile::shareDenyWrite | (bIsUnicodeFile ? CFile::typeBinary : 0)))
	{
		try {
			if (bIsUnicodeFile)
				sdirfile->Seek(sizeof(WORD), SEEK_CUR); // skip BOM

			CString toadd;
			while (sdirfile->ReadString(toadd))
			{
				toadd.Trim(_T("\r\n")); // need to trim '\r' in binary mode
				addresses_list.AddHead(toadd);
			}
		}
		catch (CFileException* ex) {
			ASSERT(0);
			ex->Delete();
		}
		sdirfile->Close();
	}
	delete sdirfile;
	delete[] fullpath;
	fullpath = NULL;

	userhash[5] = 14;
	userhash[14] = 111;

	// deadlake PROXYSUPPORT
	m_UseProxyListenPort = false;
	ListenPort = 0;

	// Explicitly inform the user about errors with incoming/temp folders!
	if (!PathFileExists(GetIncomingDir()) && !::CreateDirectory(GetIncomingDir(),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetIncomingDir(), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);
		_stprintf(incomingdir,_T("%sincoming"),appdir);
		if (!PathFileExists(GetIncomingDir()) && !::CreateDirectory(GetIncomingDir(),0)){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_INCOMING), GetIncomingDir(), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
	if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)) {
		CString strError;
		strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
		AfxMessageBox(strError, MB_ICONERROR);
		
		tempdir.SetAt(0,appdir + _T("temp") );
		if (!PathFileExists(GetTempDir()) && !::CreateDirectory(GetTempDir(),0)){
			strError.Format(GetResString(IDS_ERR_CREATE_DIR), GetResString(IDS_PW_TEMP), GetTempDir(), GetErrorMessage(GetLastError()));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}

	// Create 'skins' directory
	if (!PathFileExists(GetSkinProfileDir()) && !CreateDirectory(GetSkinProfileDir(), 0)) {
		m_strSkinProfileDir = appdir + _T("skins");
		CreateDirectory(GetSkinProfileDir(), 0);
	}

	// Create 'toolbars' directory
	if (!PathFileExists(GetToolbarBitmapFolderSettings()) && !CreateDirectory(GetToolbarBitmapFolderSettings(), 0)) {
		m_sToolbarBitmapFolder = appdir + _T("skins");
		CreateDirectory(GetToolbarBitmapFolderSettings(), 0);
	}


	if (((int*)userhash[0]) == 0 && ((int*)userhash[1]) == 0 && ((int*)userhash[2]) == 0 && ((int*)userhash[3]) == 0)
		CreateUserHash();
}

void CPreferences::Uninit()
{
	while (!catMap.IsEmpty())
	{
		Category_Struct* delcat = catMap.GetAt(0); 
		catMap.RemoveAt(0); 
		delete delcat;
	}
}

void CPreferences::SetStandartValues()
{
	CreateUserHash();

	WINDOWPLACEMENT defaultWPM;
	defaultWPM.length = sizeof(WINDOWPLACEMENT);
	defaultWPM.rcNormalPosition.left=10;defaultWPM.rcNormalPosition.top=10;
	defaultWPM.rcNormalPosition.right=700;defaultWPM.rcNormalPosition.bottom=500;
	defaultWPM.showCmd=0;
	EmuleWindowPlacement=defaultWPM;
	versioncheckLastAutomatic=0;

//	Save();
}

bool CPreferences::IsTempFile(const CString& rstrDirectory, const CString& rstrName)
{
	bool bFound = false;
	for (int i=0;i<tempdir.GetCount() && !bFound;i++)
		if (CompareDirectories(rstrDirectory, GetTempDir(i))==0)
			bFound = true; //ok, found a directory
	
	if(!bFound) //found nowhere - not a tempfile...
		return false;

	// do not share a file from the temp directory, if it matches one of the following patterns
	CString strNameLower(rstrName);
	strNameLower.MakeLower();
	strNameLower += _T("|"); // append an EOS character which we can query for
	static const LPCTSTR _apszNotSharedExts[] = {
		_T("%u.part") _T("%c"), 
		_T("%u.part.met") _T("%c"), 
		_T("%u.part.met") PARTMET_BAK_EXT _T("%c"), 
		_T("%u.part.met") PARTMET_TMP_EXT _T("%c") 
	};
	for (int i = 0; i < ARRSIZE(_apszNotSharedExts); i++){
		UINT uNum;
		TCHAR iChar;
		// "misuse" the 'scanf' function for a very simple pattern scanning.
		if (_stscanf(strNameLower, _apszNotSharedExts[i], &uNum, &iChar) == 2 && iChar == _T('|'))
			return true;
	}

	return false;
}

// SLUGFILLER: SafeHash
bool CPreferences::IsConfigFile(const CString& rstrDirectory, const CString& rstrName)
{
	if (CompareDirectories(rstrDirectory, configdir))
		return false;

	// do not share a file from the config directory, if it contains one of the following extensions
	static const LPCTSTR _apszNotSharedExts[] = { _T(".met.bak"), _T(".ini.old") };
	for (int i = 0; i < ARRSIZE(_apszNotSharedExts); i++){
		int iLen = _tcslen(_apszNotSharedExts[i]);
		if (rstrName.GetLength()>=iLen && rstrName.Right(iLen).CompareNoCase(_apszNotSharedExts[i])==0)
			return true;
	}

	// do not share following files from the config directory
	static const LPCTSTR _apszNotSharedFiles[] = 
	{
		_T("AC_SearchStrings.dat"),
		_T("AC_ServerMetURLs.dat"),
		_T("addresses.dat"),
		_T("category.ini"),
		_T("clients.met"),
		_T("cryptkey.dat"),
		_T("emfriends.met"),
		_T("fileinfo.ini"),
		_T("ipfilter.dat"),
		_T("known.met"),
		_T("preferences.dat"),
		_T("preferences.ini"),
		_T("server.met"),
		_T("server.met.new"),
		_T("server_met.download"),
		_T("server_met.old"),
		_T("shareddir.dat"),
		_T("sharedsubdir.dat"),
		_T("staticservers.dat"),
		_T("webservices.dat")
	};
	for (int i = 0; i < ARRSIZE(_apszNotSharedFiles); i++){
		if (rstrName.CompareNoCase(_apszNotSharedFiles[i])==0)
			return true;
	}

	return false;
}
// SLUGFILLER: SafeHash

uint16 CPreferences::GetMaxDownload(){
    return (uint16)(GetMaxDownloadInBytesPerSec()/1024);
}

uint64 CPreferences::GetMaxDownloadInBytesPerSec(bool dynamic){
//dont be a Lam3r :)
    uint64 maxup;
    if(dynamic && thePrefs.IsDynUpEnabled() && theApp.uploadqueue->GetWaitingUserCount() != 0 && theApp.uploadqueue->GetDatarate() != 0) {
        maxup = theApp.uploadqueue->GetDatarate();
    } else {
        maxup = GetMaxUpload()*1024;
    }

	if( maxup < 4*1024 )
		return (( (maxup < 10*1024) && (maxup*3 < maxdownload*1024) )? maxup*3 : maxdownload*1024);
	return (( (maxup < 10*1024) && (maxup*4 < maxdownload*1024) )? maxup*4 : maxdownload*1024);
}

// -khaos--+++> A whole bunch of methods!  Keep going until you reach the end tag.
void CPreferences::SaveStats(int bBackUp){
	// This function saves all of the new statistics in my addon.  It is also used to
	// save backups for the Reset Stats function, and the Restore Stats function (Which is actually LoadStats)
	// bBackUp = 0: DEFAULT; save to statistics.ini
	// bBackUp = 1: Save to statbkup.ini, which is used to restore after a reset
	// bBackUp = 2: Save to statbkuptmp.ini, which is temporarily created during a restore and then renamed to statbkup.ini

	CString fullpath(configdir);
	if (bBackUp == 1)
		fullpath += _T("statbkup.ini");
	else if (bBackUp == 2)
		fullpath += _T("statbkuptmp.ini");
	else
		fullpath += _T("statistics.ini");
	
	CIni ini(fullpath, _T("Statistics"));

	// Save cumulative statistics to preferences.ini, going in order as they appear in CStatisticsDlg::ShowStatistics.
	// We do NOT SET the values in prefs struct here.

    // Save Cum Down Data
	ini.WriteUInt64(_T("TotalDownloadedBytes"), theStats.sessionReceivedBytes + GetTotalDownloaded());
	ini.WriteInt(_T("DownSuccessfulSessions"), cumDownSuccessfulSessions);
	ini.WriteInt(_T("DownFailedSessions"), cumDownFailedSessions);
	ini.WriteInt(_T("DownAvgTime"), (GetDownC_AvgTime() + GetDownS_AvgTime()) / 2);
	ini.WriteUInt64(_T("LostFromCorruption"), cumLostFromCorruption + sesLostFromCorruption);
	ini.WriteUInt64(_T("SavedFromCompression"), sesSavedFromCompression + cumSavedFromCompression);
	ini.WriteInt(_T("PartsSavedByICH"), cumPartsSavedByICH + sesPartsSavedByICH);

	ini.WriteUInt64(_T("DownData_EDONKEY"), GetCumDownData_EDONKEY());
	ini.WriteUInt64(_T("DownData_EDONKEYHYBRID"), GetCumDownData_EDONKEYHYBRID());
	ini.WriteUInt64(_T("DownData_EMULE"), GetCumDownData_EMULE());
	ini.WriteUInt64(_T("DownData_MLDONKEY"), GetCumDownData_MLDONKEY());
	ini.WriteUInt64(_T("DownData_LMULE"), GetCumDownData_EMULECOMPAT());
	ini.WriteUInt64(_T("DownData_AMULE"), GetCumDownData_AMULE());
	ini.WriteUInt64(_T("DownData_SHAREAZA"), GetCumDownData_SHAREAZA());
	ini.WriteUInt64(_T("DownData_URL"), GetCumDownData_URL());
	ini.WriteUInt64(_T("DownDataPort_4662"), GetCumDownDataPort_4662());
	ini.WriteUInt64(_T("DownDataPort_OTHER"), GetCumDownDataPort_OTHER());
	ini.WriteUInt64(_T("DownDataPort_PeerCache"), GetCumDownDataPort_PeerCache());

	ini.WriteUInt64(_T("DownOverheadTotal"),theStats.GetDownDataOverheadFileRequest() +
										theStats.GetDownDataOverheadSourceExchange() +
										theStats.GetDownDataOverheadServer() +
										theStats.GetDownDataOverheadKad() +
										theStats.GetDownDataOverheadOther() +
										GetDownOverheadTotal());
	ini.WriteUInt64(_T("DownOverheadFileReq"), theStats.GetDownDataOverheadFileRequest() + GetDownOverheadFileReq());
	ini.WriteUInt64(_T("DownOverheadSrcEx"), theStats.GetDownDataOverheadSourceExchange() + GetDownOverheadSrcEx());
	ini.WriteUInt64(_T("DownOverheadServer"), theStats.GetDownDataOverheadServer() + GetDownOverheadServer());
	ini.WriteUInt64(_T("DownOverheadKad"), theStats.GetDownDataOverheadKad() + GetDownOverheadKad());
	
	ini.WriteUInt64(_T("DownOverheadTotalPackets"), theStats.GetDownDataOverheadFileRequestPackets() + 
												theStats.GetDownDataOverheadSourceExchangePackets() + 
												theStats.GetDownDataOverheadServerPackets() + 
												theStats.GetDownDataOverheadKadPackets() + 
												theStats.GetDownDataOverheadOtherPackets() + 
												GetDownOverheadTotalPackets());
	ini.WriteUInt64(_T("DownOverheadFileReqPackets"), theStats.GetDownDataOverheadFileRequestPackets() + GetDownOverheadFileReqPackets());
	ini.WriteUInt64(_T("DownOverheadSrcExPackets"), theStats.GetDownDataOverheadSourceExchangePackets() + GetDownOverheadSrcExPackets());
	ini.WriteUInt64(_T("DownOverheadServerPackets"), theStats.GetDownDataOverheadServerPackets() + GetDownOverheadServerPackets());
	ini.WriteUInt64(_T("DownOverheadKadPackets"), theStats.GetDownDataOverheadKadPackets() + GetDownOverheadKadPackets());

	// Save Cumulative Upline Statistics
	ini.WriteUInt64(_T("TotalUploadedBytes"), theStats.sessionSentBytes + GetTotalUploaded());
	ini.WriteInt(_T("UpSuccessfulSessions"), theApp.uploadqueue->GetSuccessfullUpCount() + GetUpSuccessfulSessions());
	ini.WriteInt(_T("UpFailedSessions"), theApp.uploadqueue->GetFailedUpCount() + GetUpFailedSessions());
	ini.WriteInt(_T("UpAvgTime"), (theApp.uploadqueue->GetAverageUpTime() + GetUpAvgTime())/2);
	ini.WriteUInt64(_T("UpData_EDONKEY"), GetCumUpData_EDONKEY());
	ini.WriteUInt64(_T("UpData_EDONKEYHYBRID"), GetCumUpData_EDONKEYHYBRID());
	ini.WriteUInt64(_T("UpData_EMULE"), GetCumUpData_EMULE());
	ini.WriteUInt64(_T("UpData_MLDONKEY"), GetCumUpData_MLDONKEY());
	ini.WriteUInt64(_T("UpData_LMULE"), GetCumUpData_EMULECOMPAT());
	ini.WriteUInt64(_T("UpData_AMULE"), GetCumUpData_AMULE());
	ini.WriteUInt64(_T("UpData_SHAREAZA"), GetCumUpData_SHAREAZA());
	ini.WriteUInt64(_T("UpDataPort_4662"), GetCumUpDataPort_4662());
	ini.WriteUInt64(_T("UpDataPort_OTHER"), GetCumUpDataPort_OTHER());
	ini.WriteUInt64(_T("UpDataPort_PeerCache"), GetCumUpDataPort_PeerCache());
	ini.WriteUInt64(_T("UpData_File"), GetCumUpData_File());
	ini.WriteUInt64(_T("UpData_Partfile"), GetCumUpData_Partfile());

	ini.WriteUInt64(_T("UpOverheadTotal"), theStats.GetUpDataOverheadFileRequest() + 
										theStats.GetUpDataOverheadSourceExchange() + 
										theStats.GetUpDataOverheadServer() + 
										theStats.GetUpDataOverheadKad() + 
										theStats.GetUpDataOverheadOther() + 
										GetUpOverheadTotal());
	ini.WriteUInt64(_T("UpOverheadFileReq"), theStats.GetUpDataOverheadFileRequest() + GetUpOverheadFileReq());
	ini.WriteUInt64(_T("UpOverheadSrcEx"), theStats.GetUpDataOverheadSourceExchange() + GetUpOverheadSrcEx());
	ini.WriteUInt64(_T("UpOverheadServer"), theStats.GetUpDataOverheadServer() + GetUpOverheadServer());
	ini.WriteUInt64(_T("UpOverheadKad"), theStats.GetUpDataOverheadKad() + GetUpOverheadKad());

	ini.WriteUInt64(_T("UpOverheadTotalPackets"), theStats.GetUpDataOverheadFileRequestPackets() + 
										theStats.GetUpDataOverheadSourceExchangePackets() + 
										theStats.GetUpDataOverheadServerPackets() + 
										theStats.GetUpDataOverheadKadPackets() + 
										theStats.GetUpDataOverheadOtherPackets() + 
										GetUpOverheadTotalPackets());
	ini.WriteUInt64(_T("UpOverheadFileReqPackets"), theStats.GetUpDataOverheadFileRequestPackets() + GetUpOverheadFileReqPackets());
	ini.WriteUInt64(_T("UpOverheadSrcExPackets"), theStats.GetUpDataOverheadSourceExchangePackets() + GetUpOverheadSrcExPackets());
	ini.WriteUInt64(_T("UpOverheadServerPackets"), theStats.GetUpDataOverheadServerPackets() + GetUpOverheadServerPackets());
	ini.WriteUInt64(_T("UpOverheadKadPackets"), theStats.GetUpDataOverheadKadPackets() + GetUpOverheadKadPackets());

	// Save Cumulative Connection Statistics
	float tempRate = 0.0F;

	// Download Rate Average
	tempRate = theStats.GetAvgDownloadRate(AVG_TOTAL);
	ini.WriteFloat(_T("ConnAvgDownRate"), tempRate);
	
	// Max Download Rate Average
	if (tempRate > GetConnMaxAvgDownRate())
		SetConnMaxAvgDownRate(tempRate);
	ini.WriteFloat(_T("ConnMaxAvgDownRate"), GetConnMaxAvgDownRate());
	
	// Max Download Rate
	tempRate = (float)theApp.downloadqueue->GetDatarate() / 1024;
	if (tempRate > GetConnMaxDownRate())
		SetConnMaxDownRate(tempRate);
	ini.WriteFloat(_T("ConnMaxDownRate"), GetConnMaxDownRate());
	
	// Upload Rate Average
	tempRate = theStats.GetAvgUploadRate(AVG_TOTAL);
	ini.WriteFloat(_T("ConnAvgUpRate"), tempRate);
	
	// Max Upload Rate Average
	if (tempRate > GetConnMaxAvgUpRate())
		SetConnMaxAvgUpRate(tempRate);
	ini.WriteFloat(_T("ConnMaxAvgUpRate"), GetConnMaxAvgUpRate());
	
	// Max Upload Rate
	tempRate = (float)theApp.uploadqueue->GetDatarate() / 1024;
	if (tempRate > GetConnMaxUpRate())
		SetConnMaxUpRate(tempRate);
	ini.WriteFloat(_T("ConnMaxUpRate"), GetConnMaxUpRate());
	
	// Overall Run Time
	ini.WriteInt(_T("ConnRunTime"), (UINT)((GetTickCount() - theStats.starttime)/1000 + GetConnRunTime()));
	
	// Number of Reconnects
	ini.WriteInt(_T("ConnNumReconnects"), (theStats.reconnects>0) ? (theStats.reconnects - 1 + GetConnNumReconnects()) : GetConnNumReconnects());
	
	// Average Connections
	if (theApp.serverconnect->IsConnected())
		ini.WriteInt(_T("ConnAvgConnections"), (UINT)((theApp.listensocket->GetAverageConnections() + cumConnAvgConnections)/2));
	
	// Peak Connections
	if (theApp.listensocket->GetPeakConnections() > cumConnPeakConnections)
		cumConnPeakConnections = theApp.listensocket->GetPeakConnections();
	ini.WriteInt(_T("ConnPeakConnections"), cumConnPeakConnections);
	
	// Max Connection Limit Reached
	if (theApp.listensocket->GetMaxConnectionReached() + cumConnMaxConnLimitReached > cumConnMaxConnLimitReached)
		ini.WriteInt(_T("ConnMaxConnLimitReached"), theApp.listensocket->GetMaxConnectionReached() + cumConnMaxConnLimitReached);
	
	// Time Stuff...
	ini.WriteInt(_T("ConnTransferTime"), GetConnTransferTime() + theStats.GetTransferTime());
	ini.WriteInt(_T("ConnUploadTime"), GetConnUploadTime() + theStats.GetUploadTime());
	ini.WriteInt(_T("ConnDownloadTime"), GetConnDownloadTime() + theStats.GetDownloadTime());
	ini.WriteInt(_T("ConnServerDuration"), GetConnServerDuration() + theStats.GetServerDuration());
	
	// Compare and Save Server Records
	uint32 servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus(servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile, servocc);
	
	if (servtotal - servfail > cumSrvrsMostWorkingServers)
		cumSrvrsMostWorkingServers = servtotal - servfail;
	ini.WriteInt(_T("SrvrsMostWorkingServers"), cumSrvrsMostWorkingServers);

	if (servtuser > cumSrvrsMostUsersOnline)
		cumSrvrsMostUsersOnline = servtuser;
	ini.WriteInt(_T("SrvrsMostUsersOnline"), cumSrvrsMostUsersOnline);

	if (servtfile > cumSrvrsMostFilesAvail)
		cumSrvrsMostFilesAvail = servtfile;
	ini.WriteInt(_T("SrvrsMostFilesAvail"), cumSrvrsMostFilesAvail);

	// Compare and Save Shared File Records
	if (theApp.sharedfiles->GetCount() > cumSharedMostFilesShared)
		cumSharedMostFilesShared = theApp.sharedfiles->GetCount();
	ini.WriteInt(_T("SharedMostFilesShared"), cumSharedMostFilesShared);

	uint64 bytesLargestFile = 0;
	uint64 allsize = theApp.sharedfiles->GetDatasize(bytesLargestFile);
	if (allsize > cumSharedLargestShareSize)
		cumSharedLargestShareSize = allsize;
	ini.WriteUInt64(_T("SharedLargestShareSize"), cumSharedLargestShareSize);
	if (bytesLargestFile > cumSharedLargestFileSize)
		cumSharedLargestFileSize = bytesLargestFile;
	ini.WriteUInt64(_T("SharedLargestFileSize"), cumSharedLargestFileSize);

	if (theApp.sharedfiles->GetCount() != 0) {
		uint64 tempint = allsize/theApp.sharedfiles->GetCount();
		if (tempint > cumSharedLargestAvgFileSize)
			cumSharedLargestAvgFileSize = tempint;
	}

	ini.WriteUInt64(_T("SharedLargestAvgFileSize"), cumSharedLargestAvgFileSize);
	ini.WriteUInt64(_T("statsDateTimeLastReset"), stat_datetimeLastReset);

	// If we are saving a back-up or a temporary back-up, return now.
	if (bBackUp != 0)
		return;
}

void CPreferences::SetRecordStructMembers() {

	// The purpose of this function is to be called from CStatisticsDlg::ShowStatistics()
	// This was easier than making a bunch of functions to interface with the record
	// members of the prefs struct from ShowStatistics.

	// This function is going to compare current values with previously saved records, and if
	// the current values are greater, the corresponding member of prefs will be updated.
	// We will not write to INI here, because this code is going to be called a lot more often
	// than SaveStats()  - Khaos

	CString buffer;

	// Servers
	uint32 servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile;
	float servocc;
	theApp.serverlist->GetStatus( servtotal, servfail, servuser, servfile, servlowiduser, servtuser, servtfile, servocc );
	if ((servtotal-servfail)>cumSrvrsMostWorkingServers) cumSrvrsMostWorkingServers = (servtotal-servfail);
	if (servtuser>cumSrvrsMostUsersOnline) cumSrvrsMostUsersOnline = servtuser;
	if (servtfile>cumSrvrsMostFilesAvail) cumSrvrsMostFilesAvail = servtfile;

	// Shared Files
	if (theApp.sharedfiles->GetCount()>cumSharedMostFilesShared) cumSharedMostFilesShared = theApp.sharedfiles->GetCount();
	uint64 bytesLargestFile = 0;
	uint64 allsize=theApp.sharedfiles->GetDatasize(bytesLargestFile);
	if (allsize>cumSharedLargestShareSize) cumSharedLargestShareSize = allsize;
	if (bytesLargestFile>cumSharedLargestFileSize) cumSharedLargestFileSize = bytesLargestFile;
	if (theApp.sharedfiles->GetCount() != 0) {
		uint64 tempint = allsize/theApp.sharedfiles->GetCount();
		if (tempint>cumSharedLargestAvgFileSize) cumSharedLargestAvgFileSize = tempint;
	}
} // SetRecordStructMembers()

void CPreferences::SaveCompletedDownloadsStat(){

	// This function saves the values for the completed
	// download members to INI.  It is called from
	// CPartfile::PerformFileComplete ...   - Khaos

	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+MAX_PATH]; // i_a
	_stprintf(fullpath,_T("%sstatistics.ini"),configdir);
	
	CIni ini( fullpath, _T("Statistics") );

	delete[] fullpath;

	ini.WriteInt(_T("DownCompletedFiles"),			GetDownCompletedFiles());
	ini.WriteInt(_T("DownSessionCompletedFiles"),	GetDownSessionCompletedFiles());
} // SaveCompletedDownloadsStat()

void CPreferences::Add2SessionTransferData(UINT uClientID, UINT uClientPort, BOOL bFromPF, 
										   BOOL bUpDown, uint32 bytes, bool sentToFriend)
{
	//	This function adds the transferred bytes to the appropriate variables,
	//	as well as to the totals for all clients. - Khaos
	//	PARAMETERS:
	//	uClientID - The identifier for which client software sent or received this data, eg SO_EMULE
	//	uClientPort - The remote port of the client that sent or received this data, eg 4662
	//	bFromPF - Applies only to uploads.  True is from partfile, False is from non-partfile.
	//	bUpDown - True is Up, False is Down
	//	bytes - Number of bytes sent by the client.  Subtract header before calling.

	switch (bUpDown){
		case true:
			//	Upline Data
			switch (uClientID){
				// Update session client breakdown stats for sent bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesUpData_EMULE+=bytes;			break;
				case SO_EDONKEYHYBRID:	sesUpData_EDONKEYHYBRID+=bytes;	break;
				case SO_EDONKEY:		sesUpData_EDONKEY+=bytes;		break;
				case SO_MLDONKEY:		sesUpData_MLDONKEY+=bytes;		break;
				case SO_AMULE:			sesUpData_AMULE+=bytes;			break;
				case SO_SHAREAZA:		sesUpData_SHAREAZA+=bytes;		break;
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesUpData_EMULECOMPAT+=bytes;	break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for sent bytes...
				case 4662:				sesUpDataPort_4662+=bytes;		break;
				case (UINT)-1:			sesUpDataPort_PeerCache+=bytes;	break;
				//case (UINT)-2:		sesUpDataPort_URL+=bytes;		break;
				default:				sesUpDataPort_OTHER+=bytes;		break;
			}

			if (bFromPF)				sesUpData_Partfile+=bytes;
			else						sesUpData_File+=bytes;

			//	Add to our total for sent bytes...
			theApp.UpdateSentBytes(bytes, sentToFriend);

			break;

		case false:
			// Downline Data
			switch (uClientID){
                // Update session client breakdown stats for received bytes...
				case SO_EMULE:
				case SO_OLDEMULE:		sesDownData_EMULE+=bytes;		break;
				case SO_EDONKEYHYBRID:	sesDownData_EDONKEYHYBRID+=bytes;break;
				case SO_EDONKEY:		sesDownData_EDONKEY+=bytes;		break;
				case SO_MLDONKEY:		sesDownData_MLDONKEY+=bytes;	break;
				case SO_AMULE:			sesDownData_AMULE+=bytes;		break;
				case SO_SHAREAZA:		sesDownData_SHAREAZA+=bytes;	break;
				case SO_CDONKEY:
				case SO_LPHANT:
				case SO_XMULE:			sesDownData_EMULECOMPAT+=bytes;	break;
				case SO_URL:			sesDownData_URL+=bytes;			break;
			}

			switch (uClientPort){
				// Update session port breakdown stats for received bytes...
				// For now we are only going to break it down by default and non-default.
				// A statistical analysis of all data sent from every single port/domain is
				// beyond the scope of this add-on.
				case 4662:				sesDownDataPort_4662+=bytes;	break;
				case (UINT)-1:			sesDownDataPort_PeerCache+=bytes;break;
				//case (UINT)-2:		sesDownDataPort_URL+=bytes;		break;
				default:				sesDownDataPort_OTHER+=bytes;	break;
			}

			//	Add to our total for received bytes...
			theApp.UpdateReceivedBytes(bytes);
	}
}

// Reset Statistics by Khaos

void CPreferences::ResetCumulativeStatistics(){

	// Save a backup so that we can undo this action
	SaveStats(1);

	// SET ALL CUMULATIVE STAT VALUES TO 0  :'-(

	totalDownloadedBytes=0;
	totalUploadedBytes=0;
	cumDownOverheadTotal=0;
	cumDownOverheadFileReq=0;
	cumDownOverheadSrcEx=0;
	cumDownOverheadServer=0;
	cumDownOverheadKad=0;
	cumDownOverheadTotalPackets=0;
	cumDownOverheadFileReqPackets=0;
	cumDownOverheadSrcExPackets=0;
	cumDownOverheadServerPackets=0;
	cumDownOverheadKadPackets=0;
	cumUpOverheadTotal=0;
	cumUpOverheadFileReq=0;
	cumUpOverheadSrcEx=0;
	cumUpOverheadServer=0;
	cumUpOverheadKad=0;
	cumUpOverheadTotalPackets=0;
	cumUpOverheadFileReqPackets=0;
	cumUpOverheadSrcExPackets=0;
	cumUpOverheadServerPackets=0;
	cumUpOverheadKadPackets=0;
	cumUpSuccessfulSessions=0;
	cumUpFailedSessions=0;
	cumUpAvgTime=0;
	cumUpData_EDONKEY=0;
	cumUpData_EDONKEYHYBRID=0;
	cumUpData_EMULE=0;
	cumUpData_MLDONKEY=0;
	cumUpData_AMULE=0;
	cumUpData_EMULECOMPAT=0;
	cumUpData_SHAREAZA=0;
	cumUpDataPort_4662=0;
	cumUpDataPort_OTHER=0;
	cumUpDataPort_PeerCache=0;
	cumDownCompletedFiles=0;
	cumDownSuccessfulSessions=0;
	cumDownFailedSessions=0;
	cumDownAvgTime=0;
	cumLostFromCorruption=0;
	cumSavedFromCompression=0;
	cumPartsSavedByICH=0;
	cumDownData_EDONKEY=0;
	cumDownData_EDONKEYHYBRID=0;
	cumDownData_EMULE=0;
	cumDownData_MLDONKEY=0;
	cumDownData_AMULE=0;
	cumDownData_EMULECOMPAT=0;
	cumDownData_SHAREAZA=0;
	cumDownData_URL=0;
	cumDownDataPort_4662=0;
	cumDownDataPort_OTHER=0;
	cumDownDataPort_PeerCache=0;
	cumConnAvgDownRate=0;
	cumConnMaxAvgDownRate=0;
	cumConnMaxDownRate=0;
	cumConnAvgUpRate=0;
	cumConnRunTime=0;
	cumConnNumReconnects=0;
	cumConnAvgConnections=0;
	cumConnMaxConnLimitReached=0;
	cumConnPeakConnections=0;
	cumConnDownloadTime=0;
	cumConnUploadTime=0;
	cumConnTransferTime=0;
	cumConnServerDuration=0;
	cumConnMaxAvgUpRate=0;
	cumConnMaxUpRate=0;
	cumSrvrsMostWorkingServers=0;
	cumSrvrsMostUsersOnline=0;
	cumSrvrsMostFilesAvail=0;
    cumSharedMostFilesShared=0;
	cumSharedLargestShareSize=0;
	cumSharedLargestAvgFileSize=0;

	// Set the time of last reset...
	time_t timeNow;
	time(&timeNow);
	stat_datetimeLastReset = (__int64)timeNow;

	// Save the reset stats
	SaveStats();
	theApp.emuledlg->statisticswnd->ShowStatistics(true);
}


// Load Statistics
// This used to be integrated in LoadPreferences, but it has been altered
// so that it can be used to load the backup created when the stats are reset.
// Last Modified: 2-22-03 by Khaos
bool CPreferences::LoadStats(int loadBackUp)
{
	// loadBackUp is 0 by default
	// loadBackUp = 0: Load the stats normally like we used to do in LoadPreferences
	// loadBackUp = 1: Load the stats from statbkup.ini and create a backup of the current stats.  Also, do not initialize session variables.
	CString sINI;
	CFileFind findBackUp;

	switch (loadBackUp) {
		case 0:{
			// for transition...
			if(PathFileExists(configdir+_T("statistics.ini")))
				sINI.Format(_T("%sstatistics.ini"), configdir);
			else
				sINI.Format(_T("%spreferences.ini"), configdir);

			break;
			   }
		case 1:
			sINI.Format(_T("%sstatbkup.ini"), configdir);
			if (!findBackUp.FindFile(sINI))
				return false;
			SaveStats(2); // Save our temp backup of current values to statbkuptmp.ini, we will be renaming it at the end of this function.
			break;
	}

	BOOL fileex = PathFileExists(sINI);
	CIni ini(sINI, _T("Statistics"));

	totalDownloadedBytes			= ini.GetUInt64(_T("TotalDownloadedBytes"));
	totalUploadedBytes				= ini.GetUInt64(_T("TotalUploadedBytes"));

	// Load stats for cumulative downline overhead
	cumDownOverheadTotal			= ini.GetUInt64(_T("DownOverheadTotal"));
	cumDownOverheadFileReq			= ini.GetUInt64(_T("DownOverheadFileReq"));
	cumDownOverheadSrcEx			= ini.GetUInt64(_T("DownOverheadSrcEx"));
	cumDownOverheadServer			= ini.GetUInt64(_T("DownOverheadServer"));
	cumDownOverheadKad				= ini.GetUInt64(_T("DownOverheadKad"));
	cumDownOverheadTotalPackets		= ini.GetUInt64(_T("DownOverheadTotalPackets"));
	cumDownOverheadFileReqPackets	= ini.GetUInt64(_T("DownOverheadFileReqPackets"));
	cumDownOverheadSrcExPackets		= ini.GetUInt64(_T("DownOverheadSrcExPackets"));
	cumDownOverheadServerPackets	= ini.GetUInt64(_T("DownOverheadServerPackets"));
	cumDownOverheadKadPackets		= ini.GetUInt64(_T("DownOverheadKadPackets"));

	// Load stats for cumulative upline overhead
	cumUpOverheadTotal				= ini.GetUInt64(_T("UpOverHeadTotal"));
	cumUpOverheadFileReq			= ini.GetUInt64(_T("UpOverheadFileReq"));
	cumUpOverheadSrcEx				= ini.GetUInt64(_T("UpOverheadSrcEx"));
	cumUpOverheadServer				= ini.GetUInt64(_T("UpOverheadServer"));
	cumUpOverheadKad				= ini.GetUInt64(_T("UpOverheadKad"));
	cumUpOverheadTotalPackets		= ini.GetUInt64(_T("UpOverHeadTotalPackets"));
	cumUpOverheadFileReqPackets		= ini.GetUInt64(_T("UpOverheadFileReqPackets"));
	cumUpOverheadSrcExPackets		= ini.GetUInt64(_T("UpOverheadSrcExPackets"));
	cumUpOverheadServerPackets		= ini.GetUInt64(_T("UpOverheadServerPackets"));
	cumUpOverheadKadPackets			= ini.GetUInt64(_T("UpOverheadKadPackets"));

	// Load stats for cumulative upline data
	cumUpSuccessfulSessions			= ini.GetInt(_T("UpSuccessfulSessions"));
	cumUpFailedSessions				= ini.GetInt(_T("UpFailedSessions"));
	cumUpAvgTime					= ini.GetInt(_T("UpAvgTime"));

	// Load cumulative client breakdown stats for sent bytes
	cumUpData_EDONKEY				= ini.GetUInt64(_T("UpData_EDONKEY"));
	cumUpData_EDONKEYHYBRID			= ini.GetUInt64(_T("UpData_EDONKEYHYBRID"));
	cumUpData_EMULE					= ini.GetUInt64(_T("UpData_EMULE"));
	cumUpData_MLDONKEY				= ini.GetUInt64(_T("UpData_MLDONKEY"));
	cumUpData_EMULECOMPAT			= ini.GetUInt64(_T("UpData_LMULE"));
	cumUpData_AMULE					= ini.GetUInt64(_T("UpData_AMULE"));
	cumUpData_SHAREAZA				= ini.GetUInt64(_T("UpData_SHAREAZA"));

	// Load cumulative port breakdown stats for sent bytes
	cumUpDataPort_4662				= ini.GetUInt64(_T("UpDataPort_4662"));
	cumUpDataPort_OTHER				= ini.GetUInt64(_T("UpDataPort_OTHER"));
	cumUpDataPort_PeerCache			= ini.GetUInt64(_T("UpDataPort_PeerCache"));

	// Load cumulative source breakdown stats for sent bytes
	cumUpData_File					= ini.GetUInt64(_T("UpData_File"));
	cumUpData_Partfile				= ini.GetUInt64(_T("UpData_Partfile"));

	// Load stats for cumulative downline data
	cumDownCompletedFiles			= ini.GetInt(_T("DownCompletedFiles"));
	cumDownSuccessfulSessions		= ini.GetInt(_T("DownSuccessfulSessions"));
	cumDownFailedSessions			= ini.GetInt(_T("DownFailedSessions"));
	cumDownAvgTime					= ini.GetInt(_T("DownAvgTime"));

	// Cumulative statistics for saved due to compression/lost due to corruption
	cumLostFromCorruption			= ini.GetUInt64(_T("LostFromCorruption"));
	cumSavedFromCompression			= ini.GetUInt64(_T("SavedFromCompression"));
	cumPartsSavedByICH				= ini.GetInt(_T("PartsSavedByICH"));

	// Load cumulative client breakdown stats for received bytes
	cumDownData_EDONKEY				= ini.GetUInt64(_T("DownData_EDONKEY"));
	cumDownData_EDONKEYHYBRID		= ini.GetUInt64(_T("DownData_EDONKEYHYBRID"));
	cumDownData_EMULE				= ini.GetUInt64(_T("DownData_EMULE"));
	cumDownData_MLDONKEY			= ini.GetUInt64(_T("DownData_MLDONKEY"));
	cumDownData_EMULECOMPAT			= ini.GetUInt64(_T("DownData_LMULE"));
	cumDownData_AMULE				= ini.GetUInt64(_T("DownData_AMULE"));
	cumDownData_SHAREAZA			= ini.GetUInt64(_T("DownData_SHAREAZA"));
	cumDownData_URL					= ini.GetUInt64(_T("DownData_URL"));

	// Load cumulative port breakdown stats for received bytes
	cumDownDataPort_4662			= ini.GetUInt64(_T("DownDataPort_4662"));
	cumDownDataPort_OTHER			= ini.GetUInt64(_T("DownDataPort_OTHER"));
	cumDownDataPort_PeerCache		= ini.GetUInt64(_T("DownDataPort_PeerCache"));

	// Load stats for cumulative connection data
	cumConnAvgDownRate				= ini.GetFloat(_T("ConnAvgDownRate"));
	cumConnMaxAvgDownRate			= ini.GetFloat(_T("ConnMaxAvgDownRate"));
	cumConnMaxDownRate				= ini.GetFloat(_T("ConnMaxDownRate"));
	cumConnAvgUpRate				= ini.GetFloat(_T("ConnAvgUpRate"));
	cumConnMaxAvgUpRate				= ini.GetFloat(_T("ConnMaxAvgUpRate"));
	cumConnMaxUpRate				= ini.GetFloat(_T("ConnMaxUpRate"));
	cumConnRunTime					= ini.GetUInt64(_T("ConnRunTime"));
	cumConnTransferTime				= ini.GetInt(_T("ConnTransferTime"));
	cumConnDownloadTime				= ini.GetInt(_T("ConnDownloadTime"));
	cumConnUploadTime				= ini.GetInt(_T("ConnUploadTime"));
	cumConnServerDuration			= ini.GetInt(_T("ConnServerDuration"));
	cumConnNumReconnects			= ini.GetInt(_T("ConnNumReconnects"));
	cumConnAvgConnections			= ini.GetInt(_T("ConnAvgConnections"));
	cumConnMaxConnLimitReached		= ini.GetInt(_T("ConnMaxConnLimitReached"));
	cumConnPeakConnections			= ini.GetInt(_T("ConnPeakConnections"));

	// Load date/time of last reset
	stat_datetimeLastReset			= ini.GetUInt64(_T("statsDateTimeLastReset"));

	// Smart Load For Restores - Don't overwrite records that are greater than the backed up ones
	if (loadBackUp == 1)
	{
		// Load records for servers / network
		if ((UINT)ini.GetInt(_T("SrvrsMostWorkingServers")) > cumSrvrsMostWorkingServers)
			cumSrvrsMostWorkingServers = ini.GetInt(_T("SrvrsMostWorkingServers"));

		if ((UINT)ini.GetInt(_T("SrvrsMostUsersOnline")) > cumSrvrsMostUsersOnline)
			cumSrvrsMostUsersOnline = ini.GetInt(_T("SrvrsMostUsersOnline"));

		if ((UINT)ini.GetInt(_T("SrvrsMostFilesAvail")) > cumSrvrsMostFilesAvail)
			cumSrvrsMostFilesAvail = ini.GetInt(_T("SrvrsMostFilesAvail"));

		// Load records for shared files
		if ((UINT)ini.GetInt(_T("SharedMostFilesShared")) > cumSharedMostFilesShared)
			cumSharedMostFilesShared =	ini.GetInt(_T("SharedMostFilesShared"));

		uint64 temp64 = ini.GetUInt64(_T("SharedLargestShareSize"));
		if (temp64 > cumSharedLargestShareSize)
			cumSharedLargestShareSize = temp64;

		temp64 = ini.GetUInt64(_T("SharedLargestAvgFileSize"));
		if (temp64 > cumSharedLargestAvgFileSize)
			cumSharedLargestAvgFileSize = temp64;

		temp64 = ini.GetUInt64(_T("SharedLargestFileSize"));
		if (temp64 > cumSharedLargestFileSize)
			cumSharedLargestFileSize = temp64;

		// Check to make sure the backup of the values we just overwrote exists.  If so, rename it to the backup file.
		// This allows us to undo a restore, so to speak, just in case we don't like the restored values...
		CString sINIBackUp;
		sINIBackUp.Format(_T("%sstatbkuptmp.ini"), configdir);
		if (findBackUp.FindFile(sINIBackUp)){
			CFile::Remove(sINI);				// Remove the backup that we just restored from
			CFile::Rename(sINIBackUp, sINI);	// Rename our temporary backup to the normal statbkup.ini filename.
		}

		// Since we know this is a restore, now we should call ShowStatistics to update the data items to the new ones we just loaded.
		// Otherwise user is left waiting around for the tick counter to reach the next automatic update (Depending on setting in prefs)
		theApp.emuledlg->statisticswnd->ShowStatistics();
	}
	// Stupid Load -> Just load the values.
	else
	{
		// Load records for servers / network
		cumSrvrsMostWorkingServers	= ini.GetInt(_T("SrvrsMostWorkingServers"));
		cumSrvrsMostUsersOnline		= ini.GetInt(_T("SrvrsMostUsersOnline"));
		cumSrvrsMostFilesAvail		= ini.GetInt(_T("SrvrsMostFilesAvail"));

		// Load records for shared files
		cumSharedMostFilesShared	= ini.GetInt(_T("SharedMostFilesShared"));
		cumSharedLargestShareSize	= ini.GetUInt64(_T("SharedLargestShareSize"));
		cumSharedLargestAvgFileSize = ini.GetUInt64(_T("SharedLargestAvgFileSize"));
		cumSharedLargestFileSize	= ini.GetUInt64(_T("SharedLargestFileSize"));

		// Initialize new session statistic variables...
		sesDownCompletedFiles		= 0;
		
		sesUpData_EDONKEY			= 0;
		sesUpData_EDONKEYHYBRID		= 0;
		sesUpData_EMULE				= 0;
		sesUpData_MLDONKEY			= 0;
		sesUpData_AMULE				= 0;
		sesUpData_EMULECOMPAT		= 0;
		sesUpData_SHAREAZA			= 0;
		sesUpDataPort_4662			= 0;
		sesUpDataPort_OTHER			= 0;
		sesUpDataPort_PeerCache		= 0;

		sesDownData_EDONKEY			= 0;
		sesDownData_EDONKEYHYBRID	= 0;
		sesDownData_EMULE			= 0;
		sesDownData_MLDONKEY		= 0;
		sesDownData_AMULE			= 0;
		sesDownData_EMULECOMPAT		= 0;
		sesDownData_SHAREAZA		= 0;
		sesDownData_URL				= 0;
		sesDownDataPort_4662		= 0;
		sesDownDataPort_OTHER		= 0;
		sesDownDataPort_PeerCache	= 0;

		sesDownSuccessfulSessions	= 0;
		sesDownFailedSessions		= 0;
		sesPartsSavedByICH			= 0;
	}

	if (!fileex)
	{
		time_t timeNow;
		time(&timeNow);
		stat_datetimeLastReset = (__int64)timeNow;
	}

	return true;
}

// This formats the UTC long value that is saved for stat_datetimeLastReset
// If this value is 0 (Never reset), then it returns Unknown.
CString CPreferences::GetStatsLastResetStr(bool formatLong)
{
	// formatLong dictates the format of the string returned.
	// For example...
	// true: DateTime format from the .ini
	// false: DateTime format from the .ini for the log
	CString	returnStr;
	if (GetStatsLastResetLng()) {
		tm *statsReset;
		TCHAR szDateReset[128];
		time_t lastResetDateTime = (time_t) GetStatsLastResetLng();
		statsReset = localtime(&lastResetDateTime);
		if (statsReset){
			_tcsftime(szDateReset, ARRSIZE(szDateReset), formatLong ? GetDateTimeFormat() : GetDateTimeFormat4Log(), statsReset);
			returnStr = szDateReset;
		}
	}
	if (returnStr.IsEmpty())
		returnStr = GetResString(IDS_UNKNOWN);
	return returnStr;
}

// <-----khaos-

bool CPreferences::Save(){

	bool error = false;
	TCHAR* fullpath = new TCHAR[_tcslen(configdir)+MAX_PATH]; // i_a
	_stprintf(fullpath,_T("%spreferences.dat"),configdir);

	FILE* preffile = _tfsopen(fullpath,_T("wb"), _SH_DENYWR);
	delete[] fullpath;
	prefsExt->version = PREFFILE_VERSION;
	if (preffile){
		prefsExt->version=PREFFILE_VERSION;
		prefsExt->EmuleWindowPlacement=EmuleWindowPlacement;
		md4cpy(prefsExt->userhash, userhash);

		error = fwrite(prefsExt,sizeof(Preferences_Ext_Struct),1,preffile)!=1;
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			fflush(preffile); // flush file stream buffers to disk buffers
			(void)_commit(_fileno(preffile)); // commit disk buffers to disk
		}
		fclose(preffile);
	}
	else
		error = true;

	SavePreferences();
	SaveStats();

	fullpath = new TCHAR[_tcslen(configdir) + 14];
	_stprintf(fullpath, _T("%sshareddir.dat"), configdir);
	CStdioFile sdirfile;
	if (sdirfile.Open(fullpath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		try{
			// write Unicode byte-order mark 0xFEFF
			WORD wBOM = 0xFEFF;
			sdirfile.Write(&wBOM, sizeof(wBOM));

			for (POSITION pos = shareddir_list.GetHeadPosition();pos != 0;){
				sdirfile.WriteString(shareddir_list.GetNext(pos).GetBuffer());
				sdirfile.Write(_T("\r\n"), sizeof(TCHAR)*2);
			}
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
				sdirfile.Flush(); // flush file stream buffers to disk buffers
				if (_commit(_fileno(sdirfile.m_pStream)) != 0) // commit disk buffers to disk
					AfxThrowFileException(CFileException::hardIO, GetLastError(), sdirfile.GetFileName());
			}
			sdirfile.Close();
		}
		catch(CFileException* error){
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			if (thePrefs.GetVerbose())
				AddDebugLogLine(true,_T("Failed to save %s - %s"), fullpath, buffer);
			error->Delete();
		}
	}
	else
		error = true;
	delete[] fullpath;
	fullpath=NULL;

	::CreateDirectory(GetIncomingDir(),0);
	::CreateDirectory(GetTempDir(),0);
	return error;
}

void CPreferences::CreateUserHash()
{
	for (int i = 0; i < 8; i++)
	{
		uint16 random = GetRandomUInt16();
		memcpy(&userhash[i*2], &random, 2);
	}

	// mark as emule client. that will be need in later version
	userhash[5] = 14;
	userhash[14] = 111;
}

int CPreferences::GetRecommendedMaxConnections() {
	int iRealMax = ::GetMaxWindowsTCPConnections();
	if(iRealMax == -1 || iRealMax > 520)
		return 500;

	if(iRealMax < 20)
		return iRealMax;

	if(iRealMax <= 256)
		return iRealMax - 10;

	return iRealMax - 20;
}

void CPreferences::SavePreferences()
{
	USES_CONVERSION;
	CString buffer;
	
	CIni ini(GetConfigFile(), _T("eMule"));
	//---
	ini.WriteString(_T("AppVersion"), theApp.m_strCurVersionLong);
	//---

#ifdef _DEBUG
	ini.WriteInt(_T("DebugHeap"), m_iDbgHeap);
#endif

	ini.WriteStringUTF8(_T("Nick"), strNick);
	ini.WriteString(_T("IncomingDir"), incomingdir);
	
	ini.WriteString(_T("TempDir"), tempdir.GetAt(0));

	CString tempdirs;
	for (int i=1;i<tempdir.GetCount();i++) {
		tempdirs.Append(tempdir.GetAt(i) );
		if (i+1<tempdir.GetCount())
			tempdirs.Append(_T("|"));
	}
	ini.WriteString(_T("TempDirs"), tempdirs);

    ini.WriteInt(_T("MinUpload"), minupload);
	ini.WriteInt(_T("MaxUpload"),maxupload);
	ini.WriteInt(_T("MaxDownload"),maxdownload);
	ini.WriteInt(_T("MaxConnections"),maxconnections);
	ini.WriteInt(_T("MaxHalfConnections"),maxhalfconnections);
	ini.WriteBool(_T("ConditionalTCPAccept"), m_bConditionalTCPAccept);
	ini.WriteInt(_T("Port"),port);
	ini.WriteInt(_T("UDPPort"),udpport);
	ini.WriteInt(_T("ServerUDPPort"), nServerUDPPort);
	ini.WriteInt(_T("MaxSourcesPerFile"),maxsourceperfile );
	ini.WriteWORD(_T("Language"),m_wLanguageID);
	ini.WriteInt(_T("SeeShare"),m_iSeeShares);
	ini.WriteInt(_T("ToolTipDelay"),m_iToolDelayTime);
	ini.WriteInt(_T("StatGraphsInterval"),trafficOMeterInterval);
	ini.WriteInt(_T("StatsInterval"),statsInterval);
	ini.WriteInt(_T("DownloadCapacity"),maxGraphDownloadRate);
	ini.WriteInt(_T("UploadCapacityNew"),maxGraphUploadRate);
	ini.WriteInt(_T("DeadServerRetry"),m_uDeadServerRetries);
	ini.WriteInt(_T("ServerKeepAliveTimeout"),m_dwServerKeepAliveTimeout);
	ini.WriteInt(_T("SplitterbarPosition"),splitterbarPosition+2);
	ini.WriteInt(_T("SplitterbarPositionServer"),splitterbarPositionSvr);
	ini.WriteInt(_T("SplitterbarPositionStat"),splitterbarPositionStat+1);
	ini.WriteInt(_T("SplitterbarPositionStat_HL"),splitterbarPositionStat_HL+1);
	ini.WriteInt(_T("SplitterbarPositionStat_HR"),splitterbarPositionStat_HR+1);
	ini.WriteInt(_T("SplitterbarPositionFriend"),splitterbarPositionFriend);
	ini.WriteInt(_T("SplitterbarPositionIRC"),splitterbarPositionIRC+2);
	ini.WriteInt(_T("SplitterbarPositionShared"),splitterbarPositionShared);
	ini.WriteInt(_T("TransferWnd1"),m_uTransferWnd1);
	ini.WriteInt(_T("TransferWnd2"),m_uTransferWnd2);
	ini.WriteInt(_T("VariousStatisticsMaxValue"),statsMax);
	ini.WriteInt(_T("StatsAverageMinutes"),statsAverageMinutes);
	ini.WriteInt(_T("MaxConnectionsPerFiveSeconds"),MaxConperFive);
	ini.WriteInt(_T("Check4NewVersionDelay"),versioncheckdays);

	ini.WriteBool(_T("Reconnect"),reconnect);
	ini.WriteBool(_T("Scoresystem"),m_bUseServerPriorities);
	ini.WriteBool(_T("Serverlist"),m_bAutoUpdateServerList);
	ini.WriteBool(_T("UpdateNotifyTestClient"),updatenotify);
	ini.WriteBool(_T("MinToTray"),mintotray);
	ini.WriteBool(_T("AddServersFromServer"),m_bAddServersFromServer);
	ini.WriteBool(_T("AddServersFromClient"),m_bAddServersFromClients);
	ini.WriteBool(_T("Splashscreen"),splashscreen);
	ini.WriteBool(_T("BringToFront"),bringtoforeground);
	ini.WriteBool(_T("TransferDoubleClick"),transferDoubleclick);
	ini.WriteBool(_T("BeepOnError"),beepOnError);
	ini.WriteBool(_T("ConfirmExit"),confirmExit);
	ini.WriteBool(_T("FilterBadIPs"),filterLANIPs);
    ini.WriteBool(_T("Autoconnect"),autoconnect);
	ini.WriteBool(_T("OnlineSignature"),onlineSig);
	ini.WriteBool(_T("StartupMinimized"),startMinimized);
	ini.WriteBool(_T("AutoStart"),m_bAutoStart);
	ini.WriteInt(_T("LastMainWndDlgID"),m_iLastMainWndDlgID);
	ini.WriteInt(_T("LastLogPaneID"),m_iLastLogPaneID);
	ini.WriteBool(_T("SafeServerConnect"),m_bSafeServerConnect);
	ini.WriteBool(_T("ShowRatesOnTitle"),showRatesInTitle);
	ini.WriteBool(_T("IndicateRatings"),indicateratings);
	ini.WriteBool(_T("WatchClipboard4ED2kFilelinks"),watchclipboard);
	ini.WriteInt(_T("SearchMethod"),m_iSearchMethod);
	ini.WriteBool(_T("CheckDiskspace"),checkDiskspace);
	ini.WriteInt(_T("MinFreeDiskSpace"),m_uMinFreeDiskSpace);
	ini.WriteBool(_T("SparsePartFiles"),m_bSparsePartFiles);
	ini.WriteString(_T("YourHostname"),m_strYourHostname);

	// Barry - New properties...
    ini.WriteBool(_T("AutoConnectStaticOnly"), m_bAutoConnectToStaticServersOnly);
	ini.WriteBool(_T("AutoTakeED2KLinks"), autotakeed2klinks);
    ini.WriteBool(_T("AddNewFilesPaused"), addnewfilespaused);
    ini.WriteInt (_T("3DDepth"), depth3D);  

	ini.WriteString(_T("NotifierConfiguration"), notifierConfiguration);
	ini.WriteBool(_T("NotifyOnDownload"), notifierOnDownloadFinished);
	ini.WriteBool(_T("NotifyOnNewDownload"), notifierOnNewDownload);
	ini.WriteBool(_T("NotifyOnChat"), notifierOnChat);
	ini.WriteBool(_T("NotifyOnLog"), notifierOnLog);
	ini.WriteBool(_T("NotifyOnImportantError"), notifierOnImportantError);
	ini.WriteBool(_T("NotifierPopEveryChatMessage"), notifierOnEveryChatMsg);
	ini.WriteBool(_T("NotifierPopNewVersion"), notifierOnNewVersion);
	ini.WriteInt(_T("NotifierUseSound"), (int)notifierSoundType);
	ini.WriteString(_T("NotifierSoundPath"), notifierSoundFile);

	ini.WriteString(_T("TxtEditor"),TxtEditor);
	ini.WriteString(_T("VideoPlayer"),VideoPlayer);
	ini.WriteString(_T("MessageFilter"),messageFilter);
	ini.WriteString(_T("CommentFilter"),commentFilter);
	ini.WriteString(_T("DateTimeFormat"),GetDateTimeFormat());
	ini.WriteString(_T("DateTimeFormat4Log"),GetDateTimeFormat4Log());
	ini.WriteString(_T("WebTemplateFile"),m_sTemplateFile);
	ini.WriteString(_T("FilenameCleanups"),filenameCleanups);
	ini.WriteInt(_T("ExtractMetaData"),m_iExtractMetaData);

	ini.WriteString(_T("DefaultIRCServerNew"),m_sircserver);
	ini.WriteString(_T("IRCNick"),m_sircnick);
	ini.WriteBool(_T("IRCAddTimestamp"), m_bircaddtimestamp);
	ini.WriteString(_T("IRCFilterName"), m_sircchannamefilter);
	ini.WriteInt(_T("IRCFilterUser"), m_iircchanneluserfilter);
	ini.WriteBool(_T("IRCUseFilter"), m_bircusechanfilter);
	ini.WriteString(_T("IRCPerformString"), m_sircperformstring);
	ini.WriteBool(_T("IRCUsePerform"), m_bircuseperform);
	ini.WriteBool(_T("IRCListOnConnect"), m_birclistonconnect);
	ini.WriteBool(_T("IRCAcceptLink"), m_bircacceptlinks);
	ini.WriteBool(_T("IRCAcceptLinkFriends"), m_bircacceptlinksfriends);
	ini.WriteBool(_T("IRCSoundEvents"), m_bircsoundevents);
	ini.WriteBool(_T("IRCIgnoreMiscMessages"), m_bircignoremiscmessage);
	ini.WriteBool(_T("IRCIgnoreJoinMessages"), m_bircignorejoinmessage);
	ini.WriteBool(_T("IRCIgnorePartMessages"), m_bircignorepartmessage);
	ini.WriteBool(_T("IRCIgnoreQuitMessages"), m_bircignorequitmessage);
	ini.WriteBool(_T("IRCIgnoreEmuleProtoAddFriend"), m_bircignoreemuleprotoaddfriend);
	ini.WriteBool(_T("IRCAllowEmuleProtoAddFriend"), m_bircallowemuleprotoaddfriend);
	ini.WriteBool(_T("IRCIgnoreEmuleProtoSendLink"), m_bircignoreemuleprotosendlink);
	ini.WriteBool(_T("IRCHelpChannel"), m_birchelpchannel);
	ini.WriteBool(_T("SmartIdCheck"), m_bSmartServerIdCheck);
	ini.WriteBool(_T("Verbose"), m_bVerbose);
	ini.WriteBool(_T("DebugSourceExchange"), m_bDebugSourceExchange);	// do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogBannedClients"), m_bLogBannedClients);			// do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogRatingDescReceived"), m_bLogRatingDescReceived);// do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogSecureIdent"), m_bLogSecureIdent);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogFilteredIPs"), m_bLogFilteredIPs);				// do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogFileSaving"), m_bLogFileSaving);				// do *not* use the according 'Get...' function here!
    ini.WriteBool(_T("LogA4AF"), m_bLogA4AF);                           // do *not* use the according 'Get...' function here!
	ini.WriteBool(_T("LogUlDlEvents"), m_bLogUlDlEvents);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	// following options are for debugging or when using an external debug device viewer only.
	ini.WriteInt(_T("DebugServerTCP"),m_iDebugServerTCPLevel);
	ini.WriteInt(_T("DebugServerUDP"),m_iDebugServerUDPLevel);
	ini.WriteInt(_T("DebugServerSources"),m_iDebugServerSourcesLevel);
	ini.WriteInt(_T("DebugServerSearches"),m_iDebugServerSearchesLevel);
	ini.WriteInt(_T("DebugClientTCP"),m_iDebugClientTCPLevel);
	ini.WriteInt(_T("DebugClientUDP"),m_iDebugClientUDPLevel);
	ini.WriteInt(_T("DebugClientKadUDP"),m_iDebugClientKadUDPLevel);
#endif
	ini.WriteBool(_T("PreviewPrio"), m_bpreviewprio);
	ini.WriteBool(_T("UpdateQueueListPref"), m_bupdatequeuelist);
	ini.WriteBool(_T("ManualHighPrio"), m_bManualAddedServersHighPriority);
	ini.WriteBool(_T("FullChunkTransfers"), m_btransferfullchunks);
	ini.WriteBool(_T("ShowOverhead"), m_bshowoverhead);
	ini.WriteBool(_T("VideoPreviewBackupped"), moviePreviewBackup);
	ini.WriteInt(_T("StartNextFile"), m_istartnextfile);

	ini.DeleteKey(_T("FileBufferSizePref")); // delete old 'file buff size' setting
	ini.WriteInt(_T("FileBufferSize"), m_iFileBufferSize);

	ini.DeleteKey(_T("QueueSizePref")); // delete old 'queue size' setting
	ini.WriteInt(_T("QueueSize"), m_iQueueSize);

	ini.WriteInt(_T("CommitFiles"), m_iCommitFiles);
	ini.WriteBool(_T("DAPPref"), m_bDAP);
	ini.WriteBool(_T("UAPPref"), m_bUAP);
	ini.WriteBool(_T("FilterServersByIP"),filterserverbyip);
	ini.WriteBool(_T("DisableKnownClientList"),m_bDisableKnownClientList);
	ini.WriteBool(_T("DisableQueueList"),m_bDisableQueueList);
	ini.WriteBool(_T("UseCreditSystem"),m_bCreditSystem);
	ini.WriteBool(_T("SaveLogToDisk"),log2disk);
	ini.WriteBool(_T("SaveDebugToDisk"),debug2disk);
	ini.WriteBool(_T("EnableScheduler"),scheduler);
	ini.WriteBool(_T("MessagesFromFriendsOnly"),msgonlyfriends);
	ini.WriteBool(_T("MessageFromValidSourcesOnly"),msgsecure);
	ini.WriteBool(_T("ShowInfoOnCatTabs"),showCatTabInfos);
	ini.WriteBool(_T("DontRecreateStatGraphsOnResize"),dontRecreateGraphs);
	ini.WriteBool(_T("AutoFilenameCleanup"),autofilenamecleanup);
	ini.WriteBool(_T("ShowExtControls"),m_bExtControls);
	ini.WriteBool(_T("UseAutocompletion"),m_bUseAutocompl);
	ini.WriteBool(_T("NetworkKademlia"),networkkademlia);
	ini.WriteBool(_T("NetworkED2K"),networked2k);
	ini.WriteBool(_T("AutoClearCompleted"),m_bRemoveFinishedDownloads);
	ini.WriteBool(_T("TransflstRemainOrder"),m_bTransflstRemain);
	ini.WriteBool(_T("UseSimpleTimeRemainingcomputation"),m_bUseOldTimeRemaining);

	ini.WriteInt(_T("VersionCheckLastAutomatic"), versioncheckLastAutomatic);
	ini.WriteInt(_T("FilterLevel"),filterlevel);

	ini.WriteBool(_T("SecureIdent"), m_bUseSecureIdent);// change the name in future version to enable it by default
	ini.WriteBool(_T("AdvancedSpamFilter"),m_bAdvancedSpamfilter);
	ini.WriteBool(_T("ShowDwlPercentage"),m_bShowDwlPercentage);
	ini.WriteBool(_T("RemoveFilesToBin"),m_bRemove2bin);
	//ini.WriteBool(_T("ShowCopyEd2kLinkCmd"),m_bShowCopyEd2kLinkCmd);

	// Toolbar
	ini.WriteString(_T("ToolbarSetting"), m_sToolbarSettings);
	ini.WriteString(_T("ToolbarBitmap"), m_sToolbarBitmap );
	ini.WriteString(_T("ToolbarBitmapFolder"), m_sToolbarBitmapFolder);
	ini.WriteInt(_T("ToolbarLabels"), m_nToolbarLabels);
	ini.WriteInt(_T("ToolbarIconSize"), m_sizToolbarIconSize.cx);
	ini.WriteString(_T("SkinProfile"), m_strSkinProfile);
	ini.WriteString(_T("SkinProfileDir"), m_strSkinProfileDir);

	ini.WriteBinary(_T("HyperTextFont"), (LPBYTE)&m_lfHyperText, sizeof m_lfHyperText);
	ini.WriteBinary(_T("LogTextFont"), (LPBYTE)&m_lfLogText, sizeof m_lfLogText);

	// ZZ:UploadSpeedSense -->
    ini.WriteBool(_T("USSEnabled"), m_bDynUpEnabled);
    ini.WriteBool(_T("USSUseMillisecondPingTolerance"), m_bDynUpUseMillisecondPingTolerance);
    ini.WriteInt(_T("USSPingTolerance"), m_iDynUpPingTolerance);
	ini.WriteInt(_T("USSPingToleranceMilliseconds"), m_iDynUpPingToleranceMilliseconds); // EastShare - Add by TAHO, USS limit
    ini.WriteInt(_T("USSGoingUpDivider"), m_iDynUpGoingUpDivider);
    ini.WriteInt(_T("USSGoingDownDivider"), m_iDynUpGoingDownDivider);
    ini.WriteInt(_T("USSNumberOfPings"), m_iDynUpNumberOfPings);
	// ZZ:UploadSpeedSense <--

    ini.WriteBool(_T("A4AFSaveCpu"), m_bA4AFSaveCpu); // ZZ:DownloadManager
	ini.WriteInt(_T("WebMirrorAlertLevel"), m_nWebMirrorAlertLevel);
	ini.WriteBool(_T("RunAsUnprivilegedUser"), m_bRunAsUser);
	ini.WriteBool(_T("OpenPortsOnStartUp"), m_bOpenPortsOnStartUp);
	ini.WriteInt(_T("DebugLogLevel"), m_byLogLevel);
	ini.WriteInt(_T("WinXPSP2"), IsRunningXPSP2());
	ini.WriteBool(_T("RememberCancelledFiles"), m_bRememberCancelledFiles);
	ini.WriteBool(_T("RememberDownloadedFiles"), m_bRememberDownloadedFiles);

	ini.WriteBool(_T("NotifierSendMail"), m_bNotifierSendMail);
	ini.WriteString(_T("NotifierMailSender"), m_strNotifierMailSender);
	ini.WriteString(_T("NotifierMailServer"), m_strNotifierMailServer);
	ini.WriteString(_T("NotifierMailRecipient"), m_strNotifierMailReceiver);

	ini.WriteBool(_T("WinaTransToolbar"), m_bWinaTransToolbar);

	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	ini.WriteBool(_T("ProxyEnablePassword"),proxy.EnablePassword,_T("Proxy"));
	ini.WriteBool(_T("ProxyEnableProxy"),proxy.UseProxy,_T("Proxy"));
	ini.WriteString(_T("ProxyName"),proxy.name,_T("Proxy"));
	ini.WriteString(_T("ProxyPassword"),A2CT(proxy.password),_T("Proxy"));
	ini.WriteString(_T("ProxyUser"),A2CT(proxy.user),_T("Proxy"));
	ini.WriteInt(_T("ProxyPort"),proxy.port,_T("Proxy"));
	ini.WriteInt(_T("ProxyType"),proxy.type,_T("Proxy"));
	ini.WriteBool(_T("ConnectWithoutProxy"),m_bIsASCWOP,_T("Proxy"));
	ini.WriteBool(_T("ShowErrors"),m_bShowProxyErrors,_T("Proxy"));


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	ini.WriteInt(_T("statsConnectionsGraphRatio"), statsConnectionsGraphRatio,_T("Statistics"));
	ini.WriteString(_T("statsExpandedTreeItems"), statsExpandedTreeItems);
	CString buffer2;
	for (int i=0;i<15;i++) {
		buffer.Format(_T("0x%06x"),GetStatsColor(i));
		buffer2.Format(_T("StatColor%i"),i);
		ini.WriteString(buffer2,buffer,_T("Statistics") );
	}


	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	ini.WriteString(_T("Password"), GetWSPass(), _T("WebServer"));
	ini.WriteString(_T("PasswordLow"), GetWSLowPass());
	ini.WriteInt(_T("Port"), m_nWebPort);
	ini.WriteBool(_T("Enabled"), m_bWebEnabled);
	ini.WriteBool(_T("UseGzip"), m_bWebUseGzip);
	ini.WriteInt(_T("PageRefreshTime"), m_nWebPageRefresh);
	ini.WriteBool(_T("UseLowRightsUser"), m_bWebLowEnabled);
	ini.WriteBool(_T("AllowAdminHiLevelFunc"),m_bAllowAdminHiLevFunc);
	ini.WriteInt(_T("WebTimeoutMins"), m_iWebTimeoutMins);


	///////////////////////////////////////////////////////////////////////////
	// Section: "MobileMule"
	//
	ini.WriteString(_T("Password"), GetMMPass(), _T("MobileMule"));
	ini.WriteBool(_T("Enabled"), m_bMMEnabled);
	ini.WriteInt(_T("Port"), m_nMMPort);


	///////////////////////////////////////////////////////////////////////////
	// Section: "PeerCache"
	//
	ini.WriteInt(_T("LastSearch"), m_uPeerCacheLastSearch, _T("PeerCache"));
	ini.WriteBool(_T("Found"), m_bPeerCacheWasFound);
	ini.WriteBool(_T("Enabled"), m_bPeerCacheEnabled);
	ini.WriteInt(_T("PCPort"), m_nPeerCachePort);
}

void CPreferences::ResetStatsColor(int index)
{
	switch(index)
	{
		case  0: m_adwStatsColors[ 0]=RGB(  0,  0, 64);break;
		case  1: m_adwStatsColors[ 1]=RGB(192,192,255);break;
		case  2: m_adwStatsColors[ 2]=RGB(128,255,128);break;
		case  3: m_adwStatsColors[ 3]=RGB(  0,210,  0);break;
		case  4: m_adwStatsColors[ 4]=RGB(  0,128,  0);break;
		case  5: m_adwStatsColors[ 5]=RGB(255,128,128);break;
		case  6: m_adwStatsColors[ 6]=RGB(200,  0,  0);break;
		case  7: m_adwStatsColors[ 7]=RGB(140,  0,  0);break;
		case  8: m_adwStatsColors[ 8]=RGB(150,150,255);break;
		case  9: m_adwStatsColors[ 9]=RGB(192,  0,192);break;
		case 10: m_adwStatsColors[10]=RGB(255,255,128);break;
		case 11: m_adwStatsColors[11]=RGB(  0,  0,  0);break;
		case 12: m_adwStatsColors[12]=RGB(255,255,255);break;
		case 13: m_adwStatsColors[13]=RGB(255,255,255);break;
		case 14: m_adwStatsColors[14]=RGB(255,190,190);break;
	}
}

void CPreferences::GetAllStatsColors(int iCount, LPDWORD pdwColors)
{
	memset(pdwColors, 0, sizeof(*pdwColors) * iCount);
	memcpy(pdwColors, m_adwStatsColors, sizeof(*pdwColors) * min(ARRSIZE(m_adwStatsColors), iCount));
}

bool CPreferences::SetAllStatsColors(int iCount, const DWORD* pdwColors)
{
	bool bModified = false;
	int iMin = min(ARRSIZE(m_adwStatsColors), iCount);
	for (int i = 0; i < iMin; i++)
	{
		if (m_adwStatsColors[i] != pdwColors[i])
		{
			m_adwStatsColors[i] = pdwColors[i];
			bModified = true;
		}
	}
	return bModified;
}

void CPreferences::IniCopy(CString si, CString di) {
	CIni ini(GetConfigFile(), _T("eMule"));
	
	CString s=ini.GetString(si);

	ini.SetSection(_T("ListControlSetup"));
	
	ini.WriteString(di,s);
}

// Imports the tablesetups of emuleversions (.ini) <0.46b		- temporary
void CPreferences::ImportOldTableSetup() {

	IniCopy(_T("DownloadColumnHidden") ,	_T("DownloadListCtrlColumnHidden") );
	IniCopy(_T("DownloadColumnWidths") ,	_T("DownloadListCtrlColumnWidths") );
	IniCopy(_T("DownloadColumnOrder") ,		_T("DownloadListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemDownload") ,	_T("DownloadListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingDownload") , _T("DownloadListCtrlTableSortAscending") );

	IniCopy(_T("ONContactListCtrlColumnHidden") ,	_T("ONContactListCtrlColumnHidden") );
	IniCopy(_T("ONContactListCtrlColumnWidths") ,	_T("ONContactListCtrlColumnWidths") );
	IniCopy(_T("ONContactListCtrlColumnOrders") ,		_T("ONContactListCtrlColumnOrders") );

	IniCopy(_T("KadSearchListCtrlColumnHidden") ,	_T("KadSearchListCtrlColumnHidden") );
	IniCopy(_T("KadSearchListCtrlColumnWidths") ,	_T("KadSearchListCtrlColumnWidths") );
	IniCopy(_T("KadSearchListCtrlColumnOrders") ,		_T("KadSearchListCtrlColumnOrders") );
	
	IniCopy(_T("UploadColumnHidden") ,		_T("UploadListCtrlColumnHidden") );
	IniCopy(_T("UploadColumnWidths") ,		_T("UploadListCtrlColumnWidths") );
	IniCopy(_T("UploadColumnOrder") ,		_T("UploadListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemUpload") ,		_T("UploadListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingUpload"), _T("UploadListCtrlTableSortAscending") );

	IniCopy(_T("QueueColumnHidden") ,		_T("QueueListCtrlColumnHidden") );
	IniCopy(_T("QueueColumnWidths") ,		_T("QueueListCtrlColumnWidths") );
	IniCopy(_T("QueueColumnOrder") ,		_T("QueueListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemQueue") ,		_T("QueueListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingQueue") , _T("QueueListCtrlTableSortAscending") );

	IniCopy(_T("SearchColumnHidden") ,		_T("SearchListCtrlColumnHidden") );
	IniCopy(_T("SearchColumnWidths") ,		_T("SearchListCtrlColumnWidths") );
	IniCopy(_T("SearchColumnOrder") ,		_T("SearchListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemSearch") ,		_T("SearchListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingSearch"), _T("SearchListCtrlTableSortAscending") );

	IniCopy(_T("SharedColumnHidden") ,		_T("SharedFilesCtrlColumnHidden") );
	IniCopy(_T("SharedColumnWidths") ,		_T("SharedFilesCtrlColumnWidths") );
	IniCopy(_T("SharedColumnOrder") ,		_T("SharedFilesCtrlColumnOrders") );
	IniCopy(_T("TableSortItemShared") ,		_T("SharedFilesCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingShared"), _T("SharedFilesCtrlTableSortAscending") );

	IniCopy(_T("ServerColumnHidden") ,		_T("ServerListCtrlColumnHidden") );
	IniCopy(_T("ServerColumnWidths") ,		_T("ServerListCtrlColumnWidths") );
	IniCopy(_T("ServerColumnOrder") ,		_T("ServerListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemServer") ,		_T("ServerListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingServer"), _T("ServerListCtrlTableSortAscending") );

	IniCopy(_T("ClientListColumnHidden") ,		_T("ClientListCtrlColumnHidden") );
	IniCopy(_T("ClientListColumnWidths") ,		_T("ClientListCtrlColumnWidths") );
	IniCopy(_T("ClientListColumnOrder") ,		_T("ClientListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemClientList") ,		_T("ClientListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingClientList"), _T("ClientListCtrlTableSortAscending") );

	IniCopy(_T("FilenamesListColumnHidden") ,	_T("FileDetailDlgNameColumnHidden") );
	IniCopy(_T("FilenamesListColumnWidths") ,	_T("FileDetailDlgNameColumnWidths") );
	IniCopy(_T("FilenamesListColumnOrder") ,	_T("FileDetailDlgNameColumnOrders") );
	IniCopy(_T("TableSortItemFilenames") ,		_T("FileDetailDlgNameTableSortItem") );
	IniCopy(_T("TableSortAscendingFilenames"),  _T("FileDetailDlgNameTableSortAscending") );

	IniCopy(_T("IrcMainColumnHidden") ,		_T("IrcNickListCtrlColumnHidden") );
	IniCopy(_T("IrcMainColumnWidths") ,		_T("IrcNickListCtrlColumnWidths") );
	IniCopy(_T("IrcMainColumnOrder") ,		_T("IrcNickListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemIrcMain") ,	_T("IrcNickListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingIrcMain"),_T("IrcNickListCtrlTableSortAscending") );

	IniCopy(_T("IrcChannelsColumnHidden") ,		_T("IrcChannelListCtrlColumnHidden") );
	IniCopy(_T("IrcChannelsColumnWidths") ,		_T("IrcChannelListCtrlColumnWidths") );
	IniCopy(_T("IrcChannelsColumnOrder") ,		_T("IrcChannelListCtrlColumnOrders") );
	IniCopy(_T("TableSortItemIrcChannels") ,	_T("IrcChannelListCtrlTableSortItem") );
	IniCopy(_T("TableSortAscendingIrcChannels"),_T("IrcChannelListCtrlTableSortAscending") );

	IniCopy(_T("DownloadClientsColumnHidden") ,		_T("DownloadClientsCtrlColumnHidden") );
	IniCopy(_T("DownloadClientsColumnWidths") ,		_T("DownloadClientsCtrlColumnWidths") );
	IniCopy(_T("DownloadClientsColumnOrder") ,		_T("DownloadClientsCtrlColumnOrders") );
}

void CPreferences::LoadPreferences()
{
	USES_CONVERSION;
	TCHAR buffer[256];

	CIni ini(GetConfigFile(), _T("eMule"));

	// import old (<0.46b) table setups - temporary
	if (ini.GetInt(_T("SearchListCtrlTableSortItem"),-1,_T("ListControlSetup"))==-1)
		ImportOldTableSetup();
	ini.SetSection(_T("eMule"));

	CString strCurrVersion, strPrefsVersion;

	strCurrVersion = theApp.m_strCurVersionLong;
	strPrefsVersion = ini.GetString(_T("AppVersion"));

	m_bFirstStart = false;

	if (strCurrVersion != strPrefsVersion){
		m_bFirstStart = true;
	}

#ifdef _DEBUG
	m_iDbgHeap = ini.GetInt(_T("DebugHeap"), 1);
#else
	m_iDbgHeap = 0;
#endif

	m_nWebMirrorAlertLevel = ini.GetInt(_T("WebMirrorAlertLevel"),0);
	updatenotify=ini.GetBool(_T("UpdateNotifyTestClient"),true);

	SetUserNick(ini.GetStringUTF8(_T("Nick"), DEFAULT_NICK));
	if (strNick.IsEmpty() || IsDefaultNick(strNick))
		SetUserNick(DEFAULT_NICK);

	_stprintf(buffer,_T("%sIncoming"),appdir);
	_stprintf(incomingdir,_T("%s"),ini.GetString(_T("IncomingDir"),buffer ));
	MakeFoldername(incomingdir);

	// load tempdir(s) setting
	_stprintf(buffer,_T("%sTemp"),appdir);

	CString tempdirs;
	tempdirs=ini.GetString(_T("TempDir"),buffer);
	tempdirs+= _T("|") + ini.GetString(_T("TempDirs"));

	int curPos=0;
	bool doubled;
	CString atmp=tempdirs.Tokenize(_T("|"), curPos);
	while (!atmp.IsEmpty())
	{
		atmp.Trim();
		if (!atmp.IsEmpty()) {
			MakeFoldername(atmp.GetBuffer(MAX_PATH));
			atmp.ReleaseBuffer();
			doubled=false;
			for (int i=0;i<tempdir.GetCount();i++)	// avoid double tempdirs
				if (atmp.CompareNoCase(GetTempDir(i))==0) {
					doubled=true;
					break;
				}
			if (!doubled) {
				if (PathFileExists(atmp)==FALSE) {
					CreateDirectory(atmp,NULL);
					if (PathFileExists(atmp)==TRUE || tempdir.GetCount()==0)
						tempdir.Add(atmp);
				}
				else
					tempdir.Add(atmp);
			}
		}
		atmp = tempdirs.Tokenize(_T("|"), curPos);
	}

	maxGraphDownloadRate=ini.GetInt(_T("DownloadCapacity"),96);
	if (maxGraphDownloadRate==0)
		maxGraphDownloadRate=96;
	
	maxGraphUploadRate = ini.GetInt(_T("UploadCapacityNew"),-1);
	if (maxGraphUploadRate == 0)
		maxGraphUploadRate = UNLIMITED;
	else if (maxGraphUploadRate == -1){
		// converting value from prior versions
		int nOldUploadCapacity = ini.GetInt(_T("UploadCapacity"), 16);
		if (nOldUploadCapacity == 16 && ini.GetInt(_T("MaxUpload"),12) == 12){
			// either this is a complete new install, or the prior version used the default value
			// in both cases, set the new default values to unlimited
			maxGraphUploadRate = UNLIMITED;
			ini.WriteInt(_T("MaxUpload"),UNLIMITED, _T("eMule"));
		}
		else
			maxGraphUploadRate = nOldUploadCapacity; // use old custoum value
	}

	minupload=ini.GetInt(_T("MinUpload"), 1);
	maxupload=ini.GetInt(_T("MaxUpload"),UNLIMITED);
	if (maxupload > maxGraphUploadRate && maxupload != UNLIMITED)
		maxupload = (uint16)(maxGraphUploadRate * .8);
	
	maxdownload=ini.GetInt(_T("MaxDownload"), UNLIMITED);
	if (maxdownload > maxGraphDownloadRate && maxdownload != UNLIMITED) maxdownload = (uint16)(maxGraphDownloadRate * .8);
	maxconnections=ini.GetInt(_T("MaxConnections"),GetRecommendedMaxConnections());
	maxhalfconnections=ini.GetInt(_T("MaxHalfConnections"),9);
	m_bConditionalTCPAccept = ini.GetBool(_T("ConditionalTCPAccept"), false);

	// reset max halfopen to a default if OS changed to SP2 or away
	int dwSP2 = ini.GetInt(_T("WinXPSP2"), -1);
	int dwCurSP2 = IsRunningXPSP2();
	if (dwSP2 != dwCurSP2){
		if (dwCurSP2 == 0)
			maxhalfconnections = 50;
		else if (dwCurSP2 == 1)
			maxhalfconnections = 9;
	}

	port=ini.GetInt(_T("Port"), DEFAULT_TCP_PORT);
	udpport=ini.GetInt(_T("UDPPort"),port+10);
	nServerUDPPort = ini.GetInt(_T("ServerUDPPort"), -1); // 0 = Don't use UDP port for servers, -1 = use a random port (for backward compatibility)
	maxsourceperfile=ini.GetInt(_T("MaxSourcesPerFile"),400 );
	m_wLanguageID=ini.GetWORD(_T("Language"),0);
	m_iSeeShares=(EViewSharedFilesAccess)ini.GetInt(_T("SeeShare"),vsfaNobody);
	m_iToolDelayTime=ini.GetInt(_T("ToolTipDelay"),1);
	trafficOMeterInterval=ini.GetInt(_T("StatGraphsInterval"),3);
	statsInterval=ini.GetInt(_T("statsInterval"),5);
	dontcompressavi=ini.GetBool(_T("DontCompressAvi"),false);

	m_uDeadServerRetries=ini.GetInt(_T("DeadServerRetry"),1);
	if (m_uDeadServerRetries > MAX_SERVERFAILCOUNT)
		m_uDeadServerRetries = MAX_SERVERFAILCOUNT;
	m_dwServerKeepAliveTimeout=ini.GetInt(_T("ServerKeepAliveTimeout"),0);
	splitterbarPosition=ini.GetInt(_T("SplitterbarPosition"),75);
	if (splitterbarPosition < 9)
		splitterbarPosition = 9;
	else if (splitterbarPosition > 93)
		splitterbarPosition = 93;
	splitterbarPositionStat=ini.GetInt(_T("SplitterbarPositionStat"),30);
	splitterbarPositionStat_HL=ini.GetInt(_T("SplitterbarPositionStat_HL"),66);
	splitterbarPositionStat_HR=ini.GetInt(_T("SplitterbarPositionStat_HR"),33);
	if (splitterbarPositionStat_HR+1>=splitterbarPositionStat_HL){
		splitterbarPositionStat_HL = 66;
		splitterbarPositionStat_HR = 33;
	}
	splitterbarPositionFriend=ini.GetInt(_T("SplitterbarPositionFriend"),300);
	splitterbarPositionShared=ini.GetInt(_T("SplitterbarPositionShared"),179);
	splitterbarPositionIRC=ini.GetInt(_T("SplitterbarPositionIRC"),200);
	splitterbarPositionSvr=ini.GetInt(_T("SplitterbarPositionServer"),75);
	if (splitterbarPositionSvr>90 || splitterbarPositionSvr<10)
		splitterbarPositionSvr=75;

	m_uTransferWnd1 = ini.GetInt(_T("TransferWnd1"),0);
	m_uTransferWnd2 = ini.GetInt(_T("TransferWnd2"),0);

	statsMax=ini.GetInt(_T("VariousStatisticsMaxValue"),100);
	statsAverageMinutes=ini.GetInt(_T("StatsAverageMinutes"),5);
	MaxConperFive=ini.GetInt(_T("MaxConnectionsPerFiveSeconds"),GetDefaultMaxConperFive());

	reconnect = ini.GetBool(_T("Reconnect"), true);
	m_bUseServerPriorities = ini.GetBool(_T("Scoresystem"), true);
	ICH = ini.GetBool(_T("ICH"), true);
	m_bAutoUpdateServerList = ini.GetBool(_T("Serverlist"), false);

	mintotray=ini.GetBool(_T("MinToTray"),false);
	m_bAddServersFromServer=ini.GetBool(_T("AddServersFromServer"),true);
	m_bAddServersFromClients=ini.GetBool(_T("AddServersFromClient"),true);
	splashscreen=ini.GetBool(_T("Splashscreen"),true);
	bringtoforeground=ini.GetBool(_T("BringToFront"),true);
	transferDoubleclick=ini.GetBool(_T("TransferDoubleClick"),true);
	beepOnError=ini.GetBool(_T("BeepOnError"),true);
	confirmExit=ini.GetBool(_T("ConfirmExit"),true);
	filterLANIPs=ini.GetBool(_T("FilterBadIPs"),true);
	m_bAllocLocalHostIP=ini.GetBool(_T("AllowLocalHostIP"),false);
	autoconnect=ini.GetBool(_T("Autoconnect"),false);
	showRatesInTitle=ini.GetBool(_T("ShowRatesOnTitle"),false);

	onlineSig=ini.GetBool(_T("OnlineSignature"),false);
	startMinimized=ini.GetBool(_T("StartupMinimized"),false);
	m_bAutoStart=ini.GetBool(_T("AutoStart"),false);
	m_bRestoreLastMainWndDlg=ini.GetBool(_T("RestoreLastMainWndDlg"),false);
	m_iLastMainWndDlgID=ini.GetInt(_T("LastMainWndDlgID"),0);
	m_bRestoreLastLogPane=ini.GetBool(_T("RestoreLastLogPane"),false);
	m_iLastLogPaneID=ini.GetInt(_T("LastLogPaneID"),0);
	m_bSafeServerConnect =ini.GetBool(_T("SafeServerConnect"),false);

	m_bTransflstRemain =ini.GetBool(_T("TransflstRemainOrder"),false);
	filterserverbyip=ini.GetBool(_T("FilterServersByIP"),false);
	filterlevel=ini.GetInt(_T("FilterLevel"),127);
	checkDiskspace=ini.GetBool(_T("CheckDiskspace"),false);
	m_uMinFreeDiskSpace=ini.GetInt(_T("MinFreeDiskSpace"),20*1024*1024);
	m_bSparsePartFiles=ini.GetBool(_T("SparsePartFiles"),false);
	m_strYourHostname=ini.GetString(_T("YourHostname"), _T(""));

	// Barry - New properties...
	m_bAutoConnectToStaticServersOnly = ini.GetBool(_T("AutoConnectStaticOnly"),false); 
	autotakeed2klinks = ini.GetBool(_T("AutoTakeED2KLinks"),true); 
	addnewfilespaused = ini.GetBool(_T("AddNewFilesPaused"),false); 
	depth3D = ini.GetInt(_T("3DDepth"), 5);
	m_bEnableMiniMule = ini.GetBool(_T("MiniMule"), true);

	// Notifier
	notifierConfiguration = ini.GetString(_T("NotifierConfiguration"));
    notifierOnDownloadFinished = ini.GetBool(_T("NotifyOnDownload"));
	notifierOnNewDownload = ini.GetBool(_T("NotifyOnNewDownload"));
    notifierOnChat = ini.GetBool(_T("NotifyOnChat"));
    notifierOnLog = ini.GetBool(_T("NotifyOnLog"));
	notifierOnImportantError = ini.GetBool(_T("NotifyOnImportantError"));
	notifierOnEveryChatMsg = ini.GetBool(_T("NotifierPopEveryChatMessage"));
	notifierOnNewVersion = ini.GetBool(_T("NotifierPopNewVersion"));
    notifierSoundType = (ENotifierSoundType)ini.GetInt(_T("NotifierUseSound"), ntfstNoSound);
	notifierSoundFile = ini.GetString(_T("NotifierSoundPath"));

	_stprintf(datetimeformat,_T("%s"),ini.GetString(_T("DateTimeFormat"),_T("%A, %x, %X")));
	if (_tcslen(datetimeformat)==0) _tcscpy(datetimeformat,_T("%A, %x, %X"));
	_stprintf(datetimeformat4log,_T("%s"),ini.GetString(_T("DateTimeFormat4Log"),_T("%c")));
	if (_tcslen(datetimeformat4log)==0) _tcscpy(datetimeformat4log,_T("%c"));

	_stprintf(m_sircserver,_T("%s"),ini.GetString(_T("DefaultIRCServerNew"),_T("ircchat.emule-project.net")));
	_stprintf(m_sircnick,_T("%s"),ini.GetString(_T("IRCNick")));
	m_bircaddtimestamp=ini.GetBool(_T("IRCAddTimestamp"),true);
	_stprintf(m_sircchannamefilter,_T("%s"),ini.GetString(_T("IRCFilterName"), _T("") ));
	m_bircusechanfilter=ini.GetBool(_T("IRCUseFilter"), false);
	m_iircchanneluserfilter=ini.GetInt(_T("IRCFilterUser"), 0);
	_stprintf(m_sircperformstring,_T("%s"),ini.GetString(_T("IRCPerformString"), _T("") ));
	m_bircuseperform=ini.GetBool(_T("IRCUsePerform"), false);
	m_birclistonconnect=ini.GetBool(_T("IRCListOnConnect"), true);
	m_bircacceptlinks=ini.GetBool(_T("IRCAcceptLink"), true);
	m_bircacceptlinksfriends=ini.GetBool(_T("IRCAcceptLinkFriends"), true);
	m_bircsoundevents=ini.GetBool(_T("IRCSoundEvents"), false);
	m_bircignoremiscmessage=ini.GetBool(_T("IRCIgnoreMiscMessages"), false);
	m_bircignorejoinmessage=ini.GetBool(_T("IRCIgnoreJoinMessages"), true);
	m_bircignorepartmessage=ini.GetBool(_T("IRCIgnorePartMessages"), true);
	m_bircignorequitmessage=ini.GetBool(_T("IRCIgnoreQuitMessages"), true);
	m_bircignoreemuleprotoaddfriend=ini.GetBool(_T("IRCIgnoreEmuleProtoAddFriend"), false);
	m_bircallowemuleprotoaddfriend=ini.GetBool(_T("IRCAllowEmuleProtoAddFriend"), true);
	m_bircignoreemuleprotosendlink=ini.GetBool(_T("IRCIgnoreEmuleProtoSendLink"), false);
	m_birchelpchannel=ini.GetBool(_T("IRCHelpChannel"),true);
	m_bSmartServerIdCheck=ini.GetBool(_T("SmartIdCheck"),true);

	log2disk = ini.GetBool(_T("SaveLogToDisk"),false);
	uMaxLogFileSize = ini.GetInt(_T("MaxLogFileSize"), 1024*1024);
	iMaxLogBuff = ini.GetInt(_T("MaxLogBuff"),64) * 1024;
	m_iLogFileFormat = (ELogFileFormat)ini.GetInt(_T("LogFileFormat"), Unicode);
	m_bEnableVerboseOptions=ini.GetBool(_T("VerboseOptions"), true);
	if (m_bEnableVerboseOptions)
	{
		m_bVerbose=ini.GetBool(_T("Verbose"),false);
		m_bFullVerbose=ini.GetBool(_T("FullVerbose"),false);
		debug2disk=ini.GetBool(_T("SaveDebugToDisk"),false);
		m_bDebugSourceExchange=ini.GetBool(_T("DebugSourceExchange"),false);
		m_bLogBannedClients=ini.GetBool(_T("LogBannedClients"), true);
		m_bLogRatingDescReceived=ini.GetBool(_T("LogRatingDescReceived"),true);
		m_bLogSecureIdent=ini.GetBool(_T("LogSecureIdent"),true);
		m_bLogFilteredIPs=ini.GetBool(_T("LogFilteredIPs"),true);
		m_bLogFileSaving=ini.GetBool(_T("LogFileSaving"),false);
        m_bLogA4AF=ini.GetBool(_T("LogA4AF"),false); // ZZ:DownloadManager
		m_bLogUlDlEvents=ini.GetBool(_T("LogUlDlEvents"),true);
	}
	else
	{
		if (m_bRestoreLastLogPane && m_iLastLogPaneID>=2)
			m_iLastLogPaneID = 1;
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	// following options are for debugging or when using an external debug device viewer only.
	m_iDebugServerTCPLevel = ini.GetInt(_T("DebugServerTCP"), 0);
	m_iDebugServerUDPLevel = ini.GetInt(_T("DebugServerUDP"), 0);
	m_iDebugServerSourcesLevel = ini.GetInt(_T("DebugServerSources"), 0);
	m_iDebugServerSearchesLevel = ini.GetInt(_T("DebugServerSearches"), 0);
	m_iDebugClientTCPLevel = ini.GetInt(_T("DebugClientTCP"), 0);
	m_iDebugClientUDPLevel = ini.GetInt(_T("DebugClientUDP"), 0);
	m_iDebugClientKadUDPLevel = ini.GetInt(_T("DebugClientKadUDP"), 0);
	m_iDebugSearchResultDetailLevel = ini.GetInt(_T("DebugSearchResultDetailLevel"), 0);
#else
	// for normal release builds ensure that those options are all turned off
	m_iDebugServerTCPLevel = 0;
	m_iDebugServerUDPLevel = 0;
	m_iDebugServerSourcesLevel = 0;
	m_iDebugServerSearchesLevel = 0;
	m_iDebugClientTCPLevel = 0;
	m_iDebugClientUDPLevel = 0;
	m_iDebugClientKadUDPLevel = 0;
	m_iDebugSearchResultDetailLevel = 0;
#endif

	m_bpreviewprio=ini.GetBool(_T("PreviewPrio"),false);
	m_bupdatequeuelist=ini.GetBool(_T("UpdateQueueListPref"),false);
	m_bManualAddedServersHighPriority=ini.GetBool(_T("ManualHighPrio"),false);
	m_btransferfullchunks=ini.GetBool(_T("FullChunkTransfers"),true);
	m_istartnextfile=ini.GetInt(_T("StartNextFile"),0);
	m_bshowoverhead=ini.GetBool(_T("ShowOverhead"),false);
	moviePreviewBackup=ini.GetBool(_T("VideoPreviewBackupped"),true);
	m_iPreviewSmallBlocks=ini.GetInt(_T("PreviewSmallBlocks"), 0);
	m_bPreviewCopiedArchives=ini.GetBool(_T("PreviewCopiedArchives"), true);
	m_iInspectAllFileTypes=ini.GetInt(_T("InspectAllFileTypes"), 0);

	// read file buffer size (with backward compatibility)
	m_iFileBufferSize=ini.GetInt(_T("FileBufferSizePref"),0); // old setting
	if (m_iFileBufferSize == 0)
		m_iFileBufferSize = 256*1024;
	else
		m_iFileBufferSize = ((m_iFileBufferSize*15000 + 512)/1024)*1024;
	m_iFileBufferSize=ini.GetInt(_T("FileBufferSize"),m_iFileBufferSize);

	// read queue size (with backward compatibility)
	m_iQueueSize=ini.GetInt(_T("QueueSizePref"),0); // old setting
	if (m_iQueueSize == 0)
		m_iQueueSize = 50*100;
	else
		m_iQueueSize = m_iQueueSize*100;
	m_iQueueSize=ini.GetInt(_T("QueueSize"),m_iQueueSize);

	m_iCommitFiles=ini.GetInt(_T("CommitFiles"), 1); // 1 = "commit" on application shut down; 2 = "commit" on each file saveing
	versioncheckdays=ini.GetInt(_T("Check4NewVersionDelay"),5);
	m_bDAP=ini.GetBool(_T("DAPPref"),true);
	m_bUAP=ini.GetBool(_T("UAPPref"),true);
	m_bPreviewOnIconDblClk=ini.GetBool(_T("PreviewOnIconDblClk"),false);
	indicateratings=ini.GetBool(_T("IndicateRatings"),true);
	watchclipboard=ini.GetBool(_T("WatchClipboard4ED2kFilelinks"),false);
	m_iSearchMethod=ini.GetInt(_T("SearchMethod"),0);

	showCatTabInfos=ini.GetBool(_T("ShowInfoOnCatTabs"),false);
//	resumeSameCat=ini.GetBool(_T("ResumeNextFromSameCat"),false);
	dontRecreateGraphs =ini.GetBool(_T("DontRecreateStatGraphsOnResize"),false);
	m_bExtControls =ini.GetBool(_T("ShowExtControls"),false);

	versioncheckLastAutomatic=ini.GetInt(_T("VersionCheckLastAutomatic"),0);
	m_bDisableKnownClientList=ini.GetBool(_T("DisableKnownClientList"),false);
	m_bDisableQueueList=ini.GetBool(_T("DisableQueueList"),false);
	m_bCreditSystem=ini.GetBool(_T("UseCreditSystem"),true);
	scheduler=ini.GetBool(_T("EnableScheduler"),false);
	msgonlyfriends=ini.GetBool(_T("MessagesFromFriendsOnly"),false);
	msgsecure=ini.GetBool(_T("MessageFromValidSourcesOnly"),true);
	autofilenamecleanup=ini.GetBool(_T("AutoFilenameCleanup"),false);
	m_bUseAutocompl=ini.GetBool(_T("UseAutocompletion"),true);
	m_bShowDwlPercentage=ini.GetBool(_T("ShowDwlPercentage"),false);
	networkkademlia=ini.GetBool(_T("NetworkKademlia"),false);
	networked2k=ini.GetBool(_T("NetworkED2K"),true);
	m_bRemove2bin=ini.GetBool(_T("RemoveFilesToBin"),true);
	m_bShowCopyEd2kLinkCmd=ini.GetBool(_T("ShowCopyEd2kLinkCmd"),false);

	m_iMaxChatHistory=ini.GetInt(_T("MaxChatHistoryLines"),100);
	if (m_iMaxChatHistory < 1)
		m_iMaxChatHistory = 100;
	maxmsgsessions=ini.GetInt(_T("MaxMessageSessions"),50);
	m_bShowActiveDownloadsBold = ini.GetBool(_T("ShowActiveDownloadsBold"), false);

	_stprintf(TxtEditor,_T("%s"),ini.GetString(_T("TxtEditor"),_T("notepad.exe")));
	_stprintf(VideoPlayer,_T("%s"),ini.GetString(_T("VideoPlayer"),_T("")));
	
	_stprintf(m_sTemplateFile,_T("%s"),ini.GetString(_T("WebTemplateFile"), GetConfigDir()+_T("eMule.tmpl")));

	_stprintf(messageFilter,_T("%s"),ini.GetString(_T("MessageFilter"),_T("Your client has an infinite queue|Your client is connecting too fast|fastest download speed")));
	commentFilter = ini.GetString(_T("CommentFilter"),_T("http://|https://|www."));
	commentFilter.MakeLower();
	_stprintf(filenameCleanups,_T("%s"),ini.GetString(_T("FilenameCleanups"),_T("http|www.|.com|.de|.org|.net|shared|powered|sponsored|sharelive|filedonkey|")));
	m_iExtractMetaData = ini.GetInt(_T("ExtractMetaData"), 1); // 0=disable, 1=mp3, 2=MediaDet
	if (m_iExtractMetaData > 1)
		m_iExtractMetaData = 1;
	m_bAdjustNTFSDaylightFileTime=ini.GetBool(_T("AdjustNTFSDaylightFileTime"), true);

	m_bUseSecureIdent=ini.GetBool(_T("SecureIdent"),true);
	m_bAdvancedSpamfilter=ini.GetBool(_T("AdvancedSpamFilter"),true);
	m_bRemoveFinishedDownloads=ini.GetBool(_T("AutoClearCompleted"),false);
	m_bUseOldTimeRemaining= ini.GetBool(_T("UseSimpleTimeRemainingcomputation"),false);

	// Toolbar
	m_sToolbarSettings = ini.GetString(_T("ToolbarSetting"), strDefaultToolbar);
	m_sToolbarBitmap = ini.GetString(_T("ToolbarBitmap"), _T(""));
	m_sToolbarBitmapFolder = ini.GetString(_T("ToolbarBitmapFolder"), appdir + _T("skins"));
	m_nToolbarLabels = (EToolbarLabelType)ini.GetInt(_T("ToolbarLabels"), CMuleToolbarCtrl::GetDefaultLabelType());
	m_bReBarToolbar = ini.GetBool(_T("ReBarToolbar"), 1);
	m_sizToolbarIconSize.cx = m_sizToolbarIconSize.cy = ini.GetInt(_T("ToolbarIconSize"), 32);
	m_iStraightWindowStyles=ini.GetInt(_T("StraightWindowStyles"),0);
	m_bRTLWindowsLayout = ini.GetBool(_T("RTLWindowsLayout"));
	m_strSkinProfile = ini.GetString(_T("SkinProfile"), _T(""));
	m_strSkinProfileDir = ini.GetString(_T("SkinProfileDir"), appdir + _T("skins"));

	LPBYTE pData = NULL;
	UINT uSize = sizeof m_lfHyperText;
	if (ini.GetBinary(_T("HyperTextFont"), &pData, &uSize) && uSize == sizeof m_lfHyperText)
		memcpy(&m_lfHyperText, pData, sizeof m_lfHyperText);
	else
		memset(&m_lfHyperText, 0, sizeof m_lfHyperText);
	delete[] pData;

	pData = NULL;
	uSize = sizeof m_lfLogText;
	if (ini.GetBinary(_T("LogTextFont"), &pData, &uSize) && uSize == sizeof m_lfLogText)
		memcpy(&m_lfLogText, pData, sizeof m_lfLogText);
	else
		memset(&m_lfLogText, 0, sizeof m_lfLogText);
	delete[] pData;

	m_crLogError = ini.GetColRef(_T("LogErrorColor"), m_crLogError);
	m_crLogWarning = ini.GetColRef(_T("LogWarningColor"), m_crLogWarning);
	m_crLogSuccess = ini.GetColRef(_T("LogSuccessColor"), m_crLogSuccess);

	if (statsAverageMinutes < 1)
		statsAverageMinutes = 5;

	// ZZ:UploadSpeedSense -->
    m_bDynUpEnabled = ini.GetBool(_T("USSEnabled"), false);
    m_bDynUpUseMillisecondPingTolerance = ini.GetBool(_T("USSUseMillisecondPingTolerance"), false);
    m_iDynUpPingTolerance = ini.GetInt(_T("USSPingTolerance"), 500);
	m_iDynUpPingToleranceMilliseconds = ini.GetInt(_T("USSPingToleranceMilliseconds"), 200);
	if( minupload < 1 )
		minupload = 1;
	m_iDynUpGoingUpDivider = ini.GetInt(_T("USSGoingUpDivider"), 1000);
    m_iDynUpGoingDownDivider = ini.GetInt(_T("USSGoingDownDivider"), 1000);
    m_iDynUpNumberOfPings = ini.GetInt(_T("USSNumberOfPings"), 1);
	// ZZ:UploadSpeedSense <--

    m_bA4AFSaveCpu = ini.GetBool(_T("A4AFSaveCpu"), false); // ZZ:DownloadManager

	m_bRunAsUser = ini.GetBool(_T("RunAsUnprivilegedUser"), false);
	m_bPreferRestrictedOverUser = ini.GetBool(_T("PreferRestrictedOverUser"), false);
	m_bOpenPortsOnStartUp = ini.GetBool(_T("OpenPortsOnStartUp"), false);
	m_byLogLevel = ini.GetInt(_T("DebugLogLevel"), DLP_VERYLOW);
	m_bTrustEveryHash = ini.GetBool(_T("AICHTrustEveryHash"), false);
	m_bRememberCancelledFiles = ini.GetBool(_T("RememberCancelledFiles"), true);
	m_bRememberDownloadedFiles = ini.GetBool(_T("RememberDownloadedFiles"), true);

	m_bNotifierSendMail = ini.GetBool(_T("NotifierSendMail"), false);
	m_strNotifierMailSender = ini.GetString(_T("NotifierMailSender"), _T(""));
	m_strNotifierMailServer = ini.GetString(_T("NotifierMailServer"), _T(""));
	m_strNotifierMailReceiver = ini.GetString(_T("NotifierMailRecipient"), _T(""));

	m_bWinaTransToolbar = ini.GetBool(_T("WinaTransToolbar"), false);

	///////////////////////////////////////////////////////////////////////////
	// Section: "Proxy"
	//
	proxy.EnablePassword = ini.GetBool(_T("ProxyEnablePassword"),false,_T("Proxy"));
	proxy.UseProxy = ini.GetBool(_T("ProxyEnableProxy"),false,_T("Proxy"));
	_sntprintf(proxy.name, ARRSIZE(proxy.name), _T("%s"), ini.GetString(_T("ProxyName"), _T(""), _T("Proxy")));
	_snprintf(proxy.password, ARRSIZE(proxy.password), "%s", T2CA(ini.GetString(_T("ProxyPassword"), _T(""), _T("Proxy"))));
	_snprintf(proxy.user, ARRSIZE(proxy.user), "%s", T2CA(ini.GetString(_T("ProxyUser"), _T(""), _T("Proxy"))));
	proxy.port = ini.GetInt(_T("ProxyPort"),1080,_T("Proxy"));
	proxy.type = ini.GetInt(_T("ProxyType"),PROXYTYPE_NOPROXY,_T("Proxy"));
	m_bIsASCWOP = ini.GetBool(_T("ConnectWithoutProxy"),false,_T("Proxy"));
	m_bShowProxyErrors = ini.GetBool(_T("ShowErrors"),false,_T("Proxy"));


	///////////////////////////////////////////////////////////////////////////
	// Section: "Statistics"
	//
	statsSaveInterval = ini.GetInt(_T("SaveInterval"), 60, _T("Statistics"));
	statsConnectionsGraphRatio = ini.GetInt(_T("statsConnectionsGraphRatio"), 3, _T("Statistics"));
	_stprintf(statsExpandedTreeItems,_T("%s"),ini.GetString(_T("statsExpandedTreeItems"),_T("111000000100000110000010000011110000010010"),_T("Statistics")));
	CString buffer2;
	for (int i = 0; i < ARRSIZE(m_adwStatsColors); i++) {
		buffer2.Format(_T("StatColor%i"), i);
		_stprintf(buffer, _T("%s"), ini.GetString(buffer2, _T(""), _T("Statistics")));
		m_adwStatsColors[i] = 0;
		if (_stscanf(buffer, _T("%i"), &m_adwStatsColors[i]) != 1)
			ResetStatsColor(i);
	}
	m_bShowVerticalHourMarkers = ini.GetBool(_T("ShowVerticalHourMarkers"), true, _T("Statistics"));

	// -khaos--+++> Load Stats
	// I changed this to a seperate function because it is now also used
	// to load the stats backup and to load stats from preferences.ini.old.
	LoadStats();
	// <-----khaos-

	///////////////////////////////////////////////////////////////////////////
	// Section: "WebServer"
	//
	_stprintf(m_sWebPassword,_T("%s"),ini.GetString(_T("Password"), _T(""),_T("WebServer")));
	_stprintf(m_sWebLowPassword,_T("%s"),ini.GetString(_T("PasswordLow"), _T("")));
	m_nWebPort=ini.GetInt(_T("Port"), 4711);
	m_bWebEnabled=ini.GetBool(_T("Enabled"), false);
	m_bWebUseGzip=ini.GetBool(_T("UseGzip"), true);
	m_bWebLowEnabled=ini.GetBool(_T("UseLowRightsUser"), false);
	m_nWebPageRefresh=ini.GetInt(_T("PageRefreshTime"), 120);
	m_iWebTimeoutMins=ini.GetInt(_T("WebTimeoutMins"), 5 );
	m_iWebFileUploadSizeLimitMB=ini.GetInt(_T("MaxFileUploadSizeMB"), 5 );
	m_bAllowAdminHiLevFunc=ini.GetBool(_T("AllowAdminHiLevelFunc"), false);
	buffer2 = ini.GetString(_T("AllowedIPs"));
	int iPos = 0;
	CString strIP = buffer2.Tokenize(_T(";"), iPos);
	while (!strIP.IsEmpty())
	{
		u_long nIP = inet_addr(CStringA(strIP));
		if (nIP != INADDR_ANY && nIP != INADDR_NONE)
			m_aAllowedRemoteAccessIPs.Add(nIP);
		strIP = buffer2.Tokenize(_T(";"), iPos);
	}

	///////////////////////////////////////////////////////////////////////////
	// Section: "MobileMule"
	//
	_stprintf(m_sMMPassword,_T("%s"),ini.GetString(_T("Password"), _T(""),_T("MobileMule")));
	m_bMMEnabled = ini.GetBool(_T("Enabled"), false);
	m_nMMPort = ini.GetInt(_T("Port"), 80);

	///////////////////////////////////////////////////////////////////////////
	// Section: "PeerCache"
	//
	m_uPeerCacheLastSearch = ini.GetInt(_T("LastSearch"), 0, _T("PeerCache"));
	m_bPeerCacheWasFound = ini.GetBool(_T("Found"), false);
	m_bPeerCacheEnabled = ini.GetBool(_T("Enabled"), true);
	m_nPeerCachePort = ini.GetInt(_T("PCPort"), 0);
	m_bPeerCacheShow = ini.GetBool(_T("Show"), false);

	LoadCats();
	SetLanguage();
}


WORD CPreferences::GetWindowsVersion(){
	static bool bWinVerAlreadyDetected = false;
	if(!bWinVerAlreadyDetected)
	{	
		bWinVerAlreadyDetected = true;
		m_wWinVer = DetectWinVersion();	
	}	
	return m_wWinVer;
}

uint16 CPreferences::GetDefaultMaxConperFive(){
	switch (GetWindowsVersion()){
		case _WINVER_98_:
			return 5;
		case _WINVER_95_:	
		case _WINVER_ME_:
			return MAXCON5WIN9X;
		case _WINVER_2K_:
		case _WINVER_XP_:
			return MAXCONPER5SEC;
		default:
			return MAXCONPER5SEC;
	}
}

//////////////////////////////////////////////////////////
// category implementations
//////////////////////////////////////////////////////////

void CPreferences::SaveCats(){

	// Cats
	CString catinif,ixStr,buffer;
	catinif.Format(_T("%sCategory.ini"),configdir);
	_tremove(catinif);

	CIni catini( catinif, _T("Category") );
	catini.WriteInt(_T("Count"),catMap.GetCount()-1,_T("General"));
	for (int ix=0;ix<catMap.GetCount();ix++){
		ixStr.Format(_T("Cat#%i"),ix);
		catini.WriteString(_T("Title"),catMap.GetAt(ix)->title,ixStr);
		catini.WriteString(_T("Incoming"),catMap.GetAt(ix)->incomingpath,ixStr);
		catini.WriteString(_T("Comment"),catMap.GetAt(ix)->comment,ixStr);
		catini.WriteString(_T("RegularExpression"),catMap.GetAt(ix)->regexp,ixStr);
		buffer.Format(_T("%lu"),catMap.GetAt(ix)->color);
		catini.WriteString(_T("Color"),buffer,ixStr);
		catini.WriteInt(_T("a4afPriority"),catMap.GetAt(ix)->prio,ixStr); // ZZ:DownloadManager
		catini.WriteString(_T("AutoCat"),catMap.GetAt(ix)->autocat,ixStr); 
		catini.WriteInt(_T("Filter"),catMap.GetAt(ix)->filter,ixStr); 
		catini.WriteBool(_T("FilterNegator"),catMap.GetAt(ix)->filterNeg,ixStr);
		catini.WriteBool(_T("AutoCatAsRegularExpression"),catMap.GetAt(ix)->ac_regexpeval,ixStr);
        catini.WriteBool(_T("downloadInAlphabeticalOrder"), catMap.GetAt(ix)->downloadInAlphabeticalOrder!=FALSE, ixStr);
		catini.WriteBool(_T("Care4All"),catMap.GetAt(ix)->care4all,ixStr);
	}
}

void CPreferences::LoadCats() {
	CString ixStr,catinif,cat_a,cat_b,cat_c;
	TCHAR buffer[100];

	catinif.Format(_T("%sCategory.ini"),configdir);

	CIni catini( catinif, _T("Category") );
	int max=catini.GetInt(_T("Count"),0,_T("General"));

	for (int ix=0;ix<=max;ix++){
		ixStr.Format(_T("Cat#%i"),ix);

		Category_Struct* newcat=new Category_Struct;
		newcat->filter=0;
		_stprintf(newcat->title,_T("%s"),catini.GetString(_T("Title"),_T(""),ixStr));
		_stprintf(newcat->incomingpath,_T("%s"),catini.GetString(_T("Incoming"),_T(""),ixStr));
		MakeFoldername(newcat->incomingpath);
		if (!IsShareableDirectory(newcat->incomingpath)){
			_sntprintf(newcat->incomingpath, ARRSIZE(newcat->incomingpath), _T("%s"), GetIncomingDir());
			MakeFoldername(newcat->incomingpath);
		}
		_stprintf(newcat->comment,_T("%s"),catini.GetString(_T("Comment"),_T(""),ixStr));
		newcat->prio =catini.GetInt(_T("a4afPriority"),PR_NORMAL,ixStr); // ZZ:DownloadManager
		newcat->filter=catini.GetInt(_T("Filter"),0,ixStr);
		newcat->filterNeg =catini.GetBool(_T("FilterNegator"),FALSE,ixStr);
		newcat->ac_regexpeval  =catini.GetBool(_T("AutoCatAsRegularExpression"),FALSE,ixStr);
		newcat->care4all=catini.GetBool(_T("Care4All"),FALSE,ixStr);

		newcat->regexp=catini.GetString(_T("RegularExpression"),_T(""),ixStr);
		newcat->autocat=catini.GetString(_T("Autocat"),_T(""),ixStr);
        newcat->downloadInAlphabeticalOrder = catini.GetBool(_T("downloadInAlphabeticalOrder"), FALSE, ixStr); // ZZ:DownloadManager

		_stprintf(buffer,_T("%s"),catini.GetString(_T("Color"),_T("0"),ixStr));
		newcat->color = _tstoi(buffer);

		AddCat(newcat);
		if (!PathFileExists(newcat->incomingpath)) 
			::CreateDirectory(newcat->incomingpath,0);
	}
}
void CPreferences::RemoveCat(int index)	{
	if (index>=0 && index<catMap.GetCount()) { 
		Category_Struct* delcat;
		delcat=catMap.GetAt(index); 
		catMap.RemoveAt(index); 
		delete delcat;
	}
}

bool CPreferences::SetCatFilter(int index, int filter){
	if (index>=0 && index<catMap.GetCount()) { 
		Category_Struct* cat;
		cat=catMap.GetAt(index); 
		cat->filter=filter;
		return true;
	} 
	
	return false;
}

int CPreferences::GetCatFilter(int index){
	if (index>=0 && index<catMap.GetCount()) {
		return catMap.GetAt(index)->filter;
	}
	
    return 0;
}

bool CPreferences::GetCatFilterNeg(int index){
	if (index>=0 && index<catMap.GetCount()) {
		return catMap.GetAt(index)->filterNeg;
	}
	
    return false;
}

void CPreferences::SetCatFilterNeg(int index, bool val) {
	if (index>=0 && index<catMap.GetCount()) {
		catMap.GetAt(index)->filterNeg=val;
	}
}


bool CPreferences::MoveCat(UINT from, UINT to){
	if (from>=(UINT)catMap.GetCount() || to >=(UINT)catMap.GetCount()+1 || from==to) return false;

	Category_Struct* tomove;

	tomove=catMap.GetAt(from);

	if (from < to) {
		catMap.RemoveAt(from);
		catMap.InsertAt(to-1,tomove);
	} else {
		catMap.InsertAt(to,tomove);
		catMap.RemoveAt(from+1);
	}
	
	SaveCats();

	return true;
}


bool CPreferences::IsInstallationDirectory(const CString& rstrDir)
{
	CString strFullPath;
	if (PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;
	
	// skip sharing of several special eMule folders
	if (!CompareDirectories(strFullPath, GetAppDir()))			// ".\eMule"
		return true;
	if (!CompareDirectories(strFullPath, GetConfigDir()))		// ".\eMule\config"
		return true;
	if (!CompareDirectories(strFullPath, GetWebServerDir()))	// ".\eMule\webserver"
		return true;
	if (!CompareDirectories(strFullPath, GetLangDir()))			// ".\eMule\lang"
		return true;

	return false;
}

bool CPreferences::IsShareableDirectory(const CString& rstrDir)
{
	if (IsInstallationDirectory(rstrDir))
		return false;

	CString strFullPath;
	if (PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;
	
	// skip sharing of several special eMule folders
	for (int i=0;i<GetTempDirCount();i++)
		if (!CompareDirectories(strFullPath, GetTempDir(i)))			// ".\eMule\temp"
			return false;

	return true;
}

void CPreferences::UpdateLastVC()
{
	versioncheckLastAutomatic = safe_mktime(CTime::GetCurrentTime().GetLocalTm());
}

void CPreferences::SetWSPass(CString strNewPass)
{
	_stprintf(m_sWebPassword,_T("%s"),MD5Sum(strNewPass).GetHash().GetBuffer(0));
}

void CPreferences::SetWSLowPass(CString strNewPass)
{
	_stprintf(m_sWebLowPassword,_T("%s"),MD5Sum(strNewPass).GetHash().GetBuffer(0));
}

void CPreferences::SetMMPass(CString strNewPass)
{
	_stprintf(m_sMMPassword,_T("%s"),MD5Sum(strNewPass).GetHash().GetBuffer(0));
}

void CPreferences::SetMaxUpload(uint16 in)
{
	maxupload = (in) ? in : UNLIMITED;
}

void CPreferences::SetMaxDownload(uint16 in)
{
	maxdownload = (in) ? in : UNLIMITED;
}

void CPreferences::SetNetworkKademlia(bool val)
{
	networkkademlia = val; 
}

CString CPreferences::GetHomepageBaseURLForLevel(uint8 nLevel){
	CString tmp;
	if (nLevel == 0)
		tmp = _T("http://emule-project.net");
	else if (nLevel == 1)
		tmp = _T("http://www.emule-project.org");
	else if (nLevel == 2)
		tmp = _T("http://www.emule-project.com");
	else if (nLevel < 100)
		tmp.Format(_T("http://www%i.emule-project.net"),nLevel-2);
	else if (nLevel < 150)
		tmp.Format(_T("http://www%i.emule-project.org"),nLevel);
	else if (nLevel < 200)
		tmp.Format(_T("http://www%i.emule-project.com"),nLevel);
	else if (nLevel == 200)
		tmp = _T("http://emule.sf.net");
	else if (nLevel == 201)
		tmp = _T("http://www.emuleproject.net");
	else if (nLevel == 202)
		tmp = _T("http://sourceforge.net/projects/emule/");
	else
		tmp = _T("http://www.emule-project.net");
	return tmp;
}

CString CPreferences::GetVersionCheckBaseURL(){
	CString tmp;
	uint8 nWebMirrorAlertLevel = GetWebMirrorAlertLevel();
	if (nWebMirrorAlertLevel < 100)
		tmp = _T("http://vcheck.emule-project.net");
	else if (nWebMirrorAlertLevel < 150)
		tmp.Format(_T("http://vcheck%i.emule-project.org"),nWebMirrorAlertLevel);
	else if (nWebMirrorAlertLevel < 200)
		tmp.Format(_T("http://vcheck%i.emule-project.com"),nWebMirrorAlertLevel);
	else if (nWebMirrorAlertLevel == 200)
		tmp = _T("http://emule.sf.net");
	else if (nWebMirrorAlertLevel == 201)
		tmp = _T("http://www.emuleproject.net");
	else
		tmp = _T("http://vcheck.emule-project.net");
	return tmp;
}

bool CPreferences::IsDefaultNick(const CString strCheck){
	// not fast, but this function is called often
	for (int i = 0; i != 255; i++){
		if (GetHomepageBaseURLForLevel(i) == strCheck)
			return true;
	}
	return ( strCheck == _T("http://emule-project.net") );
}

void CPreferences::SetUserNick(LPCTSTR pszNick)
{
	strNick = pszNick;
}

uint8 CPreferences::GetWebMirrorAlertLevel(){
	// Known upcoming DDoS Attacks
	if (m_nWebMirrorAlertLevel == 0){
		// no threats known at this time
	}
	// end
	if (UpdateNotify())
		return m_nWebMirrorAlertLevel;
	else
		return 0;
}

bool CPreferences::IsRunAsUserEnabled(){
	return (GetWindowsVersion() == _WINVER_XP_ || GetWindowsVersion() == _WINVER_2K_) && m_bRunAsUser;
}

bool CPreferences::GetUseReBarToolbar()
{
	return GetReBarToolbar() && theApp.m_ullComCtrlVer >= MAKEDLLVERULL(5,8,0,0);
}

int	CPreferences::GetMaxGraphUploadRate(bool bEstimateIfUnlimited){
	if (maxGraphUploadRate != UNLIMITED || !bEstimateIfUnlimited){
		return maxGraphUploadRate;
	}
	else{
		if (maxGraphUploadRateEstimated != 0){
			return maxGraphUploadRateEstimated +4;
		}
		else
			return 16;
	}
}

void CPreferences::EstimateMaxUploadCap(uint32 nCurrentUpload){
	if (maxGraphUploadRateEstimated+1 < nCurrentUpload){
		maxGraphUploadRateEstimated = nCurrentUpload;
		if (maxGraphUploadRate == UNLIMITED && theApp.emuledlg && theApp.emuledlg->statisticswnd)
			theApp.emuledlg->statisticswnd->SetARange(false, thePrefs.GetMaxGraphUploadRate(true));
	}
}

void CPreferences::SetMaxGraphUploadRate(int in){
	maxGraphUploadRate	=(in) ? in : UNLIMITED;
}

bool CPreferences::IsDynUpEnabled()	{
	return m_bDynUpEnabled || maxGraphUploadRate == UNLIMITED;
}