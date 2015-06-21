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
#include "ClientList.h"
#include "otherfunctions.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/routing/contact.h"
#include "Kademlia/net/kademliaudplistener.h"
#include "LastCommonRouteFinder.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "Opcodes.h"
#include "Sockets.h"
#ifndef _CONSOLE
#include "emuledlg.h"
#include "TransferWnd.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CClientList::CClientList(){
	m_dwLastBannCleanUp = 0;
	m_dwLastTrackedCleanUp = 0;
	m_bannedList.InitHashTable(331);
	m_trackedClientsList.InitHashTable(2011);
}

CClientList::~CClientList(){
	POSITION pos = m_trackedClientsList.GetStartPosition();
	uint32 nKey;
	CDeletedClient* pResult;
	while (pos != NULL){
		m_trackedClientsList.GetNextAssoc( pos, nKey, pResult );
		m_trackedClientsList.RemoveKey(nKey);
		delete pResult;
	}
}

// xrmb : statsclientstatus
// -khaos--+++> Rewritten to accomodate some new statistics, and just for cleanup's sake.
//				I've added three new stats: Number of cDonkey clients, # errored clients, # banned clients.
//				We also now support LMule
void CClientList::GetStatistics(uint32 &totalclient, int stats[], CMap<uint16, uint16, uint32, uint32> *clientVersionEDonkey, CMap<uint16, uint16, uint32, uint32> *clientVersionEDonkeyHybrid, CMap<uint16, uint16, uint32, uint32> *clientVersionEMule, CMap<uint16, uint16, uint32, uint32> *clientVersionLMule){
	totalclient = list.GetCount();
	if(clientVersionEDonkeyHybrid)	clientVersionEDonkeyHybrid->RemoveAll();
	if(clientVersionEDonkey)		clientVersionEDonkey->RemoveAll();
	if(clientVersionEMule)			clientVersionEMule->RemoveAll();
	if(clientVersionLMule)			clientVersionLMule->RemoveAll();

	for (int i=0;i<15;i++) stats[i]=0;

	stats[7]=m_bannedList.GetCount();

	for (POSITION pos = list.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_client =	list.GetNext(pos);

		if (cur_client->HasLowID())
			++stats[14];
		
		switch (cur_client->GetClientSoft()) {

			case SO_EDONKEY :				
				if(clientVersionEDonkey)
				{
					++stats[1];
					(*clientVersionEDonkey)[cur_client->GetVersion()]++;
				}
				break;

			case SO_EDONKEYHYBRID : 
				if(clientVersionEDonkeyHybrid)
				{
					++stats[4];
					(*clientVersionEDonkeyHybrid)[cur_client->GetVersion()]++;
				}
				break;
			case SO_EMULE   :
			case SO_OLDEMULE:
				if(clientVersionEMule)
				{
					++stats[2];
					uint8 version = cur_client->GetMuleVersion();
					if (version == 0xFF || version == 0x66 || version==0x69 || version==0x90 || version==0x33 || version==0x60)
						continue;
					(*clientVersionEMule)[cur_client->GetVersion()]++;
				}
				break;

			case SO_XMULE:
				if(clientVersionLMule)
				{
					++stats[10];
					uint8 version = cur_client->GetMuleVersion();
					if (version == 0x66 || version==0x69 || version==0x90 || version==0x33)
						continue;
					(*clientVersionLMule)[cur_client->GetVersion()]++;
				}

				break;

			case SO_UNKNOWN :	++stats[0];		break;
			case SO_CDONKEY :	++stats[5];		break;
			case SO_MLDONKEY:	++stats[3];		break;
			case SO_SHAREAZA:   ++stats[11];    break;
		}

		if (cur_client->Credits() != NULL){
			switch(cur_client->Credits()->GetCurrentIdentState(cur_client->GetIP())){
				case IS_IDENTIFIED:
					stats[12]++;
					break;
				case IS_IDFAILED:
				case IS_IDNEEDED:
				case IS_IDBADGUY:
					stats[13]++;
			}
		}

		if (cur_client->GetDownloadState()==DS_ERROR || cur_client->GetUploadState()==US_ERROR )
				++stats[6]; // Error

		switch (cur_client->GetUserPort()) {
			case 4662:
				++stats[8]; // Default Port
				break;
			default:
				++stats[9]; // Other Port
		}
	}
}
// <-----khaos-


void CClientList::AddClient(CUpDownClient* toadd, bool bSkipDupTest)
{
	// skipping the check for duplicate list entries is only to be done for optimization purposes, if the calling
	// function has ensured that this client instance is not already within the list -> there are never duplicate
	// client instances in this list.
	if (!bSkipDupTest){
		if(list.Find(toadd))
			return;
	}
	theApp.emuledlg->transferwnd->clientlistctrl.AddClient(toadd);
	list.AddTail(toadd);
}

// ZZ:UploadSpeedSense -->
bool CClientList::GiveClientsForTraceRoute() {
    // this is a host that lastCommonRouteFinder can use to traceroute
    return theApp.lastCommonRouteFinder->AddHostsToCheck(list);
}
// ZZ:UploadSpeedSense <--

void CClientList::RemoveClient(CUpDownClient* toremove){
	POSITION pos = list.Find(toremove);
	if (pos){
		//just to be sure...
		theApp.uploadqueue->RemoveFromUploadQueue(toremove);
		theApp.uploadqueue->RemoveFromWaitingQueue(toremove);
		theApp.downloadqueue->RemoveSource(toremove);
		theApp.emuledlg->transferwnd->clientlistctrl.RemoveClient(toremove);
		RemoveTCP(toremove);
		list.RemoveAt(pos);
	}
}

void CClientList::DeleteAll(){
	theApp.uploadqueue->DeleteAll();
	theApp.downloadqueue->DeleteAll();
	POSITION pos1, pos2;
	for (pos1 = list.GetHeadPosition();( pos2 = pos1 ) != NULL;){
		list.GetNext(pos1);
		CUpDownClient* cur_client =	list.GetAt(pos2);
		list.RemoveAt(pos2);
		delete cur_client; // recursiv: this will call RemoveClient
	}
}

bool CClientList::AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender){
	POSITION pos1, pos2;
	CUpDownClient* tocheck = (*client);
	CUpDownClient* found_client = NULL;
	CUpDownClient* found_client2 = NULL;
	for (pos1 = list.GetHeadPosition();( pos2 = pos1 ) != NULL;){	//
		list.GetNext(pos1);
		CUpDownClient* cur_client =	list.GetAt(pos2);
		if (tocheck->Compare(cur_client,false)){ //matching userhash
			found_client2 = cur_client;
		}
		if (tocheck->Compare(cur_client,true)){	 //matching IP
			found_client = cur_client;
			break;
		}
	}
	if (found_client == NULL)
		found_client = found_client2;

	if (found_client != NULL){
		if (tocheck == found_client){
			//we found the same client instance (client may have sent more than one OP_HELLO). do not delete that client!
			return true;
		}
		if (sender){
			if (found_client->socket){
				if (found_client->socket->IsConnected() 
					&& (found_client->GetIP() != tocheck->GetIP() || found_client->GetUserPort() != tocheck->GetUserPort() ) )
				{
					// if found_client is connected and has the IS_IDENTIFIED, it's safe to say that the other one is a bad guy
					if (found_client->Credits() && found_client->Credits()->GetCurrentIdentState(found_client->GetIP()) == IS_IDENTIFIED){
						if (thePrefs.GetLogBannedClients())
							AddDebugLogLine(false, "Clients: %s (%s), Banreason: Userhash invalid", tocheck->GetUserName(), ipstr(tocheck->GetConnectIP()));
						tocheck->Ban();
						return false;
					}
	
					//IDS_CLIENTCOL Warning: Found matching client, to a currently connected client: %s (%s) and %s (%s)
					if (thePrefs.GetLogBannedClients())
						AddDebugLogLine(true,GetResString(IDS_CLIENTCOL), tocheck->GetUserName(), ipstr(tocheck->GetConnectIP()), found_client->GetUserName(), ipstr(found_client->GetConnectIP()));
					return false;
				}
				found_client->socket->client = 0;
				found_client->socket->Safe_Delete();
			}
			found_client->socket = sender;
			tocheck->socket = 0;
		}
		*client = 0;
		delete tocheck;
		*client = found_client;
		return true;
	}
	return false;
}

CUpDownClient* CClientList::FindClientByIP(uint32 clientip, UINT port) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip && cur_client->GetUserPort() == port)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByUserHash(const uchar* clienthash) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (!md4cmp(cur_client->GetUserHash() ,clienthash))
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP(uint32 clientip) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP_UDP(uint32 clientip, UINT nUDPport) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == clientip && cur_client->GetUDPPort() == nUDPport)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByUserID_KadPort(uint32 clientID, uint16 kadPort) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetUserIDHybrid() == clientID && cur_client->GetKadPort() == kadPort)
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByIP_KadPort(uint32 ip, uint16 port) const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client = list.GetNext(pos);
		if (cur_client->GetIP() == ip && cur_client->GetKadPort() == port)
			return cur_client;
	}
	return 0;
}

//TODO: This needs to change to a random Kad user.
CUpDownClient* CClientList::GetRandomKadClient() const
{
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client =	list.GetNext(pos);
		if (cur_client->GetKadPort())
			return cur_client;
	}
	return 0;
}

CUpDownClient* CClientList::FindClientByServerID(uint32 uServerIP, uint32 uED2KUserID) const
{
	uint32 uHybridUserID = ntohl(uED2KUserID);
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_client =	list.GetNext(pos);
		if (cur_client->GetServerIP() == uServerIP && cur_client->GetUserIDHybrid() == uHybridUserID)
			return cur_client;
	}
	return 0;
}

void CClientList::AddBannedClient(uint32 dwIP){
	m_bannedList.SetAt(dwIP, ::GetTickCount());
}

bool CClientList::IsBannedClient(uint32 dwIP){
	uint32 dwBantime = 0;
	if (m_bannedList.Lookup(dwIP, dwBantime)){
		if (dwBantime + CLIENTBANTIME > ::GetTickCount() )
			return true;
		else
			RemoveBannedClient(dwIP);
	}
	return false; 
}

void CClientList::RemoveBannedClient(uint32 dwIP){
	m_bannedList.RemoveKey(dwIP);
}

void CClientList::AddTrackClient(CUpDownClient* toadd){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(toadd->GetIP(), pResult)){
		pResult->m_dwInserted = ::GetTickCount();
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == toadd->GetUserPort()){
				// already tracked, update
				pResult->m_ItemsList[i].pHash = toadd->Credits();
				return;
			}
		}
		PORTANDHASH porthash = { toadd->GetUserPort(), toadd->Credits()};
		pResult->m_ItemsList.Add(porthash);
	}
	else{
		m_trackedClientsList.SetAt(toadd->GetIP(), new CDeletedClient(toadd));
	}
}

// true = everything ok, hash didn't changed
// false = hash changed
bool CClientList::ComparePriorUserhash(uint32 dwIP, uint16 nPort, void* pNewHash){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(dwIP, pResult)){
		for (int i = 0; i != pResult->m_ItemsList.GetCount(); i++){
			if (pResult->m_ItemsList[i].nPort == nPort){
				if (pResult->m_ItemsList[i].pHash != pNewHash)
					return false;
				else
					break;
			}
		}
	}
	return true;
}

uint16 CClientList::GetClientsFromIP(uint32 dwIP){
	CDeletedClient* pResult = 0;
	if (m_trackedClientsList.Lookup(dwIP, pResult)){
		return pResult->m_ItemsList.GetCount();
	}
	return 0;
}

void CClientList::Process(){
	const uint32 cur_tick = ::GetTickCount();
	if (m_dwLastBannCleanUp + BAN_CLEANUP_TIME < cur_tick){
		m_dwLastBannCleanUp = cur_tick;
		
		POSITION pos = m_bannedList.GetStartPosition();
		uint32 nKey;
		uint32 dwBantime;
		while (pos != NULL){
			m_bannedList.GetNextAssoc( pos, nKey, dwBantime );
			if (dwBantime + CLIENTBANTIME < cur_tick )
				RemoveBannedClient(nKey);
		}
	}

	
	if (m_dwLastTrackedCleanUp + TRACKED_CLEANUP_TIME < cur_tick ){
		m_dwLastTrackedCleanUp = cur_tick;
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, "Cleaning up TrackedClientList, %i clients on List...", m_trackedClientsList.GetCount());
		POSITION pos = m_trackedClientsList.GetStartPosition();
		uint32 nKey;
		CDeletedClient* pResult;
		while (pos != NULL){
			m_trackedClientsList.GetNextAssoc( pos, nKey, pResult );
			if (pResult->m_dwInserted + KEEPTRACK_TIME < cur_tick ){
				m_trackedClientsList.RemoveKey(nKey);
				delete pResult;
			}
		}
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, "...done, %i clients left on list", m_trackedClientsList.GetCount());
	}

	//We need to try to connect to the clients in RequestTCPList
	//If connected, remove them from the list and send a message back to Kad so we can send a ACK.
	//If we don't connect, we need to remove the client..
	//The sockets timeout should delete this object.
	POSITION pos1, pos2;
	for (pos1 = RequestTCPList.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		RequestTCPList.GetNext(pos1);
		CUpDownClient* cur_client =	RequestTCPList.GetAt(pos2);
		switch(cur_client->GetKadState())
		{
			case KS_QUEUED_FWCHECK:
				//I removed the self check here because we already did it when we added the client object.
				cur_client->TryToConnect();
				break;

			case KS_CONNECTING_FWCHECK:
				//Ignore this state as we are just waiting for results.
				break;

			case KS_CONNECTED_FWCHECK:
				//Send the Kademlia client a TCP connection ack!
				Kademlia::CKademlia::getUDPListener()->sendNullPacket(KADEMLIA_FIREWALLED_ACK, ntohl(cur_client->GetIP()), cur_client->GetKadPort());
				cur_client->SetKadState(KS_NONE);
				break;
			case KS_QUEUED_BUDDY:
				//a client lowID client wanting to be in the Kad network
				break;
			case KS_CONNECTED_BUDDY:
				//a client lowID client wanting to be in the Kad network
				//Not working at the moment so just set to none..
				cur_client->SetKadState(KS_NONE);
			default:
				RemoveTCP(cur_client);
		}
	}
}

#ifdef _DEBUG
void CClientList::Debug_SocketDeleted(CClientReqSocket* deleted){
	for (POSITION pos = list.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_client =	list.GetNext(pos);
		if (!AfxIsValidAddress(cur_client, sizeof(CUpDownClient))) {
			AfxDebugBreak();
		}
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		if (cur_client->socket == deleted){
			AfxDebugBreak();
		}
	}
}
#endif

bool CClientList::IsValidClient(CUpDownClient* tocheck)
{
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(tocheck);
	return list.Find(tocheck);
}

void CClientList::RequestTCP(Kademlia::CContact* contact)
{
	uint32 nContactIP = ntohl(contact->getIPAddress());
	//If eMule already knows this client, abort this.. It could cause conflicts.
	//Although the odds of this happening is very small, it could still happen.
	if (FindClientByIP(nContactIP, contact->getTCPPort()))
	{
		return;
	}

	// don't connect ourself
	if (theApp.serverconnect->GetLocalIP() == nContactIP && thePrefs.GetPort() == contact->getTCPPort())
		return;

	//Add client to the lists to be processed.
	CUpDownClient* test = new CUpDownClient(0, contact->getTCPPort(), contact->getIPAddress(), 0, 0, false );
	test->SetKadPort(contact->getUDPPort());
	test->SetKadState(KS_QUEUED_FWCHECK);
	RequestTCPList.AddTail(test);
	AddClient(test);
}

void CClientList::RemoveTCP(CUpDownClient* torem){
	POSITION pos = RequestTCPList.Find(torem);
	if(pos)
	{
		RequestTCPList.RemoveAt(pos);
	}
}

CDeletedClient::CDeletedClient(CUpDownClient* pClient)
{
	m_dwInserted = ::GetTickCount();
	PORTANDHASH porthash = { pClient->GetUserPort(), pClient->Credits()};
	m_ItemsList.Add(porthash);
}