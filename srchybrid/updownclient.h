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
#ifndef __UP_DOWN_CLIENT_H__
#define	__UP_DOWN_CLIENT_H__
#pragma once
#include "loggable.h"
#include "BarShader.h"

class CClientReqSocket;
class CPeerCacheDownSocket;
class CPeerCacheUpSocket;
class CFriend;
class CPartFile;
class CClientCredits;
class CAbstractFile;
class CKnownFile;
class Packet;
class CxImage;
struct Requested_Block_Struct;
class CSafeMemFile;

struct Pending_Block_Struct{
	Requested_Block_Struct*	block;
	struct z_stream_s*      zStream;       // Barry - Used to unzip packets
	uint32                  totalUnzipped; // Barry - This holds the total unzipped bytes for all packets so far
	UINT					fZStreamError : 1,
							fRecovered    : 1;
};

#pragma pack(1)
struct Requested_File_Struct{
	uchar	  fileid[16];
	uint32	  lastasked;
	uint8	  badrequests;
};
#pragma pack()

enum EUploadState{
	US_UPLOADING,
	US_ONUPLOADQUEUE,
	US_WAITCALLBACK,
	US_CONNECTING,
	US_PENDING,
	US_LOWTOLOWIP,
	US_BANNED,
	US_ERROR,
	US_NONE
};

enum EDownloadState{
	DS_DOWNLOADING,
	DS_ONQUEUE,
	DS_CONNECTED,
	DS_CONNECTING,
	DS_WAITCALLBACK,
	DS_REQHASHSET,
	DS_NONEEDEDPARTS,
	DS_TOOMANYCONNS,
	DS_LOWTOLOWIP,
	DS_BANNED,
	DS_ERROR,
	DS_NONE,
	DS_REMOTEQUEUEFULL  // not used yet, except in statistics
};

enum EPeerCacheDownState{
	PCDS_NONE = 0,
	PCDS_WAIT_CLIENT_REPLY,
	PCDS_WAIT_CACHE_REPLY,
	PCDS_DOWNLOADING
};

enum EPeerCacheUpState{
	PCUS_NONE = 0,
	PCUS_WAIT_CACHE_REPLY,
	PCUS_UPLOADING
};

enum EChatState{
	MS_NONE,
	MS_CHATTING,
	MS_CONNECTING,
	MS_UNABLETOCONNECT
};

enum EKadState{
	KS_NONE,
	KS_QUEUED_FWCHECK,
	KS_CONNECTING_FWCHECK,
	KS_CONNECTED_FWCHECK,
	KS_QUEUED_BUDDY,
	KS_CONNECTED_BUDDY
};

enum EClientSoftware{
	SO_EMULE			= 0,	// default
	SO_CDONKEY			= 1,	// ET_COMPATIBLECLIENT
	SO_XMULE			= 2,	// ET_COMPATIBLECLIENT
	SO_AMULE			= 3,	// ET_COMPATIBLECLIENT
	SO_SHAREAZA			= 4,	// ET_COMPATIBLECLIENT
	SO_MLDONKEY			= 10,	// ET_COMPATIBLECLIENT
	SO_LPHANT			= 20,	// ET_COMPATIBLECLIENT
	// other client types which are not identified with ET_COMPATIBLECLIENT
	SO_EDONKEYHYBRID	= 50,
	SO_EDONKEY,
	SO_OLDEMULE,
	SO_URL,
	SO_UNKNOWN
};

enum ESecureIdentState{
	IS_UNAVAILABLE		= 0,
	IS_ALLREQUESTSSEND  = 0,
	IS_SIGNATURENEEDED	= 1,
	IS_KEYANDSIGNEEDED	= 2,
};

enum EInfoPacketState{
	IP_NONE				= 0,
	IP_EDONKEYPROTPACK  = 1,
	IP_EMULEPROTPACK	= 2,
	IP_BOTH				= 3,
};

enum ESourceFrom{
	SF_SERVER			= 0,
	SF_KADEMLIA			= 1,
	SF_SOURCE_EXCHANGE	= 2,
	SF_PASSIVE			= 3,
	SF_LINK				= 4
};

struct PartFileStamp {
	CPartFile*	file;
	DWORD		timestamp;
};

#define	MAKE_CLIENT_VERSION(mjr, min, upd) \
	((UINT)(mjr)*100U*10U*100U + (UINT)(min)*100U*10U + (UINT)(upd)*100U)

//#pragma pack(2)
class CUpDownClient: public CLoggable
					,public CObject
{
	DECLARE_DYNAMIC(CUpDownClient)

	friend class CUploadQueue;
public:
	//base
	CUpDownClient(CClientReqSocket* sender = 0);
	CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid, uint32 in_serverup, uint16 in_serverport, bool ed2kID = false);
	virtual ~CUpDownClient();

	///////////////////////////////////////////////////////////////////////////
	// PeerCache client
	// 
	int m_iHttpSendState;
	uint32 m_uPeerCacheDownloadPushId;
	uint32 m_uPeerCacheUploadPushId;
	CPeerCacheDownSocket* m_pPCDownSocket;
	CPeerCacheUpSocket* m_pPCUpSocket;
	uint32 m_uPeerCacheRemoteIP;
	EPeerCacheDownState m_ePeerCacheDownState;
	EPeerCacheUpState m_ePeerCacheUpState;
	bool m_bPeerCacheDownHit;
	bool m_bPeerCacheUpHit;

	bool IsDownloadingFromPeerCache() const;
	bool IsUploadingToPeerCache() const;
	void SetPeerCacheDownState(EPeerCacheDownState eState);
	void SetPeerCacheUpState(EPeerCacheUpState eState);

	bool SendPeerCacheFileRequest();
	bool ProcessPeerCacheQuery(const char* packet, UINT size);
	bool ProcessPeerCacheAnswer(const char* packet, UINT size);
	bool ProcessPeerCacheAcknowledge(const char* packet, UINT size);
	void OnPeerCacheDownSocketClosed(int nErrorCode);
	bool OnPeerCacheDownSocketTimeout();
	
	bool ProcessPeerCacheDownHttpResponse(const CStringAArray& astrHeaders);
	bool ProcessPeerCacheDownHttpResponseBody(const BYTE* pucData, UINT uSize);
	bool ProcessPeerCacheUpHttpResponse(const CStringAArray& astrHeaders);
	UINT ProcessPeerCacheUpHttpRequest(const CStringAArray& astrHeaders);

	virtual bool ProcessHttpDownResponse(const CStringAArray& astrHeaders);
	virtual bool ProcessHttpDownResponseBody(const BYTE* pucData, UINT uSize);

	void StartDownload();
	virtual void SendCancelTransfer(Packet* packet = NULL);
	virtual void CheckDownloadTimeout();

	virtual bool	IsEd2kClient() const { return true; }
	virtual bool	Disconnected(LPCTSTR pszReason, bool bFromSocket = false);
	virtual bool	TryToConnect(bool bIgnoreMaxCon = false, CRuntimeClass* pClassSocket = NULL);
	virtual bool	Connect();
	virtual void	ConnectionEstablished();
	virtual void	OnSocketConnected(int nErrorCode);
	bool			CheckHandshakeFinished(UINT protocol, UINT opcode) const;
	void			CheckFailedFileIdReqs(const uchar* aucFileHash);
	uint32			GetUserIDHybrid() const
					{
						return m_nUserIDHybrid;
					}
	void			SetUserIDHybrid(uint32 val)	
					{
						m_nUserIDHybrid = val;
					}
	LPCTSTR			GetUserName() const			
					{
						return m_pszUsername;
					}
	void			SetUserName(LPCTSTR pszNewName);
	uint32			GetIP() const
					{
						return m_dwUserIP;
					}
	bool			HasLowID() const;
	uint32			GetConnectIP() const				{return m_nConnectIP;}
	uint16			GetUserPort() const
					{
						return m_nUserPort;
					}
	void			SetUserPort(uint16 val)
					{
						m_nUserPort = val;
					}

	uint32			GetTransferedUp() const
					{
						return m_nTransferedUp;
					}
	uint32			GetTransferedDown() const
					{
						return m_nTransferedDown;
					}
	uint32			GetServerIP() const
					{
						return m_dwServerIP;
					}
	void			SetServerIP(uint32 nIP)
					{
						m_dwServerIP = nIP;
					}
	uint16			GetServerPort() const
					{
						return m_nServerPort;
					}
	void			SetServerPort(uint16 nPort)
					{
						m_nServerPort = nPort;
					}
	const uchar*	GetUserHash() const
					{
						return (uchar*)m_achUserHash;
					}
	void			SetUserHash(const uchar* m_achTempUserHash);
	bool			HasValidHash() const
					{
						return ((const int*)m_achUserHash[0]) != 0 || ((const int*)m_achUserHash[1]) != 0 || ((const int*)m_achUserHash[2]) != 0 || ((const int*)m_achUserHash[3]) != 0; 
					}
	int				GetHashType() const;
	EClientSoftware	GetClientSoft() const
					{
						return (EClientSoftware)m_clientSoft;
					}
	const CString&	GetClientSoftVer() const
					{
						return m_strClientSoftware;
					}
	const CString&	GetClientModVer() const
					{
						return m_strModVersion;
					}
	void			ReGetClientSoft();
	uint32			GetVersion() const
					{
						return m_nClientVersion;
					}
	uint8			GetMuleVersion() const
					{
						return m_byEmuleVersion;
					}
	bool			ExtProtocolAvailable() const
					{
						return m_bEmuleProtocol;
					}
	bool			SupportMultiPacket()const
					{
						return m_bMultiPacket;
					}
	bool			SupportPeerCache() const { return m_fPeerCache; }
	bool			IsEmuleClient() const
					{
						return m_byEmuleVersion;
					}
	uint8			GetSourceExchangeVersion() const
					{
						return m_bySourceExchangeVer;
					}
	CClientCredits* Credits() const
					{
						return credits;
					}
	bool			IsBanned() const;
	const CString&	GetClientFilename() const
					{
						return m_strClientFilename;
					}
	void			SetClientFilename( CString fileName )
					{
						m_strClientFilename = fileName;
					}
	uint16			GetUDPPort() const
					{
						return m_nUDPPort;
					}
	void			SetUDPPort(uint16 nPort)	
					{
						m_nUDPPort = nPort;
					}
	uint8			GetUDPVersion() const		
					{
						return m_byUDPVer;
					}
	bool			SupportsUDP() const			
					{
						return GetUDPVersion() != 0 && m_nUDPPort != 0;
					}
	uint16			GetKadPort() const
					{
						return m_nKadPort;
					}
	void			SetKadPort(uint16 nPort)
					{
						m_nKadPort = nPort;
					}
	uint8			GetExtendedRequestsVersion() const 
					{
						return m_byExtendedRequestsVer;
					}
	void			RequestSharedFileList();
	void			ProcessSharedFileList(char* pachPacket, uint32 nSize, LPCTSTR pszDirectory = NULL);
	void			ClearHelloProperties();
	bool			ProcessHelloAnswer(char* pachPacket, uint32 nSize);
	bool			ProcessHelloPacket(char* pachPacket, uint32 nSize);
	void			SendHelloAnswer();
	virtual bool	SendHelloPacket();
	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(char* pachPacket, uint32 nSize);
	void			ProcessMuleCommentPacket(char* pachPacket, uint32 nSize);
	void			ProcessEmuleQueueRank(char* packet, UINT size);
	void			ProcessEdonkeyQueueRank(char* packet, UINT size);
	void			CheckQueueRankFlood();
	bool			Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash = false) const;
	void			ResetFileStatusInfo();
	uint32			GetLastSrcReqTime() const	
					{
						return m_dwLastSourceRequest;
					}
	void			SetLastSrcReqTime()			
					{
						m_dwLastSourceRequest = ::GetTickCount();
					}
	uint32			GetLastSrcAnswerTime() const
					{
						return m_dwLastSourceAnswer;
					}
	void			SetLastSrcAnswerTime()
					{
						m_dwLastSourceAnswer = ::GetTickCount();
					}
	uint32			GetLastAskedForSources() const 
					{
						return m_dwLastAskedForSources;
					}
	void			SetLastAskedForSources()
					{
						m_dwLastAskedForSources = ::GetTickCount();
					}
	bool			GetFriendSlot() const;
	void			SetFriendSlot(bool bNV)		
					{
						m_bFriendSlot = bNV;
					}
	bool			IsFriend() const
					{
						return m_Friend != NULL;
					}
	void			SetCommentDirty(bool bDirty = true)
					{
						m_bCommentDirty = bDirty;
					}
	bool			GetSentCancelTransfer() const 
					{
						return m_fSentCancelTransfer;
					}
	void			SetSentCancelTransfer(bool bVal)
					{
						m_fSentCancelTransfer = bVal;
					}
	void			ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize);
	void			SendPublicIPRequest();
	// secure ident
	void			SendPublicKeyPacket();
	void			SendSignaturePacket();
	void			ProcessPublicKeyPacket(uchar* pachPacket, uint32 nSize);
	void			ProcessSignaturePacket(uchar* pachPacket, uint32 nSize);
	uint8			GetSecureIdentState() const
					{
						return m_SecureIdentState;
					}
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(uchar* pachPacket, uint32 nSize);
	uint8			GetInfoPacketsReceived() const
					{
						return m_byInfopacketsReceived;
					}
	void			InfoPacketsReceived();
	// preview
	void			SendPreviewRequest(const CAbstractFile* pForFile);
	void			SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount);
	void			ProcessPreviewReq(char* pachPacket, uint32 nSize);
	void			ProcessPreviewAnswer(char* pachPacket, uint32 nSize);
	bool			GetPreviewSupport() const
					{
						return m_fSupportsPreview && GetViewSharedFilesSupport();
					}
	bool			GetViewSharedFilesSupport() const
					{
						return m_fNoViewSharedFiles==0;
					}
	bool			SafeSendPacket(Packet* packet);
	void			CheckForGPLEvilDoer();
	//upload
	EUploadState	GetUploadState() const
					{
						return (EUploadState)m_nUploadState;
					}
	void			SetUploadState(EUploadState news);
	uint32			GetWaitStartTime() const;
	void 			SetWaitStartTime();
	void 			ClearWaitStartTime();
	uint32			GetWaitTime() const
					{
						return m_dwUploadTime - GetWaitStartTime();
					}
	bool			IsDownloading() const
					{
						return (m_nUploadState == US_UPLOADING);
					}
	bool			HasBlocks() const
					{
						return !m_BlockRequests_queue.IsEmpty();
					}
	uint32			GetDatarate() const
					{
						return m_nUpDatarate;
					}	
	uint32			GetScore(bool sysvalue, bool isdownloading = false, bool onlybasevalue = false) const;
	void			AddReqBlock(Requested_Block_Struct* reqblock);
	void			CreateNextBlockPackage();
	uint32			GetUpStartTimeDelay() const
					{
						return ::GetTickCount() - m_dwUploadTime;
					}
	void 			SetUpStartTime()
					{
						m_dwUploadTime = ::GetTickCount();
					}
	void			SendHashsetPacket(char* forfileid);
	const uchar*	GetUploadFileID() const	
					{
						return requpfileid;
					}
	void			SetUploadFileID(CKnownFile* newreqfile);
	uint32			SendBlockData();
	void			ClearUploadBlockRequests();
	void			SendRankingInfo();
	void			SendCommentInfo(/*const*/ CKnownFile *file);
	void			AddRequestCount(const uchar* fileid);
	void			UnBan();
	void			Ban(LPCTSTR pszReason = NULL);
	uint32			GetAskedCount() const
					{
						return m_cAsked;
					}
	void			AddAskedCount()
					{
						m_cAsked++;
					}
	void			SetAskedCount(uint32 m_cInAsked)
					{
						m_cAsked = m_cInAsked;
					}
	void			FlushSendBlocks(); // call this when you stop upload, or the socket might be not able to send
	uint32			GetLastUpRequest() const	
					{
						return m_dwLastUpRequest;
					}
	void			SetLastUpRequest()
					{
						m_dwLastUpRequest = ::GetTickCount();
					}
	uint32			GetSessionUp() const
					{
						return m_nTransferedUp - m_nCurSessionUp;
					}
	void			ResetSessionUp()
					{
						m_nCurSessionUp = m_nTransferedUp; 
						m_addedPayloadQueueSession = 0; 
						m_nCurQueueSessionPayloadUp = 0;
					} 
	uint32			GetQueueSessionPayloadUp() const 
					{
						return m_nCurQueueSessionPayloadUp;
					}
	void			ProcessExtendedInfo(CSafeMemFile* packet, CKnownFile* tempreqfile);
	uint16			GetUpPartCount() const
					{
						return m_nUpPartCount;
					}
	void			DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const;
    uint32          GetPayloadInBuffer() const
					{
						return m_addedPayloadQueueSession - GetQueueSessionPayloadUp();
					}
	bool			IsUpPartAvailable(uint16 iPart) const
					{
						return ( (iPart >= m_nUpPartCount) || (!m_abyUpPartStatus) )? 0:m_abyUpPartStatus[iPart];
					}
	uint8*			GetUpPartStatus() const
					{
						return m_abyUpPartStatus;
					}

	//download
	uint32			GetAskedCountDown() const	
					{
						return m_cDownAsked;
					}
	void			AddAskedCountDown()
					{
						m_cDownAsked++;
					}
	void			SetAskedCountDown(uint32 m_cInDownAsked)
					{
						m_cDownAsked = m_cInDownAsked;
					}
	EDownloadState	GetDownloadState() const
					{
						return (EDownloadState)m_nDownloadState;
					}
	void			SetDownloadState(EDownloadState nNewState);
// ZZ:DownloadManager -->
	uint32			GetLastAskedTime(const CPartFile* partFile = NULL) const
					{
                        return m_dwLastAskedTime;
					}
    void            SetLastAskedTime() {
                        m_dwLastAskedTime = ::GetTickCount();
                    }
// <-- ZZ:DownloadManager

	bool			IsPartAvailable(uint16 iPart) const
					{
						return ( (iPart >= m_nPartCount) || (!m_abyPartStatus) )? 0:m_abyPartStatus[iPart];
					}
	uint8*			GetPartStatus() const
					{
						return m_abyPartStatus;
					}
	uint16			GetPartCount() const
					{
						return m_nPartCount;
					}
	uint32			GetDownloadDatarate() const
					{
						return m_nDownDatarate;
					}
	uint16			GetRemoteQueueRank() const
					{
						return m_nRemoteQueueRank;
					}
	void			SetRemoteQueueRank(uint16 nr);
	bool			IsRemoteQueueFull() const
					{
						return m_bRemoteQueueFull;
					}
	void			SetRemoteQueueFull(bool flag)
					{
						m_bRemoteQueueFull = flag;
					}
	void			DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const;
	bool			AskForDownload();
	virtual void	SendFileRequest();
	void			SendStartupLoadReq();
	void			ProcessFileInfo(CSafeMemFile* data, CPartFile* file);
	void			ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file);
	void			ProcessHashSet(char* data, uint32 size);
	void			ProcessAcceptUpload();
	bool			AddRequestForAnotherFile(CPartFile* file);
	void			CreateBlockRequests(int iMaxBlocks);
	virtual void	SendBlockRequests();
	virtual bool	SendHttpBlockRequests();
	virtual void	ProcessBlockPacket(char* packet, uint32 size, bool packed = false);
	virtual void	ProcessHttpBlockPacket(const BYTE* pucData, UINT uSize);
	void			ClearDownloadBlockRequests();
	uint32			CalculateDownloadRate();
	UINT			GetAvailablePartCount() const;
	bool			SwapToAnotherFile(LPCTSTR pszReason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile = NULL, bool allowSame = true, bool isAboutToAsk = false, bool debug = false); // ZZ:DownloadManager
	void			DontSwapTo(/*const*/ CPartFile* file);
	bool			IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime = false, const bool fileIsNNP = false) /*const*/; // ZZ:DownloadManager
    uint32          GetTimeUntilReask() const;
    uint32          GetTimeUntilReask(const CPartFile* file) const;
	void			UDPReaskACK(uint16 nNewQR);
	void			UDPReaskFNF();
	void			UDPReaskForDownload();
	bool			IsSourceRequestAllowed() const;
    bool            IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck = false) const; // ZZ:DownloadManager

	bool			IsValidSource() const;
	ESourceFrom		GetSourceFrom() const
					{
						return (ESourceFrom)m_nSourceFrom;
					}
	void			SetSourceFrom(ESourceFrom val) {m_nSourceFrom = val;}
	// -khaos--+++> Download Sessions Stuff
	void			SetDownStartTime()
					{
						m_dwDownStartTime = ::GetTickCount();
					}
	uint32			GetDownTimeDifference()
					{
						uint32 myTime = m_dwDownStartTime;
						m_dwDownStartTime = 0;
						return ::GetTickCount() - myTime;
					}
	bool			GetTransferredDownMini() const
					{
						return m_bTransferredDownMini;
					}
	void			SetTransferredDownMini()
					{
						m_bTransferredDownMini = true;
					}
	void			InitTransferredDownMini()
					{
						m_bTransferredDownMini = false;
					}
	//				A4AF Stats Stuff:
	//				In CPartFile::Process, I am going to keep a tally of how many clients
	//				in that PF's source list are A4AF for other files.  This tally is worthless
	//				to the PartFile that it belongs to, but when we add all of these counts up for
	//				each PartFile, we will get an accurate count of how many A4AF requests there are
	//				total.  This is for the Found Sources section of the tree.  This is a better, faster
	//				option than looping through the lists for unavailable sources.
	uint16			GetA4AFCount() const
					{
						return m_OtherRequests_list.GetCount();
					}
	// <-----khaos-
	uint16			GetUpCompleteSourcesCount() const
					{
						return m_nUpCompleteSourcesCount;
					}
	void			SetUpCompleteSourcesCount(uint16 n)
					{
						m_nUpCompleteSourcesCount = n;
					}
	//chat
	EChatState		GetChatState() const
					{
						return (EChatState)m_nChatstate;
					}
	void			SetChatState(EChatState nNewS)
					{
						m_nChatstate = nNewS;
					}
	//KadIPCheck
	EKadState GetKadState() const
					{
						return (EKadState)m_nKadState;
					}
	void			SetKadState(EKadState nNewS)
					{
						m_nKadState = nNewS;
					}
	//File Comment 
    const CString&	GetFileComment() const
					{
						return m_strComment;
					} 
    void			SetFileComment(LPCSTR desc)
					{
						m_strComment = desc;
					}
    uint8			GetFileRate() const
					{
						return m_iRate;
					}
    void			SetFileRate(uint8 iNewRate)
					{
						m_iRate = iNewRate;
					}
	// Barry - Process zip file as it arrives, don't need to wait until end of block
	int				unzip(Pending_Block_Struct* block, BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion = 0);
	void			UpdateDisplayedInfo(bool force = false);
	int             GetFileListRequested() const
					{
						return m_iFileListRequested;
					}
    void            SetFileListRequested(int iFileListRequested)
					{
						m_iFileListRequested = iFileListRequested;
					}
	// message filtering
	uint8			GetMessagesReceived() const
					{	// count of chatmessages he sent to me
						return m_cMessagesReceived;
					}
	void			SetMessagesReceived(uint8 nCount)
					{
						m_cMessagesReceived = nCount;
					}
	void			IncMessagesReceived()
					{
						m_cMessagesReceived++;
					}
	uint8			GetMessagesSent() const
					{	// count of chatmessages I sent to him
						return m_cMessagesSent;
					}
	void			SetMessagesSent(uint8 nCount)
					{
						m_cMessagesSent = nCount;
					}
	void			IncMessagesSent()
					{
						m_cMessagesSent++;
					}
	bool			IsSpammer() const
					{
						return m_fIsSpammer;
					}
	void			SetSpammer(bool bVal)
					{
						m_fIsSpammer = bVal ? 1 : 0;
					}
	bool			GetMessageFiltered() const
					{
						return m_fMessageFiltered;
					}
	void			SetMessageFiltered(bool bVal)
					{
						m_fMessageFiltered = bVal ? 1 : 0;
					}
	virtual void	SetRequestFile(CPartFile* pReqFile);

	CString			GetDownloadStateDisplayString() const;
	CString			GetUploadStateDisplayString() const;

	LPCTSTR			DbgGetDownloadState() const;
	LPCTSTR			DbgGetUploadState() const;
	CString			DbgGetClientInfo(bool bFormatIP = false) const;
	CString			DbgGetFullClientSoftVer() const;
	const CString&	DbgGetHelloInfo() const { return m_strHelloInfo; }
	const CString&	DbgGetMuleInfo() const { return m_strMuleInfo; }

// ZZ:DownloadManager -->
    const bool      IsInOtherRequestList(const CPartFile* fileToCheck) const;

    const bool      SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool isNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping = false, bool debug = false);
// <-- ZZ:DownloadManager
#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CClientReqSocket* socket;
	CClientCredits*	credits;
	CFriend*		m_Friend;
	CPartFile*		reqfile;
	uint8*			m_abyUpPartStatus;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherRequests_list;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherNoNeeded_list;
	uint16			m_lastPartAsked;
	bool			m_bAddNextConnect;  // VQB Fix for LowID slots only on connection

protected:
	// base
	void	Init();
	bool	ProcessHelloTypePacket(CSafeMemFile* data);
	void	SendHelloTypePacket(CSafeMemFile* data);
	void	CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	CreatePackedPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF = true);

	uint32	m_nConnectIP;		// holds the supposed IP or (after we had a connection) the real IP
	uint32	m_dwUserIP;			// holds 0 (real IP not yet available) or the real IP (after we had a connection)
	uint32	m_dwServerIP;
	uint32	m_nUserIDHybrid;
	uint16	m_nUserPort;
	uint16	m_nServerPort;
	uint32	m_nClientVersion;
	uint32	m_nUpDatarate;
	uint32	dataratems;
	//--group to aligned int32
	uint8	m_byEmuleVersion;
	uint8	m_byDataCompVer;
	bool	m_bEmuleProtocol;
	bool	m_bIsHybrid;
	//--group to aligned int32
	TCHAR*	m_pszUsername;
	uchar	m_achUserHash[16];
	uint16	m_nUDPPort;
	uint16	m_nKadPort;
	//--group to aligned int32
	uint8	m_byUDPVer;
	uint8	m_bySourceExchangeVer;
	uint8	m_byAcceptCommentVer;
	uint8	m_byExtendedRequestsVer;
	//--group to aligned int32
	uint8	m_byCompatibleClient;
	bool	m_bFriendSlot;
	bool	m_bCommentDirty;
	bool	m_bIsML;
	//--group to aligned int32
	bool	m_bGPLEvildoer;
	bool	m_bHelloAnswerPending;
	uint8	m_byInfopacketsReceived;	// have we received the edonkeyprot and emuleprot packet already (see InfoPacketsReceived() )
	uint8	m_bySupportSecIdent;
	//--group to aligned int32
	uint32	m_dwLastSignatureIP;
	CString m_strClientSoftware;
	CString m_strModVersion;
	uint32	m_dwLastSourceRequest;
	uint32	m_dwLastSourceAnswer;
	uint32	m_dwLastAskedForSources;
    int     m_iFileListRequested;
	CString	m_strComment;
	//--group to aligned int32
	uint8	m_iRate;
	uint8	m_cMessagesReceived;		// count of chatmessages he sent to me
	uint8	m_cMessagesSent;			// count of chatmessages I sent to him
	bool	m_bMultiPacket;

	// states
#ifdef _DEBUG
	// use the 'Enums' only for debug builds, each enum costs 4 bytes (3 unused)
	EClientSoftware		m_clientSoft;
	EChatState			m_nChatstate;
	EKadState			m_nKadState;
	ESecureIdentState	m_SecureIdentState;
	EUploadState		m_nUploadState;
	EDownloadState		m_nDownloadState;
	ESourceFrom			m_nSourceFrom;
#else
	uint8 m_clientSoft;
	uint8 m_nChatstate;
	uint8 m_nKadState;
	uint8 m_SecureIdentState;
	uint8 m_nUploadState;
	uint8 m_nDownloadState;
	uint8 m_nSourceFrom;
#endif

	CTypedPtrList<CPtrList, Packet*> m_WaitingPackets_list;
	CList<PartFileStamp, PartFileStamp> m_DontSwap_list;
	DWORD	m_lastRefreshedDLDisplay;
    DWORD	m_lastRefreshedULDisplay;

	//upload
	uint32		m_nTransferedUp;
	uint32		m_dwUploadTime;
	uint32		m_cAsked;
	uint32		m_dwLastUpRequest;
	uint32		m_nCurSessionUp;
    uint32      m_nCurQueueSessionPayloadUp;
    uint32      m_addedPayloadQueueSession;
	uint16		m_nUpPartCount;
	uint16		m_nUpCompleteSourcesCount;
	static CBarShader s_UpStatusBar;
	uchar		requpfileid[16];

	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData,TransferredData>			 m_AvarageUDR_list; // By BadWolf
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_BlockRequests_queue;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DoneBlocks_list;
	CTypedPtrList<CPtrList, Requested_File_Struct*>	 m_RequestedFiles_list;
	//download
	uint32		m_cDownAsked;
	uint8*		m_abyPartStatus;
	uint32		m_dwLastAskedTime;
	CString		m_strClientFilename;
	uint32		m_nTransferedDown;
	// -khaos--+++> Download Session Stats
	uint32		m_dwDownStartTime;
	// <-----khaos-
	uint32      m_nLastBlockOffset;   // Patch for show parts that you download [Cax2]
	uint32		m_nDownDatarate;
	uint32		m_nDownDataRateMS;
	uint32		m_nSumForAvgDownDataRate;
	uint32		m_dwLastBlockReceived;
	uint32		m_nTotalUDPPackets;
	uint32		m_nFailedUDPPackets;
	//--group to aligned int32
	uint16		m_cShowDR;
	uint16		m_nRemoteQueueRank;
	//--group to aligned int32
	uint16		m_nPartCount;
	bool		m_bRemoteQueueFull;
	bool		m_bCompleteSource;
	//--group to aligned int32
	bool		m_bReaskPending;
	bool		m_bUDPPending;
	bool		m_bTransferredDownMini;
	static CBarShader s_StatusBar;

	// using bitfield for less important flags, to save some bytes
	UINT m_fHashsetRequesting : 1, // we have sent a hashset request to this client in the current connection
		 m_fSharedDirectories : 1, // client supports OP_ASKSHAREDIRS opcodes
		 m_fSentCancelTransfer: 1, // we have sent an OP_CANCELTRANSFER in the current connection
		 m_fNoViewSharedFiles : 1, // client has disabled the 'View Shared Files' feature, if this flag is not set, we just know that we don't know for sure if it is enabled
		 m_fSupportsPreview   : 1,
		 m_fPreviewReqPending : 1,
		 m_fPreviewAnsPending : 1,
		 m_fIsSpammer		  : 1,
		 m_fMessageFiltered   : 1,
		 m_fPeerCache		  : 1,
		 m_fQueueRankPending  : 1,
		 m_fUnaskQueueRankRecv: 2,
		 m_fFailedFileIdReqs  : 4, // nr. of failed file-id related requests per connection
		 m_fNeedOurPublicIP	  : 1; // we requested our IP from this client

	// By BadWolf - Accurate Speed Measurement (Ottavio84 idea)
	CList<TransferredData,TransferredData>			 m_AvarageDDR_list;
	uint32	sumavgUDR;
	// END By BadWolf - Accurate Speed Measurement (Ottavio84 idea)

	CTypedPtrList<CPtrList, Pending_Block_Struct*>	 m_PendingBlocks_list;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DownloadBlocks_list;

	CString m_strHelloInfo;
	CString m_strMuleInfo;

	CStringA m_strUrlPath;
	UINT m_uReqStart;
	UINT m_uReqEnd;
	UINT m_nUrlStartPos;

    bool    m_bSourceExchangeSwapped; // ZZ:DownloadManager
    bool    DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason); // ZZ:DownloadManager
    uint32  GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime) const;

};
//#pragma pack()

#endif//__UP_DOWN_CLIENT_H__