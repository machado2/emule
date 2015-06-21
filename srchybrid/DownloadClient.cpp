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
#include "stdafx.h"
#include "emule.h"
#include <zlib/zlib.h>
#include "UpDownClient.h"
#include "PartFile.h"
#include "OtherFunctions.h"
#include "ListenSocket.h"
#include "opcodes.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#ifndef _CONSOLE
#include "emuledlg.h"
#include "TransferWnd.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//	members of CUpDownClient
//	which are mainly used for downloading functions 
CBarShader CUpDownClient::s_StatusBar(16);
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	COLORREF crBoth; 
	COLORREF crNeither; 
	COLORREF crClientOnly; 
	COLORREF crPending;
	COLORREF crNextPending;

	if(bFlat) { 
		crBoth = RGB(0, 0, 0);
		crNeither = RGB(224, 224, 224);
		crClientOnly = RGB(0, 100, 255);
		crPending = RGB(0,150,0);
		crNextPending = RGB(255,208,0);
	} else { 
		crBoth = RGB(104, 104, 104);
		crNeither = RGB(240, 240, 240);
		crClientOnly = RGB(0, 100, 255);
		crPending = RGB(0, 150, 0);
		crNextPending = RGB(255,208,0);
	} 
						   
	ASSERT(reqfile);
	s_StatusBar.SetFileSize(reqfile->GetFileSize()); 
	s_StatusBar.SetHeight(rect->bottom - rect->top); 
	s_StatusBar.SetWidth(rect->right - rect->left); 
	s_StatusBar.Fill(crNeither); 

	uint32 uEnd; 

	// Barry - was only showing one part from client, even when reserved bits from 2 parts
	CString gettingParts;
	ShowDownloadingParts(&gettingParts);

	if (!onlygreyrect && reqfile && m_abyPartStatus) { 
		for (uint32 i = 0;i < m_nPartCount;i++){
			if (m_abyPartStatus[i]){ 
				if (PARTSIZE*(i+1) > reqfile->GetFileSize()) 
					uEnd = reqfile->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(i+1); 

				if (reqfile->IsComplete(PARTSIZE*i,PARTSIZE*(i+1)-1)) 
					s_StatusBar.FillRange(PARTSIZE*i, uEnd, crBoth);
				else if (m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset < uEnd &&
				         m_nLastBlockOffset >= PARTSIZE*i)
					s_StatusBar.FillRange(PARTSIZE*i, uEnd, crPending);
				else if (gettingParts.GetAt((uint16)i) == 'Y') //Sony: cast to (uint16) to fix VC7.1 2GB+ file error
					s_StatusBar.FillRange(PARTSIZE*i, uEnd, crNextPending);
				else
					s_StatusBar.FillRange(PARTSIZE*i, uEnd, crClientOnly);
			} 
		} 
	} 
	s_StatusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 

bool CUpDownClient::Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash) const
{
	if(!bIgnoreUserhash && HasValidHash() && tocomp->HasValidHash())
	    return !md4cmp(this->GetUserHash(), tocomp->GetUserHash());
	if (HasLowID()){
        if (GetUserIDHybrid()!=0
			&& GetUserIDHybrid() == tocomp->GetUserIDHybrid()
			&& GetServerIP()!=0
			&& GetServerIP() == tocomp->GetServerIP()
			&& GetServerPort()!=0
			&& GetServerPort() == tocomp->GetServerPort())
            return true;
		if (GetIP()!=0
			&& GetIP() == tocomp->GetIP()
			&& GetUserPort()!=0
			&& GetUserPort() == tocomp->GetUserPort())
          return true;
        if (GetIP()!=0 
			&& GetIP() == tocomp->GetIP() 
			&& GetKadPort()!=0 
			&& GetKadPort() == tocomp->GetKadPort())
            return true;
        return false;
    }
    if (GetUserPort()!=0){
        if (GetUserIDHybrid() == tocomp->GetUserIDHybrid()
			&& GetUserPort() == tocomp->GetUserPort())
            return true;
        if (GetIP()!=0
			&& GetIP() == tocomp->GetIP()
			&& GetUserPort() == tocomp->GetUserPort())
            return true;
    }
	if(GetKadPort()!=0){
		if (GetUserIDHybrid() == tocomp->GetUserIDHybrid()
			&& GetKadPort() == tocomp->GetKadPort())
			return true;
		if( GetIP()!=0
			&& GetIP() == tocomp->GetIP()
			&& GetKadPort() == tocomp->GetKadPort())
			return true;
	}
	return false;
}

// Return bool is not if you asked or not..
// false = Client was deleted!
// true = client was not deleted!
bool CUpDownClient::AskForDownload()
{
	if (theApp.listensocket->TooManySockets() && !(socket && socket->IsConnected()) ){
		if (GetDownloadState() != DS_TOOMANYCONNS)
			SetDownloadState(DS_TOOMANYCONNS);
		return true;
	}

	m_bUDPPending = false;
	m_dwLastAskedTime = ::GetTickCount();
	SetDownloadState(DS_CONNECTING);
	return TryToConnect();
}

bool CUpDownClient::IsSourceRequestAllowed() const
{
	DWORD dwTickCount = ::GetTickCount() + CONNECTION_LATENCY;
	unsigned int nTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	unsigned int nTimePassedFile   = dwTickCount - reqfile->GetLastAnsweredTime();
	bool bNeverAskedBefore = GetLastAskedForSources() == 0;

	UINT uSources = reqfile->GetSourceCount();
	return (
	         //if client has the correct extended protocol
	         ExtProtocolAvailable() && GetSourceExchangeVersion() > 1 &&
	         //AND if we need more sources
	         thePrefs.GetMaxSourcePerFileSoft() > uSources &&
	         //AND if...
	         (
	           //source is not complete and file is very rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE/5)
	           ) ||
	           //source is not complete and file is rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE || uSources - reqfile->GetValidSourcesCount() <= RARE_FILE / 2)
				 && (nTimePassedFile > SOURCECLIENTREASKF)
	           ) ||
	           // OR if file is not rare
			   ( (bNeverAskedBefore || nTimePassedClient > (unsigned)(SOURCECLIENTREASKS * MINCOMMONPENALTY)) 
				 && (nTimePassedFile > (unsigned)(SOURCECLIENTREASKF * MINCOMMONPENALTY))
	           )
	         )
	       );
}

void CUpDownClient::SendFileRequest()
{
	ASSERT(reqfile != NULL);
	if(!reqfile)
		return;
	AddAskedCountDown();

	CSafeMemFile dataFileReq(16+16);
	dataFileReq.WriteHash16(reqfile->GetFileHash());

	if( SupportMultiPacket() )
	{
		dataFileReq.WriteUInt8(OP_REQUESTFILENAME);
		//Extended information
		if( GetExtendedRequestsVersion() > 0 )
			reqfile->WritePartStatus(&dataFileReq);
		if( GetExtendedRequestsVersion() > 1 )
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		if (reqfile->GetPartCount() > 1)
			dataFileReq.WriteUInt8(OP_SETREQFILEID);
		if( IsEmuleClient() )
		{
			SetRemoteQueueFull( true );
			SetRemoteQueueRank(0);
		}	
		if(IsSourceRequestAllowed())
		{
			dataFileReq.WriteUInt8(OP_REQUESTSOURCES);
			reqfile->SetLastAnsweredTimeTimeout();
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine( false, "Send:Source Request User(%s) File(%s)", GetUserName(), reqfile->GetFileName() );
		}
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MultiPacket", this, (char*)reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq, OP_EMULEPROT);
		packet->opcode = OP_MULTIPACKET;
		theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	}
	else
	{
		//This is extended information
		if( GetExtendedRequestsVersion() > 0 ){
			reqfile->WritePartStatus(&dataFileReq);
		}
		if( GetExtendedRequestsVersion() > 1 ){
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		}
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__FileRequest", this, (char*)reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq);
		packet->opcode=OP_REQUESTFILENAME;
		theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	
		// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
		// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
		// know that the file is shared, we know also that the file is complete and don't need to request the file status.
		if (reqfile->GetPartCount() > 1)
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__SetReqFileID", this, (char*)reqfile->GetFileHash());
		    CSafeMemFile dataSetReqFileID(16);
			dataSetReqFileID.WriteHash16(reqfile->GetFileHash());
		    packet = new Packet(&dataSetReqFileID);
		    packet->opcode = OP_SETREQFILEID;
		    theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
		    socket->SendPacket(packet, true);
		}
	
		if( IsEmuleClient() )
		{
			SetRemoteQueueFull( true );
			SetRemoteQueueRank(0);
		}	
		if(IsSourceRequestAllowed()) 
		{
		    if (thePrefs.GetDebugClientTCPLevel() > 0){
			    DebugSend("OP__RequestSources", this, (char*)reqfile->GetFileHash());
			    if (GetLastAskedForSources() == 0)
				    Debug("  first source request\n");
			    else
				    Debug("  last source request was before %s\n", CastSecondsToHM((GetTickCount() - GetLastAskedForSources())/1000));
		    }
			reqfile->SetLastAnsweredTimeTimeout();
			Packet* packet = new Packet(OP_REQUESTSOURCES,16,OP_EMULEPROT);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			theApp.uploadqueue->AddUpDataOverheadSourceExchange(packet->size);
			socket->SendPacket(packet,true,true);
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine( false, "Send:Source Request User(%s) File(%s)", GetUserName(), reqfile->GetFileName() );
		}
	}
}
void CUpDownClient::SendStartupLoadReq()
{
	if (socket==NULL || reqfile==NULL)
	{
		ASSERT(0);
		return;
	}
	SetDownloadState(DS_ONQUEUE);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__StartupLoadReq", this);
	CSafeMemFile dataStartupLoadReq(16);
	dataStartupLoadReq.WriteHash16(reqfile->GetFileHash());
	Packet* packet = new Packet(&dataStartupLoadReq);
	packet->opcode = OP_STARTUPLOADREQ;
	theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet, true, true);
}

void CUpDownClient::ProcessFileInfo(CSafeMemFile* data, CPartFile* file)
{
	if (file==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; file==NULL)");
	if (reqfile==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile==NULL)");
	if (file != reqfile)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile!=file)");
	m_strClientFilename = data->ReadString();
	// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	// know that the file is shared, we know also that the file is complete and don't need to request the file status.
	if (reqfile->GetPartCount() == 1)
	{
		if (m_abyPartStatus)
		{
			delete[] m_abyPartStatus;
			m_abyPartStatus = NULL;
		}
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus,1,m_nPartCount);
		m_bCompleteSource = true;

		if (thePrefs.GetDebugClientTCPLevel() > 0)
		{
		    int iNeeded = 0;
		    for (int i = 0; i < m_nPartCount; i++)
			    if (!reqfile->IsComplete(i*PARTSIZE,((i+1)*PARTSIZE)-1))
				    iNeeded++;
			char* psz = new char[m_nPartCount + 1];
			for (int i = 0; i < m_nPartCount; i++)
				psz[i] = m_abyPartStatus[i] ? '#' : '.';
			psz[i] = '\0';
			Debug("  Parts=%u  %s  Needed=%u\n", m_nPartCount, psz, iNeeded);
			delete[] psz;
		}
		UpdateDisplayedInfo();
		reqfile->UpdateAvailablePartsCount();
		// even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (reqfile->hashsetneeded)
		{
			if (socket)
			{
				Packet* packet = new Packet(OP_HASHSETREQUEST,16);
				md4cpy(packet->pBuffer,reqfile->GetFileHash());
				theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet,true,true);
				SetDownloadState(DS_REQHASHSET);
				m_fHashsetRequesting = 1;
				reqfile->hashsetneeded = false;
			}
			else
				ASSERT(0);
		}
		else
		{
			SendStartupLoadReq();
		}
		reqfile->UpdatePartsInfo();
	}
}

void CUpDownClient::ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file)
{
	if ( !reqfile || file != reqfile )
	{
		if (reqfile==NULL)
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile==NULL)");
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile!=file)");
	}
	uint16 nED2KPartCount = data->ReadUInt16();
	if (m_abyPartStatus)
	{
		delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
	}
	bool bPartsNeeded = false;
	int iNeeded = 0;
	if (!nED2KPartCount)
	{
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus,1,m_nPartCount);
		bPartsNeeded = true;
		m_bCompleteSource = true;
		if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
		{
			for (int i = 0; i < m_nPartCount; i++)
			{
				if (!reqfile->IsComplete(i*PARTSIZE,((i+1)*PARTSIZE)-1))
					iNeeded++;
			}
		}
	}
	else
	{
		if (reqfile->GetED2KPartCount() != nED2KPartCount)
		{
			CString strError;
			strError.Format(_T("ProcessFileStatus - wrong part number recv=%u  expected=%u  %s"), nED2KPartCount, reqfile->GetED2KPartCount(), DbgGetFileInfo(reqfile->GetFileHash()));
			m_nPartCount = 0;
			throw strError;
		}
		m_nPartCount = reqfile->GetPartCount();

		m_bCompleteSource = false;
		m_abyPartStatus = new uint8[m_nPartCount];
		uint16 done = 0;
		while (done != m_nPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (sint32 i = 0;i != 8;i++)
			{
				m_abyPartStatus[done] = ((toread>>i)&1)? 1:0; 	
				if (m_abyPartStatus[done])
				{
					if (!reqfile->IsComplete(done*PARTSIZE,((done+1)*PARTSIZE)-1)){
						bPartsNeeded = true;
						iNeeded++;
					}
				}
				done++;
				if (done == m_nPartCount)
					break;
			}
		}
	}
	
	if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
	{
		char* psz = new char[m_nPartCount + 1];
		for (int i = 0; i < m_nPartCount; i++)
			psz[i] = m_abyPartStatus[i] ? '#' : '.';
		psz[i] = '\0';
		Debug("  Parts=%u  %s  Needed=%u\n", m_nPartCount, psz, iNeeded);
		delete[] psz;
	}

	UpdateDisplayedInfo();
	reqfile->UpdateAvailablePartsCount();

	// NOTE: This function is invoked from TCP and UDP socket!
	if (!bUdpPacket)
	{
		if (!bPartsNeeded)
			SetDownloadState(DS_NONEEDEDPARTS);
		//If we are using the eMule filerequest packets, this is taken care of in the Multipacket!
		else if (reqfile->hashsetneeded)
		{
			if (socket)
			{
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__HashSetRequest", this, (char*)reqfile->GetFileHash());
				Packet* packet = new Packet(OP_HASHSETREQUEST,16);
				md4cpy(packet->pBuffer,reqfile->GetFileHash());
				theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet, true, true);
				SetDownloadState(DS_REQHASHSET);
				m_fHashsetRequesting = 1;
				reqfile->hashsetneeded = false;
			}
			else
				ASSERT(0);
		}
		else
		{
			SendStartupLoadReq();
		}
	}
	else
	{
		if (!bPartsNeeded)
			SetDownloadState(DS_NONEEDEDPARTS);
		else
			SetDownloadState(DS_ONQUEUE);
	}
	reqfile->UpdatePartsInfo();
}

bool CUpDownClient::AddRequestForAnotherFile(CPartFile* file){
	for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;){
		if (m_OtherNoNeeded_list.GetNext(pos) == file)
			return false;
	}
	for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;){
		if (m_OtherRequests_list.GetNext(pos) == file)
			return false;
	}
	m_OtherRequests_list.AddTail(file);
	file->A4AFsrclist.AddTail(this); // [enkeyDEV(Ottavio84) -A4AF-]

	return true;
}

void CUpDownClient::ClearDownloadBlockRequests()
{
	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;){
		Requested_Block_Struct* cur_block = m_DownloadBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(cur_block->StartOffset,cur_block->EndOffset);
		}
		delete cur_block;
	}
	m_DownloadBlocks_list.RemoveAll();

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(pending->block->StartOffset, pending->block->EndOffset);
		}

		delete pending->block;
		// Not always allocated
		if (pending->zStream){
			inflateEnd(pending->zStream);
			delete pending->zStream;
		}
		delete pending;
	}
	m_PendingBlocks_list.RemoveAll();
}

void CUpDownClient::SetDownloadState(EDownloadState nNewState){
	if (m_nDownloadState != nNewState){
		if (reqfile){
		    if(nNewState == DS_DOWNLOADING){
			    reqfile->AddDownloadingSource(this);
		    }
		    else if(m_nDownloadState == DS_DOWNLOADING){
			    reqfile->RemoveDownloadingSource(this);
		    }
		}

		if (m_nDownloadState == DS_DOWNLOADING ){

			// -khaos--+++> Extended Statistics (Successful/Failed Download Sessions)
			if ( m_bTransferredDownMini && nNewState != DS_ERROR )
				thePrefs.Add2DownSuccessfulSessions(); // Increment our counters for successful sessions (Cumulative AND Session)
			else
				thePrefs.Add2DownFailedSessions(); // Increment our counters failed sessions (Cumulative AND Session)
			thePrefs.Add2DownSAvgTime(GetDownTimeDifference()/1000);
			// <-----khaos-

			m_nDownloadState = nNewState;

			ClearDownloadBlockRequests();
				
			m_nDownDatarate = 0;
			if (nNewState == DS_NONE){
				if (m_abyPartStatus)
					delete[] m_abyPartStatus;
				m_abyPartStatus = NULL;
				m_nPartCount = 0;
			}
			if (socket && nNewState != DS_ERROR)
				socket->DisableDownloadLimit();
		}
		m_nDownloadState = nNewState;
		if( GetDownloadState() == DS_DOWNLOADING ){
			if ( IsEmuleClient() )
				SetRemoteQueueFull(false);
			SetRemoteQueueRank(0);
			SetAskedCountDown(0);
		}
		UpdateDisplayedInfo(true);
	}
}

void CUpDownClient::ProcessHashSet(char* packet,uint32 size){
	if (!m_fHashsetRequesting)
		throw CString(_T("unwanted hashset"));
	if ( (!reqfile) || md4cmp(packet,reqfile->GetFileHash()))
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessHashSet)");	
	CSafeMemFile data((BYTE*)packet,size);
	if (reqfile->LoadHashsetFromFile(&data,true)){
		m_fHashsetRequesting = 0;
	}
	else{
		reqfile->hashsetneeded = true;
		throw GetResString(IDS_ERR_BADHASHSET);
	}
	SendStartupLoadReq();
}

void CUpDownClient::SendBlockRequests(){
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__RequestParts", this, reqfile!=NULL ? (char*)reqfile->GetFileHash() : NULL);
	m_dwLastBlockReceived = ::GetTickCount();
	if (!reqfile)
		return;
	if (m_DownloadBlocks_list.IsEmpty())
	{
		// Barry - instead of getting 3, just get how many is needed
		uint16 count = 3 - m_PendingBlocks_list.GetCount();
		Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
		if (reqfile->GetNextRequestedBlock(this,toadd,&count)){
			for (int i = 0; i < count; i++)
				m_DownloadBlocks_list.AddTail(toadd[i]);			
		}
		delete[] toadd;
	}

	// Barry - Why are unfinished blocks requested again, not just new ones?

	while (m_PendingBlocks_list.GetCount() < 3 && !m_DownloadBlocks_list.IsEmpty()){
		Pending_Block_Struct* pblock = new Pending_Block_Struct;
		pblock->block = m_DownloadBlocks_list.RemoveHead();
		pblock->zStream = NULL;
		pblock->totalUnzipped = 0;
		pblock->fZStreamError = 0;
		pblock->fRecovered = 0;
		m_PendingBlocks_list.AddTail(pblock);
	}
	if (m_PendingBlocks_list.IsEmpty()){
		if (!GetSentCancelTransfer()){
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__CancelTransfer", this);
			Packet* packet = new Packet(OP_CANCELTRANSFER,0);
			theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
			socket->SendPacket(packet,true,true);
			SetSentCancelTransfer(1);
		}
		SetDownloadState(DS_NONEEDEDPARTS);
		return;
	}
	const int iPacketSize = 16+(3*4)+(3*4); // 40
	Packet* packet = new Packet(OP_REQUESTPARTS,iPacketSize);
	CSafeMemFile data((BYTE*)packet->pBuffer,iPacketSize);
	data.WriteHash16(reqfile->GetFileHash());
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
			ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
			//ASSERT( pending->zStream == NULL );
			//ASSERT( pending->totalUnzipped == 0 );
			pending->fZStreamError = 0;
			pending->fRecovered = 0;
			data.WriteUInt32(pending->block->StartOffset);
		}
		else
			data.WriteUInt32(0);
	}
	pos = m_PendingBlocks_list.GetHeadPosition();
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
			uint32 endpos = block->EndOffset+1;
			data.WriteUInt32(endpos);
			if (thePrefs.GetDebugClientTCPLevel() > 0){
				CString strInfo;
				strInfo.Format("  Block request: Start%u=%u  End%u=%u  Size=%u  Part=%u-%u", i, block->StartOffset, i, endpos, endpos - block->StartOffset, block->StartOffset/PARTSIZE, (endpos-1)/PARTSIZE);
				strInfo.AppendFormat(",  Complete=%s", reqfile->IsComplete(block->StartOffset, block->EndOffset) ? "Yes(NOTE:)" : "No");
				strInfo.AppendFormat(",  PureGap=%s", reqfile->IsPureGap(block->StartOffset, block->EndOffset) ? "Yes" : "No(NOTE:)");
				strInfo.AppendFormat(",  AlreadyRequested=%s", reqfile->IsAlreadyRequested(block->StartOffset, block->EndOffset) ? "Yes" : "No(NOTE:)");
				strInfo += '\n';
				Debug(strInfo);
			}
		}
		else
		{
			data.WriteUInt32(0);
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				Debug("  Block request: Start%u=%u  End%u=%u  Size=%u\n", i, 0, i, 0, 0);
		}
	}
	theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

/* Barry - Originally this only wrote to disk when a full 180k block 
           had been received from a client, and only asked for data in 
		   180k blocks.

		   This meant that on average 90k was lost for every connection
		   to a client data source. That is a lot of wasted data.

		   To reduce the lost data, packets are now written to a buffer
		   and flushed to disk regularly regardless of size downloaded.
		   This includes compressed packets.

		   Data is also requested only where gaps are, not in 180k blocks.
		   The requests will still not exceed 180k, but may be smaller to
		   fill a gap.
*/
void CUpDownClient::ProcessBlockPacket(char *packet, uint32 size, bool packed)
{
	uint32 nDbgStartPos = *((uint32*)(packet+16));
	if (thePrefs.GetDebugClientTCPLevel() > 0){
		if (packed)
			Debug("  Start=%u  BlockSize=%u  Size=%u  %s\n", nDbgStartPos, *((uint32*)(packet + 16+4)), size-24, DbgGetFileInfo((uchar*)packet));
		else
			Debug("  Start=%u  End=%u  Size=%u  %s\n", nDbgStartPos, *((uint32*)(packet + 16+4)), *((uint32*)(packet + 16+4)) - nDbgStartPos, DbgGetFileInfo((uchar*)packet));
	}

	// Ignore if no data required
	if (!(GetDownloadState() == DS_DOWNLOADING || GetDownloadState() == DS_NONEEDEDPARTS))
		return;

	const int HEADER_SIZE = 24;

	// Update stats
	m_dwLastBlockReceived = ::GetTickCount();

	// Read data from packet
	CSafeMemFile data((BYTE*)packet, size);
	uchar fileID[16];
	data.ReadHash16(fileID);

	// Check that this data is for the correct file
	if ( (!reqfile) || md4cmp(packet, reqfile->GetFileHash()))
	{
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessBlockPacket)");
	}

	// Find the start & end positions, and size of this chunk of data
	uint32 nStartPos;
	uint32 nEndPos;
	uint32 nBlockSize = 0;
	nStartPos = data.ReadUInt32();
	if (packed)
	{
		nBlockSize = data.ReadUInt32();
		nEndPos = nStartPos + (size - HEADER_SIZE);
	}
	else
		nEndPos = data.ReadUInt32();

	// Check that packet size matches the declared data size + header size (24)
	if (nEndPos == nStartPos || size != ((nEndPos - nStartPos) + HEADER_SIZE))
		throw GetResString(IDS_ERR_BADDATABLOCK) + _T(" (ProcessBlockPacket)");

	// -khaos--+++>
	// Extended statistics information based on which client and remote port sent this data.
	// The new function adds the bytes to the grand total as well as the given client/port.
	// bFromPF is not relevant to downloaded data.  It is purely an uploads statistic.
	thePrefs.Add2SessionTransferData(GetClientSoft(), GetUserPort(), false, false, size - HEADER_SIZE);
	// <-----khaos-

	m_nDownDataRateMS += size - HEADER_SIZE;
	credits->AddDownloaded(size - HEADER_SIZE, GetIP());

	// Move end back one, should be inclusive
	nEndPos--;

	// Loop through to find the reserved block that this is within
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		Pending_Block_Struct *cur_block = m_PendingBlocks_list.GetNext(pos);
		if ((cur_block->block->StartOffset <= nStartPos) && (cur_block->block->EndOffset >= nStartPos))
		{
			// Found reserved block

			if (cur_block->fZStreamError){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Ignoring %u bytes of block starting at %u because of errornous zstream state for file \"%s\" - %s"), size - HEADER_SIZE, nStartPos, reqfile->GetFileName(), DbgGetClientInfo());
				reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
				return;
			}

			// Remember this start pos, used to draw part downloading in list
			m_nLastBlockOffset = nStartPos;  

			// Occasionally packets are duplicated, no point writing it twice
			// This will be 0 in these cases, or the length written otherwise
			uint32 lenWritten = 0;

			// Handle differently depending on whether packed or not
			if (!packed)
			{
				// Write to disk (will be buffered in part file class)
				lenWritten = reqfile->WriteToBuffer(size - HEADER_SIZE, 
													(BYTE *) (packet + HEADER_SIZE),
													nStartPos,
													nEndPos,
													cur_block->block );
			}
			else // Packed
			{
				ASSERT( (int)size > 0 );
				// Create space to store unzipped data, the size is only an initial guess, will be resized in unzip() if not big enough
				uint32 lenUnzipped = (size * 2); 
				// Don't get too big
				if (lenUnzipped > (EMBLOCKSIZE + 300))
					lenUnzipped = (EMBLOCKSIZE + 300);
				BYTE *unzipped = new BYTE[lenUnzipped];

				// Try to unzip the packet
				int result = unzip(cur_block, (BYTE *)(packet + HEADER_SIZE), (size - HEADER_SIZE), &unzipped, &lenUnzipped);
				// no block can be uncompressed to >2GB, 'lenUnzipped' is obviously errornous.
				if (result == Z_OK && (int)lenUnzipped >= 0)
				{
					if (lenUnzipped > 0) // Write any unzipped data to disk
					{
						ASSERT( (int)lenUnzipped > 0 );

						// Use the current start and end positions for the uncompressed data
						nStartPos = cur_block->block->StartOffset + cur_block->totalUnzipped - lenUnzipped;
						nEndPos = cur_block->block->StartOffset + cur_block->totalUnzipped - 1;

						if (nStartPos > cur_block->block->EndOffset || nEndPos > cur_block->block->EndOffset){
							if (thePrefs.GetVerbose())
								AddDebugLogLine(false, GetResString(IDS_ERR_CORRUPTCOMPRPKG),reqfile->GetFileName(),666);
							reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
							// There is no chance to recover from this error
						}
						else{
							// Write uncompressed data to file
							lenWritten = reqfile->WriteToBuffer(size - HEADER_SIZE,
								unzipped,
								nStartPos,
								nEndPos,
								cur_block->block );
						}
					}
				}
				else
				{
					if (thePrefs.GetVerbose())
					{
						CString strZipError;
						if (cur_block->zStream && cur_block->zStream->msg)
							strZipError.Format(_T(" - %s"), cur_block->zStream->msg);
						if (result == Z_OK && (int)lenUnzipped < 0){
							ASSERT(0);
							strZipError.AppendFormat(_T("; Z_OK,lenUnzipped=%d"), lenUnzipped);
						}
						AddDebugLogLine(false, GetResString(IDS_ERR_CORRUPTCOMPRPKG) + strZipError, reqfile->GetFileName(), result);
					}
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);

					// If we had an zstream error, there is no chance that we could recover from it nor that we
					// could use the current zstream (which is in error state) any longer.
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
						cur_block->zStream = NULL;
					}

					// Although we can't further use the current zstream, there is no need to disconnect the sending 
					// client because the next zstream (a series of 10K-blocks which build a 180K-block) could be
					// valid again. Just ignore all further blocks for the current zstream.
					cur_block->fZStreamError = 1;
					cur_block->totalUnzipped = 0;
				}
				delete [] unzipped;
			}

			// These checks only need to be done if any data was written
			if (lenWritten > 0)
			{
				m_nTransferedDown += lenWritten;
				
				SetTransferredDownMini(); // Sets boolean m_bTransferredDownMini to true // -khaos--+++> For determining whether the current download session was a success or not.

				// If finished reserved block
				if (nEndPos == cur_block->block->EndOffset)
				{
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
					delete cur_block->block;
					// Not always allocated
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
					}
					delete cur_block;
					m_PendingBlocks_list.RemoveAt(posLast);

					// Request next block
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("More block requests", this);
					SendBlockRequests();	
				}
			}

			// Stop looping and exit method
			return;
		}
	}
}

int CUpDownClient::unzip(Pending_Block_Struct* block, BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion)
{
#define TRACE_UNZIP	/*TRACE*/

	TRACE_UNZIP("unzip: Zipd=%6u Unzd=%6u Rcrs=%d", lenZipped, *lenUnzipped, iRecursion);
  	int err = Z_DATA_ERROR;
  	try
	{
	    // Save some typing
	    z_stream *zS = block->zStream;
    
	    // Is this the first time this block has been unzipped
	    if (zS == NULL)
	    {
		    // Create stream
		    block->zStream = new z_stream;
		    zS = block->zStream;
    
		    // Initialise stream values
		    zS->zalloc = (alloc_func)0;
		    zS->zfree = (free_func)0;
		    zS->opaque = (voidpf)0;
    
		    // Set output data streams, do this here to avoid overwriting on recursive calls
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
    
		    // Initialise the z_stream
		    err = inflateInit(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: new stream failed: %d\n", err);
			    return err;
			}

			ASSERT( block->totalUnzipped == 0 );
		}

	    // Use whatever input is provided
	    zS->next_in  = zipped;
	    zS->avail_in = lenZipped;
    
	    // Only set the output if not being called recursively
	    if (iRecursion == 0)
	    {
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
	    }
    
	    // Try to unzip the data
		TRACE_UNZIP("; inflate(ain=%6u tin=%6u aout=%6u tout=%6u)", zS->avail_in, zS->total_in, zS->avail_out, zS->total_out);
	    err = inflate(zS, Z_SYNC_FLUSH);
    
	    // Is zip finished reading all currently available input and writing all generated output
	    if (err == Z_STREAM_END)
	    {
		    // Finish up
		    err = inflateEnd(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: end stream failed: %d\n", err);
			    return err;
			}
			TRACE_UNZIP("; Z_STREAM_END\n");

		    // Got a good result, set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else if ((err == Z_OK) && (zS->avail_out == 0) && (zS->avail_in != 0))
	    {
		    // Output array was not big enough, call recursively until there is enough space
			TRACE_UNZIP("; output array not big enough (ain=%u)\n", zS->avail_in);
    
		    // What size should we try next
		    uint32 newLength = (*lenUnzipped) *= 2;
		    if (newLength == 0)
			    newLength = lenZipped * 2;
    
		    // Copy any data that was successfully unzipped to new array
		    BYTE *temp = new BYTE[newLength];
			ASSERT( zS->total_out - block->totalUnzipped <= newLength );
		    memcpy(temp, (*unzipped), (zS->total_out - block->totalUnzipped));
		    delete [] (*unzipped);
		    (*unzipped) = temp;
		    (*lenUnzipped) = newLength;
    
		    // Position stream output to correct place in new array
		    zS->next_out = (*unzipped) + (zS->total_out - block->totalUnzipped);
		    zS->avail_out = (*lenUnzipped) - (zS->total_out - block->totalUnzipped);
    
		    // Try again
		    err = unzip(block, zS->next_in, zS->avail_in, unzipped, lenUnzipped, iRecursion + 1);
	    }
	    else if ((err == Z_OK) && (zS->avail_in == 0))
	    {
			TRACE_UNZIP("; all input processed\n");
		    // All available input has been processed, everything ok.
		    // Set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else
	    {
		    // Should not get here unless input data is corrupt
			if (thePrefs.GetVerbose())
			{
				CString strZipError;
				if (zS->msg)
					strZipError.Format(_T(" %d: '%s'"), err, zS->msg);
				else if (err != Z_OK)
					strZipError.Format(_T(" %d"), err);
				TRACE_UNZIP("; Error: %s\n", strZipError);
				AddDebugLogLine(false, _T("Unexpected zip error%s in file \"%s\""), strZipError, reqfile ? reqfile->GetFileName() : NULL);
			}
	    }
    
	    if (err != Z_OK)
		    (*lenUnzipped) = 0;
  	}
  	catch (...){
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Unknown exception in %s: file \"%s\""), __FUNCTION__, reqfile ? reqfile->GetFileName() : NULL);
		err = Z_DATA_ERROR;
		ASSERT(0);
	}

	return err;
}

uint32 CUpDownClient::CalculateDownloadRate(){

	// Patch By BadWolf - Accurate datarate Calculation
	TransferredData newitem = {m_nDownDataRateMS,::GetTickCount()};
	m_AvarageDDR_list.AddTail(newitem);
	m_nSumForAvgDownDataRate += m_nDownDataRateMS;
	m_nDownDataRateMS = 0;

	while (m_AvarageDDR_list.GetCount()>500)
		m_nSumForAvgDownDataRate -= m_AvarageDDR_list.RemoveHead().datalen;
	
	if(m_AvarageDDR_list.GetCount() > 1){
		DWORD dwDuration = m_AvarageDDR_list.GetTail().timestamp - m_AvarageDDR_list.GetHead().timestamp;
		if (dwDuration)
			m_nDownDatarate = 1000 * m_nSumForAvgDownDataRate / dwDuration;
	}
	else
		m_nDownDatarate = 0;
	// END Patch By BadWolf

	m_cShowDR++;
	if (m_cShowDR == 30){
		m_cShowDR = 0;
		UpdateDisplayedInfo();
	}
	if ((::GetTickCount() - m_dwLastBlockReceived) > DOWNLOADTIMEOUT){
		if (!GetSentCancelTransfer()){
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__CancelTransfer", this);
			Packet* packet = new Packet(OP_CANCELTRANSFER,0);
			theApp.uploadqueue->AddUpDataOverheadFileRequest(packet->size);
			socket->SendPacket(packet,true,true);
			SetSentCancelTransfer(1);
		}
		SetDownloadState(DS_ONQUEUE);
	}
		
	return m_nDownDatarate;
}

uint16 CUpDownClient::GetAvailablePartCount() const
{
	uint16 result = 0;
	for (int i = 0;i < m_nPartCount;i++){
		if (IsPartAvailable(i))
			result++;
	}
	return result;
}

void CUpDownClient::SetRemoteQueueRank(uint16 nr){
	m_nRemoteQueueRank = nr;
	UpdateDisplayedInfo();
}

void CUpDownClient::UDPReaskACK(uint16 nNewQR){
	m_bUDPPending = false;
	SetRemoteQueueRank(nNewQR);
	m_dwLastAskedTime = ::GetTickCount();
}

void CUpDownClient::UDPReaskFNF(){
	m_bUDPPending = false;
	if (GetDownloadState()!=DS_DOWNLOADING){ // avoid premature deletion of 'this' client
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false,CString("UDP ANSWER FNF : %s - %s"),DbgGetClientInfo(), DbgGetFileInfo(reqfile ? reqfile->GetFileHash() : NULL));
		theApp.downloadqueue->RemoveSource(this);
		if (!socket){
			if (Disconnected("UDPReaskFNF socket=NULL"))
				delete this;
		}
	}
	else
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false,CString("UDP ANSWER FNF : %s - did not remove client because of current download state"),GetUserName());
	}
}

void CUpDownClient::UDPReaskForDownload()
{
	ASSERT ( reqfile );
	if(!reqfile || m_bUDPPending)
		return;

	//the line "m_bUDPPending = true;" use to be here
	// deadlake PROXYSUPPORT
	const ProxySettings& proxy = thePrefs.GetProxy();
	if(m_nUDPPort != 0 && thePrefs.GetUDPPort() != 0 &&
	   !theApp.IsFirewalled() && !HasLowID() && !(socket && socket->IsConnected())&& (!proxy.UseProxy))
	{ 
		// deadlake PROXYSUPPORT
		//don't use udp to ask for sources
		if(IsSourceRequestAllowed())
			return;
		m_bUDPPending = true;
		CSafeMemFile data(128);
		data.WriteHash16(reqfile->GetFileHash());
		if (GetUDPVersion() > 3)
		{
			if (reqfile->IsPartFile())
				((CPartFile*)reqfile)->WritePartStatus(&data);
			else
				data.WriteUInt16(0);
		}
		if (GetUDPVersion() > 2)
			data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
		if (thePrefs.GetDebugClientUDPLevel() > 0)
			DebugSend("OP__ReaskFilePing", this, (char*)reqfile->GetFileHash());
		Packet* response = new Packet(&data, OP_EMULEPROT);
		response->opcode = OP_REASKFILEPING;
		theApp.uploadqueue->AddUpDataOverheadFileRequest(response->size);
		theApp.clientudp->SendPacket(response,GetIP(),GetUDPPort());
	}
}

// Barry - Sets string to show parts downloading, eg NNNYNNNNYYNYN
void CUpDownClient::ShowDownloadingParts(CString *partsYN) const
{
	// Initialise to all N's
	char *n = new char[m_nPartCount+1];
	_strnset(n, 'N', m_nPartCount);
	n[m_nPartCount] = 0;
	partsYN->SetString(n, m_nPartCount);
	delete [] n;

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != 0; )
		partsYN->SetAt((m_PendingBlocks_list.GetNext(pos)->block->StartOffset / PARTSIZE), 'Y');
}

void CUpDownClient::UpdateDisplayedInfo(bool force)
{
    DWORD curTick = ::GetTickCount();

    if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+(uint32)(rand()/(RAND_MAX/1000))) {
	    theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        m_lastRefreshedDLDisplay = curTick;
    }
}


// IgnoreNoNeeded = will switch to files of which this source has no needed parts (if no better fiels found)
// ignoreSuspensions = ignore timelimit for A4Af jumping
// bRemoveCompletely = do not readd the file which the source is swapped from to the A4AF lists (needed if deleting or stopping a file)
// toFile = Try to swap to this partfile only
bool CUpDownClient::SwapToAnotherFile(bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile){
	if (GetDownloadState() == DS_DOWNLOADING)
		return false;

	CPartFile* SwapTo = NULL;
	CPartFile* cur_file = NULL;
	int cur_prio= -1;
	POSITION finalpos = NULL;
	CTypedPtrList<CPtrList, CPartFile*>* usedList = NULL;

	if (!m_OtherRequests_list.IsEmpty()){
		usedList = &m_OtherRequests_list;
		for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;m_OtherRequests_list.GetNext(pos)){
			cur_file = m_OtherRequests_list.GetAt(pos);
			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY))	
			{
				if (toFile != NULL){
					if (cur_file == toFile){
						SwapTo = cur_file;
						finalpos = pos;
						break;
					}
				}
				else if ( cur_file->GetDownPriority()>cur_prio 
					&& (ignoreSuspensions  || (!ignoreSuspensions && !IsSwapSuspended(cur_file)) ) )
				{
					SwapTo = cur_file;
					cur_prio=cur_file->GetDownPriority();
					finalpos=pos;
					if (cur_prio==PR_HIGH)
						break;
				}
			}
		}
	}


	if (!SwapTo && bIgnoreNoNeeded){
		usedList = &m_OtherNoNeeded_list;
		for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)){
			cur_file = m_OtherNoNeeded_list.GetAt(pos);
			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY) )	
			{
				if (toFile != NULL){
					if (cur_file == toFile){
						SwapTo = cur_file;
						finalpos = pos;
						break;
					}
				}
				else if ( cur_file->GetDownPriority()>cur_prio 
					&& (ignoreSuspensions  || (!ignoreSuspensions && !IsSwapSuspended(cur_file)) ) )
				{
					SwapTo = cur_file;
					cur_prio=cur_file->GetDownPriority();
					finalpos=pos;
					if (cur_prio==PR_HIGH)
						break;
				}
			}
		}
	}

	if (SwapTo){
		//if (thePrefs.GetVerbose())
		//	AddDebugLogLine(false, "Swapped source '%s'; Status %i; Remove %s to %s", this->GetUserName(), this->GetDownloadState(), (bRemoveCompletely ? "Yes" : "No" ), SwapTo->GetFileName());
		if (DoSwap(SwapTo,bRemoveCompletely)){

			usedList->RemoveAt(finalpos);
			return true;
		}
	}

	return false;
}

bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely)
{
	// 17-Dez-2003 [bc]: This "reqfile->srclists[sourcesslot].Find(this)" was the only place where 
	// the usage of the "CPartFile::srclists[100]" is more effective than using one list. If this
	// function here is still (again) a performance problem there is a more effective way to handle
	// the 'Find' situation. Hint: usage of a node ptr which is stored in the CUpDownClient.
	POSITION pos = reqfile->srclist.Find(this);
	if(pos)
	{
		// remove this client from the A4AF list of our new reqfile
		POSITION pos2 = SwapTo->A4AFsrclist.Find(this);
		if (pos2){
			SwapTo->A4AFsrclist.RemoveAt(pos2);
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(this,SwapTo);
		}

		reqfile->srclist.RemoveAt(pos);
		reqfile->RemoveDownloadingSource(this);

		if(!bRemoveCompletely)
		{
			reqfile->A4AFsrclist.AddTail(this);
			if (GetDownloadState() == DS_NONEEDEDPARTS)
				m_OtherNoNeeded_list.AddTail(reqfile);
			else
				m_OtherRequests_list.AddTail(reqfile);

			if (!bRemoveCompletely)
				theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(reqfile,this,true);
		}


		SetDownloadState(DS_NONE);
		ResetFileStatusInfo();
		m_nRemoteQueueRank = 0;
		
		reqfile->UpdatePartsInfo();
		reqfile->UpdateAvailablePartsCount();
		reqfile = SwapTo;

		SwapTo->srclist.AddTail(this);
		theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(SwapTo,this,false);

		return true;
	}
	return false;
}

void CUpDownClient::DontSwapTo(/*const*/ CPartFile* file)
{
	DWORD dwNow = ::GetTickCount();

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0; m_DontSwap_list.GetNext(pos))
		if(m_DontSwap_list.GetAt(pos).file == file) {
			m_DontSwap_list.GetAt(pos).timestamp = dwNow ;
			return;
		}
	PartFileStamp newfs = {file, dwNow };
	m_DontSwap_list.AddHead(newfs);
}

bool CUpDownClient::IsSwapSuspended(const CPartFile* file)
{
	if (m_DontSwap_list.GetCount()==0)
		return false;

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0 && m_DontSwap_list.GetCount()>0; m_DontSwap_list.GetNext(pos)){
		if(m_DontSwap_list.GetAt(pos).file == file){
			if ( ::GetTickCount() - m_DontSwap_list.GetAt(pos).timestamp  >= PURGESOURCESWAPSTOP ) {
				m_DontSwap_list.RemoveAt(pos);
				return false;
			}
			else
				return true;
		}
		else if (m_DontSwap_list.GetAt(pos).file == NULL) // in which cases should this happen?
			m_DontSwap_list.RemoveAt(pos);
	}

	return false;
}

bool CUpDownClient::IsValidSource() const
{
	bool valid = false;
	switch(GetDownloadState())
	{
		case DS_DOWNLOADING:
		case DS_ONQUEUE:
		case DS_CONNECTED:
		case DS_NONEEDEDPARTS:
		case DS_REMOTEQUEUEFULL:
		case DS_REQHASHSET:
			valid = true;
	}
	return valid;
}