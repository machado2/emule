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
#pragma once
#include "Loggable.h"

class CUpDownClient;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CUploadQueue: public CLoggable
{
public:
	CUploadQueue();
	~CUploadQueue();

	void	Process();
	void	AddClientToQueue(CUpDownClient* client,bool bIgnoreTimelimit = false);
	bool	RemoveFromUploadQueue(CUpDownClient* client,bool updatewindow = true);
	bool	RemoveFromWaitingQueue(CUpDownClient* client,bool updatewindow = true);
	bool	IsOnUploadQueue(CUpDownClient* client)	const {return (waitinglist.Find(client) != 0);}
	bool	IsDownloading(CUpDownClient* client)	const {return (uploadinglist.Find(client) != 0);}
	uint32	GetDatarate()							{return datarate;}
	bool	CheckForTimeOver(CUpDownClient* client);
	int		GetWaitingUserCount()					{return waitinglist.GetCount();}
	int		GetUploadQueueLength()					{return uploadinglist.GetCount();}
	
	POSITION GetFirstFromUploadList()				{return uploadinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromUploadList(POSITION &curpos)	{return uploadinglist.GetNext(curpos);}
	CUpDownClient* GetQueueClientAt(POSITION &curpos)	{return uploadinglist.GetAt(curpos);}

	POSITION GetFirstFromWaitingList()				{return waitinglist.GetHeadPosition();}
	CUpDownClient* GetNextFromWaitingList(POSITION &curpos)	{return waitinglist.GetNext(curpos);}
	CUpDownClient* GetWaitClientAt(POSITION &curpos)	{return waitinglist.GetAt(curpos);}

	CUpDownClient*	GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	CUpDownClient*	GetWaitingClientByIP(uint32 dwIP);
	CUpDownClient*	GetNextClient(const CUpDownClient* update);

	
	void	DeleteAll();
	uint16	GetWaitingPosition(CUpDownClient* client);
	
	//void	SetBanCount(uint32 in_count)			{bannedcount = in_count;}
	//uint32	GetBanCount()							{return bannedcount;}
	//void	UpdateBanCount();
	
	uint32	GetSuccessfullUpCount()					{return successfullupcount;}
	uint32	GetFailedUpCount()						{return failedupcount;}
	uint32	GetAverageUpTime();
//	void	FindSourcesForFileById(CUpDownClientPtrList* srclist, const uchar* filehash);
	void	AddUpDataOverheadSourceExchange(uint32 data)	{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadSourceExchange += data;
															  m_nUpDataOverheadSourceExchangePackets++;}
	void	AddUpDataOverheadFileRequest(uint32 data)		{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadFileRequest += data;
															  m_nUpDataOverheadFileRequestPackets++;}
	void	AddUpDataOverheadServer(uint32 data)			{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadServer += data;
															  m_nUpDataOverheadServerPackets++;}
	void	AddUpDataOverheadKad(uint32 data)				{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadKad += data;
															  m_nUpDataOverheadKadPackets++;}
	void	AddUpDataOverheadOther(uint32 data)				{ m_nUpDataRateMSOverhead += data;
															  m_nUpDataOverheadOther += data;
															  m_nUpDataOverheadOtherPackets++;}
	uint32	GetUpDatarateOverhead()						{return m_nUpDatarateOverhead;}
	uint64	GetUpDataOverheadSourceExchange()			{return m_nUpDataOverheadSourceExchange;}
	uint64	GetUpDataOverheadFileRequest()				{return m_nUpDataOverheadFileRequest;}
	uint64	GetUpDataOverheadServer()					{return m_nUpDataOverheadServer;}
	uint64	GetUpDataOverheadKad()						{return m_nUpDataOverheadKad;}
	uint64	GetUpDataOverheadOther()					{return m_nUpDataOverheadOther;}
	uint64	GetUpDataOverheadSourceExchangePackets()	{return m_nUpDataOverheadSourceExchangePackets;}
	uint64	GetUpDataOverheadFileRequestPackets()		{return m_nUpDataOverheadFileRequestPackets;}
	uint64	GetUpDataOverheadServerPackets()			{return m_nUpDataOverheadServerPackets;}
	uint64	GetUpDataOverheadKadPackets()				{return m_nUpDataOverheadKadPackets;}
	uint64	GetUpDataOverheadOtherPackets()				{return m_nUpDataOverheadOtherPackets;}
	void	CompUpDatarateOverhead();

protected:
	void	RemoveFromWaitingQueue(POSITION pos, bool updatewindow);
//	POSITION	GetWaitingClient(CUpDownClient* client);
//	POSITION	GetWaitingClientByID(CUpDownClient* client);
//	POSITION	GetDownloadingClient(CUpDownClient* client);
	bool		AcceptNewClient();
	void		AddUpNextClient(CUpDownClient* directadd = 0);
	
	static VOID CALLBACK UploadTimer(HWND hWnd, UINT nMsg, UINT nId, DWORD dwTime);

private:
	void	UpdateMaxClientScore();
	uint32	GetMaxClientScore()						{return m_imaxscore;}

	// By BadWolf - Accurate Speed Measurement
	typedef struct TransferredData {
		uint32	datalen;
		DWORD	timestamp;
	};
	CList<TransferredData,TransferredData> avarage_dr_list;
	DWORD	m_delay;
	DWORD	m_delaytmp;
	uint32	uLastAcceptNewClient;
	uint32	sumavgdata;
	sint32	sendperclient;

	CUpDownClientPtrList waitinglist;
	CUpDownClientPtrList uploadinglist;
	uint32	datarate;   //datarate 
	uint32	dataratems;
	uint32	datarateave; //datarage average (since progstart) *unused*
	uint32	estadatarate; // esta. max datarate	
	UINT_PTR h_timer;
	uint32	successfullupcount;
	uint32	failedupcount;
	uint32	totaluploadtime;
	uint32	m_nLastStartUpload;
	uint32	m_nUpDatarateOverhead;
	uint32	m_nUpDataRateMSOverhead;
	uint64	m_nUpDataOverheadSourceExchange;
	uint64	m_nUpDataOverheadFileRequest;
	uint64	m_nUpDataOverheadServer;
	uint64	m_nUpDataOverheadKad;
	uint64	m_nUpDataOverheadOther;
	uint64	m_nUpDataOverheadSourceExchangePackets;
	uint64	m_nUpDataOverheadFileRequestPackets;
	uint64	m_nUpDataOverheadServerPackets;
	uint64	m_nUpDataOverheadKadPackets;
	uint64	m_nUpDataOverheadOtherPackets;
	bool	lastupslotHighID; // VQB lowID alternation

	uint32	m_imaxscore;
	// By BadWolf - Accurate Speed Measurement	
	CList<TransferredData,TransferredData>	m_AvarageUDRO_list;
	uint32	sumavgUDRO;
	// END By BadWolf - Accurate Speed Measurement	
};
