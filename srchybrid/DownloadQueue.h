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
#pragma once
#include "Loggable.h"

class CSafeMemFile;
class CSearchFile;
class CUpDownClient;
class CServer;
class CPartFile;
class CSharedFileList;
class CKnownFile;
namespace Kademlia 
{
	class CUInt128;
};

// SLUGFILLER: hostnameSources
#define WM_HOSTNAMERESOLVED		(WM_USER + 0x101)

class CSourceHostnameResolveWnd : public CWnd
{
// Construction
public:
	CSourceHostnameResolveWnd();
	virtual ~CSourceHostnameResolveWnd();

	void AddToResolve(const uchar* fileid, LPCTSTR pszHostname, uint16 port);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHostnameResolved(WPARAM wParam, LPARAM lParam);

private:
	struct Hostname_Entry {
		uchar fileid[16];
		CStringA strHostname;
		uint16 port;
	};
	CTypedPtrList<CPtrList, Hostname_Entry*> m_toresolve;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};
// SLUGFILLER: hostnameSources

class CDownloadQueue: public CLoggable
{
	friend class CAddFileThread;
	friend class CServerSocket;

public:
	CDownloadQueue(CSharedFileList* in_sharedfilelist);
	~CDownloadQueue();

	void	Process();
	void	Init();
	void	AddSearchToDownload(CSearchFile* toadd,uint8 paused=2,uint8 cat=0);
	void	AddSearchToDownload(CString link,uint8 paused=2, uint8 cat=0);
	void	AddFileLinkToDownload(class CED2KFileLink* pLink,uint8 cat=0);
	bool	IsFileExisting(const uchar* fileid, bool bLogWarnings = true);
	bool	IsPartFile(const CKnownFile* file) const;
	bool	IsTempFile(const CString& rstrDirectory, const CString& rstrName) const;	// SLUGFILLER: SafeHash
	CPartFile* GetFileByID(const uchar* filehash) const;
	CPartFile* GetFileByIndex(int index) const;
	CPartFile* GetFileByKadFileSearchID(uint32 ID) const;
	void    CheckAndAddSource(CPartFile* sender,CUpDownClient* source);
	bool    CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source);
	bool	RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate = true);
	void	DeleteAll();
	void	RemoveFile(CPartFile* toremove);
	uint32	GetDatarate()			{return datarate;}
	void	SortByPriority();
	void	CheckDiskspace(bool bNotEnoughSpaceLeft = false); // SLUGFILLER: checkDiskspace
	void	CheckDiskspaceTimed();
	void	StopUDPRequests();
	typedef struct{
		int	a[19];
	} SDownloadStats;
	void	GetDownloadStats(SDownloadStats& results);
	void	GetDownloadStats(int results[],uint64& pui64TotFileSize,uint64& pui64TotBytesLeftToTransfer,uint64& pui64TotNeededSpace);
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool paused);
	CUpDownClient* GetDownloadClientByIP(uint32 dwIP);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	void	StartNextFile()									{ StartNextFile(-1); }
	void	StartNextFile(int cat);
	void	AddDownDataOverheadSourceExchange(uint32 data)	{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadSourceExchange += data;
															  m_nDownDataOverheadSourceExchangePackets++;}
	void	AddDownDataOverheadFileRequest(uint32 data)		{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadFileRequest += data;
															  m_nDownDataOverheadFileRequestPackets++;}
	void	AddDownDataOverheadServer(uint32 data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadServer += data;
															  m_nDownDataOverheadServerPackets++;}
	void	AddDownDataOverheadOther(uint32 data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadOther += data;
															  m_nDownDataOverheadOtherPackets++;}
	void	AddDownDataOverheadKad(uint32 data)				{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadKad += data;
															  m_nDownDataOverheadKadPackets++;}
	uint32	GetDownDatarateOverhead()					{return m_nDownDatarateOverhead;}
	uint64	GetDownDataOverheadSourceExchange()			{return m_nDownDataOverheadSourceExchange;}
	uint64	GetDownDataOverheadFileRequest()			{return m_nDownDataOverheadFileRequest;}
	uint64	GetDownDataOverheadServer()					{return m_nDownDataOverheadServer;}
	uint64	GetDownDataOverheadKad()					{return m_nDownDataOverheadKad;}
	uint64	GetDownDataOverheadOther()					{return m_nDownDataOverheadOther;}
	uint64	GetDownDataOverheadSourceExchangePackets()	{return m_nDownDataOverheadSourceExchangePackets;}
	uint64	GetDownDataOverheadFileRequestPackets()		{return m_nDownDataOverheadFileRequestPackets;}
	uint64	GetDownDataOverheadServerPackets()			{return m_nDownDataOverheadServerPackets;}
	uint64	GetDownDataOverheadKadPackets()				{return m_nDownDataOverheadKadPackets;}
	uint64	GetDownDataOverheadOtherPackets()			{return m_nDownDataOverheadOtherPackets;}
	void	CompDownDatarateOverhead();
	int		GetFileCount()								{return filelist.GetCount();}
	void	ResetCatParts(int cat);
	void	SetCatPrio(int cat, uint8 newprio);
	void	SetCatStatus(int cat, int newstatus);
	void	MoveCat(uint8 from, uint8 to);
	UINT	GetDownloadingFileCount() const;
	uint16	GetPausedFileCount();
	void	DisableAllA4AFAuto(void);
	void	SetAutoCat(CPartFile* newfile);
	void	SendLocalSrcRequest(CPartFile* sender);
	void	RemoveLocalServerRequest(CPartFile* pFile);
	void	ResetLocalServerRequests();
	bool 	IsInList(const CUpDownClient* client) const;
	void	SetLastKademliaFileRequest()				{lastkademliafilerequest = ::GetTickCount();}
	bool	DoKademliaFileRequest();
	void	KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 serverip, uint16 serverport, uint32 clientid);
	void	ExportPartMetFilesOverview() const;
	void	OnConnectionState(bool bConnected);

	CServer* cur_udpserver;

protected:
	bool	SendNextUDPPacket();
	void	ProcessLocalRequests();
	int		GetMaxFilesPerUDPServerPacket() const;
	bool	SendGlobGetSourcesUDPPacket(CSafeMemFile* data);

private:
	// SLUGFILLER: checkDiskspace
	bool	CompareParts(POSITION pos1, POSITION pos2);
	void	SwapParts(POSITION pos1, POSITION pos2);
	void	HeapSort(uint16 first, uint16 last);
	// SLUGFILLER: checkDiskspace
	CTypedPtrList<CPtrList, CPartFile*> filelist;
	CTypedPtrList<CPtrList, CPartFile*> m_localServerReqQueue;
	CSharedFileList* sharedfilelist;
	uint16	filesrdy;
	uint32	datarate;
	
	CPartFile*	lastfile;
	uint32		lastcheckdiskspacetime;	// SLUGFILLER: checkDiskspace
	uint32		lastudpsearchtime;
	uint32		lastudpstattime;
	uint8		udcounter;
	uint8		m_cRequestsSentToServer;
	uint32		m_dwNextTCPSrcReq;
	int			m_iSearchedServers;
	uint32		lastkademliafilerequest;

	uint64		m_datarateMS;
	uint32		m_nDownDatarateOverhead;
	uint32		m_nDownDataRateMSOverhead;
	uint64		m_nDownDataOverheadSourceExchange;
	uint64		m_nDownDataOverheadSourceExchangePackets;
	uint64		m_nDownDataOverheadFileRequest;
	uint64		m_nDownDataOverheadFileRequestPackets;
	uint64		m_nDownDataOverheadServer;
	uint64		m_nDownDataOverheadServerPackets;
	uint64		m_nDownDataOverheadOther;
	uint64		m_nDownDataOverheadOtherPackets;
	uint64		m_nDownDataOverheadKad;
	uint64		m_nDownDataOverheadKadPackets;

	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData,TransferredData> avarage_dr_list;
	CList<TransferredData,TransferredData>	m_AvarageDDRO_list;
	uint32 sumavgDDRO;
	// END By BadWolf - Accurate Speed Measurement

    //DWORD m_lastRefreshedDLDisplay;
	CSourceHostnameResolveWnd m_srcwnd;		// SLUGFILLER: hostnameSources
};
