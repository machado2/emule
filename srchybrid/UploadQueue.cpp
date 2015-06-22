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
#include "UploadQueue.h"
#include "Packets.h"
#include "KnownFile.h"
#include "ListenSocket.h"
#include "Exceptions.h"
#include "Scheduler.h"
#include "PerfLog.h"
#include "UploadBandwidthThrottler.h"
#include "ClientList.h"
#include "LastCommonRouteFinder.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "Statistics.h"
#include "MMServer.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Sockets.h"
#include "ClientCredits.h"
#include "Server.h"
#include "ServerList.h"
#include "WebServer.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "SearchDlg.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static uint32 counter, sec, statsave;
static UINT _uSaveStatistics = 0;
// -khaos--+++> Added iupdateconnstats...
static uint32 igraph, istats, iupdateconnstats;
// <-----khaos-

//TODO rewrite the whole networkcode, use overlapped sockets.. sure....

CUploadQueue::CUploadQueue()
{
	VERIFY( (h_timer = SetTimer(0,0,100,UploadTimer)) != NULL );
	if (thePrefs.GetVerbose() && !h_timer)
		AddDebugLogLine(true,_T("Failed to create 'upload queue' timer - %s"),GetErrorMessage(GetLastError()));
	datarate = 0;
	dataratems = 0;
	counter=0;
	successfullupcount = 0;
	failedupcount = 0;
	totaluploadtime = 0;
	m_nLastStartUpload = 0;
	statsave=0;
	// -khaos--+++>
	iupdateconnstats=0;
	// <-----khaos-
	m_bRemovedClientByScore = false;
}

void CUploadQueue::AddUpNextClient(CUpDownClient* directadd){
	POSITION toadd = 0;
	POSITION toaddlow = 0;
	uint32	bestscore = 0;
	uint32  bestlowscore = 0;
	CUpDownClient* newclient;
	// select next client or use given client
	if (!directadd)
	{
		POSITION pos1, pos2;
		for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
		{
			waitinglist.GetNext(pos1);
			CUpDownClient* cur_client =	waitinglist.GetAt(pos2);
			// clear dead clients
			ASSERT ( cur_client->GetLastUpRequest() );
			if ((::GetTickCount() - cur_client->GetLastUpRequest() > MAX_PURGEQUEUETIME) || !theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID()) )
			{
				cur_client->ClearWaitStartTime();
				RemoveFromWaitingQueue(pos2,true);
				continue;
			}
			// finished clearing
			uint32 cur_score = cur_client->GetScore(true);
			if ( cur_score > bestscore)
			{
				bestscore = cur_score;
				toadd = pos2;
			} 
			else 
			{
				cur_score = cur_client->GetScore(false);
				if ((cur_score > bestlowscore) && !cur_client->m_bAddNextConnect)
				{
					bestlowscore = cur_score;
					toaddlow = pos2;
				}
			}
		}
		
		if (bestlowscore > bestscore)
		{
			newclient = waitinglist.GetAt(toaddlow);
			newclient->m_bAddNextConnect = true;
		}
		if (!toadd)
			return;
		newclient = waitinglist.GetAt(toadd);
		lastupslotHighID = true; // VQB LowID alternate
		//AddLogLine(true,"Added High ID: %s", newclient->GetUserName()); // VQB:  perhaps only add to debug log?
		RemoveFromWaitingQueue(toadd, true);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
	}
	else 
	{
		newclient = directadd;
		/*if (!IsDownloading(newclient)){
            if (newclient->HasLowID())
                AddLogLine(true,"DirectAdd:  LowID: %s", newclient->GetUserName());
			else
                AddLogLine(true,"DirectAdd:  HighID: %s", newclient->GetUserName());
		}*/
	}
	if (!thePrefs.TransferFullChunks())
		UpdateMaxClientScore(); // refresh score caching, now that the highest score is removed

	if (IsDownloading(newclient))
	{
		return;
	}
	// tell the client that we are now ready to upload
	if (!newclient->socket || !newclient->socket->IsConnected())
	{
		newclient->SetUploadState(US_CONNECTING);
		if (!newclient->TryToConnect(true))
			return;
	}
	else
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", newclient);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		newclient->socket->SendPacket(packet,true);
		newclient->SetUploadState(US_UPLOADING);
	}
	newclient->SetUpStartTime();
	newclient->ResetSessionUp();

    theApp.uploadBandwidthThrottler->AddToStandardList(uploadinglist.GetCount(), newclient->socket);    
    uploadinglist.AddTail(newclient);
	
	// statistic
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)newclient->GetUploadFileID());
	if (reqfile){
		reqfile->statistic.AddAccepted();
	}
		
	theApp.emuledlg->transferwnd->uploadlistctrl.AddClient(newclient);
}

void CUploadQueue::Process() {

	theApp.sharedfiles->Publish();

	while ( avarage_dr_list.GetCount()>150)
		dataratems-=avarage_dr_list.RemoveHead().datalen;

	uint32 tick_avg = 100;
	if (avarage_dr_list.GetCount() >2) {
		uint32 deltat=(avarage_dr_list.GetTail().timestamp - avarage_dr_list.GetHead().timestamp);
		if (deltat>0) datarate = (1000i64*(dataratems-avarage_dr_list.GetHead().datalen)) / deltat;
		tick_avg = deltat / avarage_dr_list.GetCount();
	} else
		datarate = 0;



	if (AcceptNewClient() && waitinglist.GetCount()){
		m_nLastStartUpload = ::GetTickCount();
		AddUpNextClient();
	}

	if (!uploadinglist.GetCount()) {
		return;
	}

	m_bRemovedClientByScore = false;

	POSITION pos = uploadinglist.GetHeadPosition();
	while(pos != 0){
		CUpDownClient* cur_client = uploadinglist.GetNext(pos);
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_client);
		//It seems chatting or friend slots can get stuck at times in upload.. This needs looked into..
		if (!cur_client->socket)
		{
			RemoveFromUploadQueue(cur_client, _T("Uploading to client without socket? (CUploadQueue::Process)"));
			if(cur_client->Disconnected(_T("CUploadQueue::Process"))){
				delete cur_client;
			}
		} else {
            cur_client->SendBlockData();
        }
	}

	uint64 sentBytes = theApp.uploadBandwidthThrottler->GetNumberOfSentBytesSinceLastCallAndReset();

    TransferredData newitem = {sentBytes, ::GetTickCount()};  // theApp.stat_sessionSentBytes
	avarage_dr_list.AddTail(newitem);
	dataratems+=sentBytes;
};

bool CUploadQueue::AcceptNewClient(){
	// check if we can allow a new client to start downloading from us
	if (::GetTickCount() - m_nLastStartUpload < 1000 && datarate < 102400 )
		return false;

	uint16 MaxSpeed;

    if (thePrefs.IsDynUpEnabled())
        MaxSpeed = theApp.lastCommonRouteFinder->GetUpload()/1024;        
    else
		MaxSpeed = thePrefs.GetMaxUpload();
	
	uint32 upPerClient = UPLOAD_CLIENT_DATARATE;
	uint32 curUploadSlots = (uint32)uploadinglist.GetCount();

	if (curUploadSlots < MIN_UP_CLIENTS_ALLOWED)
		return true;
	else if (curUploadSlots >= MAX_UP_CLIENTS_ALLOWED ||
             curUploadSlots >= 4 &&
             (
              curUploadSlots >= (datarate/UPLOAD_CHECK_CLIENT_DR) ||
              curUploadSlots >= ((uint32)MaxSpeed)*1024/UPLOAD_CLIENT_DATARATE ||
              !theApp.lastCommonRouteFinder->AcceptNewClient() || // upload speed sense can veto a new slot if USS enabled
              (
               thePrefs.GetMaxUpload() == UNLIMITED &&
               !thePrefs.IsDynUpEnabled() &&
               thePrefs.GetMaxGraphUploadRate() > 0 &&
               curUploadSlots >= ((uint32)thePrefs.GetMaxGraphUploadRate())*1024/UPLOAD_CLIENT_DATARATE
              )
             )
            ) // max number of clients to allow for all circumstances
		return false;

    if(theApp.uploadBandwidthThrottler->GetHighestNumberOfFullyActivatedSlotsSinceLastCallAndReset() > (uint32)uploadinglist.GetSize()) {
        // uploadThrottler requests another slot. If throttler says it needs another slot, we will allow more slots
        // than what we require ourself. Never allow more slots than to give each slot high enough average transfer speed, though (checked above).
        return true;
    }

    // if throttler doesn't require another slot, go with a slightly more restrictive method
	if( MaxSpeed > 20 || MaxSpeed == UNLIMITED)
		upPerClient += datarate/43;

	if( upPerClient > 7680 )
		upPerClient = 7680;

	//now the final check

	if ( MaxSpeed == UNLIMITED )
	{
		if (curUploadSlots < (datarate/upPerClient))
			return true;
	}
	else{
		uint16 nMaxSlots;
		if (MaxSpeed > 12)
			nMaxSlots = (uint16)(((float)(MaxSpeed*1024)) / upPerClient);
		else if (MaxSpeed > 7)
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 2;
		else if (MaxSpeed > 3)
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED + 1;
		else
			nMaxSlots = MIN_UP_CLIENTS_ALLOWED;
//		AddLogLine(true,"maxslots=%u, upPerClient=%u, datarateslot=%u|%u|%u",nMaxSlots,upPerClient,datarate/UPLOAD_CHECK_CLIENT_DR, datarate, UPLOAD_CHECK_CLIENT_DR);

		if ( curUploadSlots < nMaxSlots )
		{
			return true;
		}
	}
	//nope
	return false;
}
    
CUploadQueue::~CUploadQueue(){
	if (h_timer)
		KillTimer(0,h_timer);
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP_UDP(uint32 dwIP, uint16 nUDPPort){
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort())
			return cur_client;
	}
	return 0;
}

CUpDownClient* CUploadQueue::GetWaitingClientByIP(uint32 dwIP){
	for (POSITION pos = waitinglist.GetHeadPosition();pos != 0;){
		CUpDownClient* cur_client = waitinglist.GetNext(pos);
		if (dwIP == cur_client->GetIP())
			return cur_client;
	}
	return 0;
}

void CUploadQueue::AddClientToQueue(CUpDownClient* client, bool bIgnoreTimelimit)
{
	if (theApp.serverconnect->IsConnected() && theApp.serverconnect->IsLowID() //This may need to be changed with the Kad now being used.
		&& !theApp.serverconnect->IsLocalServer(client->GetServerIP(),client->GetServerPort())
		&& client->GetDownloadState() == DS_NONE && !client->IsFriend()
		&& GetWaitingUserCount() > 50)
		return;
	client->AddAskedCount();
	client->SetLastUpRequest();
	if (!bIgnoreTimelimit)
	{
		client->AddRequestCount(client->GetUploadFileID());
	}
	if (client->IsBanned())
		return;
	uint16 cSameIP = 0;
	// check for double
	POSITION pos1, pos2;
	for (pos1 = waitinglist.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		waitinglist.GetNext(pos1);
		CUpDownClient* cur_client= waitinglist.GetAt(pos2);
		if (cur_client == client)
		{	
			//already on queue
			// VQB LowID Slot Patch -- note:  should add limit so only if #slots < UL -or- UL+1 for Low UL (?)
			if (client->m_bAddNextConnect && (uploadinglist.GetCount() < thePrefs.GetMaxUpload()))
			{
				if (lastupslotHighID) 
				{
					client->m_bAddNextConnect = false;
					RemoveFromWaitingQueue(client, true);
					AddUpNextClient(client);
					lastupslotHighID = false; // LowID alternate
					return;
				}
			}
			// VQB end
			client->SendRankingInfo();
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(client);
			return;			
		}
		else if ( client->Compare(cur_client) ) 
		{
			theApp.clientlist->AddTrackClient(client); // in any case keep track of this client

			// another client with same ip:port or hash
			// this happens only in rare cases, because same userhash / ip:ports are assigned to the right client on connecting in most cases
			if (cur_client->credits != NULL && cur_client->credits->GetCurrentIdentState(cur_client->GetIP()) == IS_IDENTIFIED)
			{
				//cur_client has a valid secure hash, don't remove him
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),client->GetUserName() );
				return;
			}
			if (client->credits != NULL && client->credits->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED)
			{
				//client has a valid secure hash, add him remove other one
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),cur_client->GetUserName() );
				RemoveFromWaitingQueue(pos2,true);
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 1")))
					{
						delete cur_client;
					}
				}
			}
			else
			{
				// remove both since we dont know who the bad on is
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false,CString(GetResString(IDS_SAMEUSERHASH)),client->GetUserName(),cur_client->GetUserName(),"Both" );
				RemoveFromWaitingQueue(pos2,true);	
				if (!cur_client->socket)
				{
					if(cur_client->Disconnected(_T("AddClientToQueue - same userhash 2")))
					{
						delete cur_client;
					}
				}
				return;
			}
		}
		else if (client->GetIP() == cur_client->GetIP())
		{
			// same IP, different port, different userhash
			cSameIP++;
		}
	}
	if (cSameIP >= 3)
	{
		// do not accept more than 3 clients from the same IP
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	else if (theApp.clientlist->GetClientsFromIP(client->GetIP()) >= 3)
	{
		if (thePrefs.GetVerbose())
			DEBUG_ONLY( AddDebugLogLine(false,_T("%s's (%s) request to enter the queue was rejected, because of too many clients with the same IP (found in TrackedClientsList)"), client->GetUserName(), ipstr(client->GetConnectIP())) );
		return;
	}
	// done

	// Add clients server to list.
	if (thePrefs.AddServersFromClient() && client->GetServerIP() && client->GetServerPort())
	{
		CServer* srv = new CServer(client->GetServerPort(), ipstr(client->GetServerIP()));
		srv->SetListName(srv->GetAddress());

		if (!theApp.emuledlg->serverwnd->serverlistctrl.AddServer(srv, true))
			delete srv;
	}

	// statistic values
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID((uchar*)client->GetUploadFileID());
	if (reqfile)
		reqfile->statistic.AddRequest();
	// TODO find better ways to cap the list
	if ((uint32)waitinglist.GetCount() > thePrefs.GetQueueSize())
	{
		return;
	}
	if (client->IsDownloading())
	{
		// he's already downloading and wants probably only another file
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__AcceptUploadReq", client);
		Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
		theStats.AddUpDataOverheadFileRequest(packet->size);
		client->socket->SendPacket(packet,true);
		return;
	}
	if (waitinglist.IsEmpty() && AcceptNewClient())
	{
		AddUpNextClient(client);
		m_nLastStartUpload = ::GetTickCount();
	}
	else
	{
		waitinglist.AddTail(client);
		client->SetUploadState(US_ONUPLOADQUEUE);
		theApp.emuledlg->transferwnd->queuelistctrl.AddClient(client,true);
		theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		client->SendRankingInfo();
	}
}

bool CUploadQueue::RemoveFromUploadQueue(CUpDownClient* client, LPCTSTR pszReason, bool updatewindow, bool earlyabort){
	for (POSITION pos = uploadinglist.GetHeadPosition();pos != 0;uploadinglist.GetNext(pos)){
		if (client == uploadinglist.GetAt(pos)){
			if (updatewindow)
				theApp.emuledlg->transferwnd->uploadlistctrl.RemoveClient(uploadinglist.GetAt(pos));

			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_VERYLOW, true,_T("---- %s: Removing client from upload list. Reason: %s ----"), client->GetUserName(), pszReason==NULL ? _T("") : pszReason);
            uploadinglist.RemoveAt(pos);
            theApp.uploadBandwidthThrottler->RemoveFromStandardList(client->socket);

			if(client->GetSessionUp() > 0) {
				++successfullupcount;
				totaluploadtime += client->GetUpStartTimeDelay()/1000;
            } else if(earlyabort == false)
				++failedupcount;

            CKnownFile* requestedFile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());

            if(requestedFile != NULL) {
                requestedFile->UpdatePartsInfo();
            }
			theApp.clientlist->AddTrackClient(client); // Keep track of this client
			client->SetUploadState(US_NONE);
			client->ClearUploadBlockRequests();
			return true;
		}
	}
	return false;
}

uint32 CUploadQueue::GetAverageUpTime(){
	if( successfullupcount ){
		return totaluploadtime/successfullupcount;
	}
	return 0;
}

bool CUploadQueue::RemoveFromWaitingQueue(CUpDownClient* client, bool updatewindow){
	POSITION pos = waitinglist.Find(client);
	if (pos){
		RemoveFromWaitingQueue(pos,updatewindow);
		if (updatewindow)
			theApp.emuledlg->transferwnd->ShowQueueCount(waitinglist.GetCount());
		return true;
	}
	else
		return false;
}

void CUploadQueue::RemoveFromWaitingQueue(POSITION pos, bool updatewindow){	
	CUpDownClient* todelete = waitinglist.GetAt(pos);
	waitinglist.RemoveAt(pos);
	if (updatewindow)
		theApp.emuledlg->transferwnd->queuelistctrl.RemoveClient(todelete);
	todelete->SetUploadState(US_NONE);
}

void CUploadQueue::UpdateMaxClientScore()
{
	m_imaxscore=0;
	for(POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ) {
		uint32 score = waitinglist.GetNext(pos)->GetScore(true, false);
		if(score > m_imaxscore )
			m_imaxscore=score;
	}
}

bool CUploadQueue::CheckForTimeOver(CUpDownClient* client){
	if (!thePrefs.TransferFullChunks()){
	    if( client->GetUpStartTimeDelay() > SESSIONMAXTIME){ // Try to keep the clients from downloading for ever
		    if (thePrefs.GetLogUlDlEvents())
			    AddDebugLogLine(DLP_LOW, false, _T("%s: Upload session ended due to max time %s."), client->GetUserName(), CastSecondsToHM(SESSIONMAXTIME/1000));
		    return true;
	    }

		// Cache current client score
		const uint32 score = client->GetScore(true, true);

		// Check if another client has a bigger score
		if (score < GetMaxClientScore() && !m_bRemovedClientByScore) {
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_VERYLOW, false, _T("%s: Upload session ended due to score."), client->GetUserName());
			m_bRemovedClientByScore = true; //makes sure only one client gets removed per round, so we the best score can be recalculated after the next has its downloadslot
			return true;
		}
	}
	else{
		// Allow the client to download a specified amount per session
		if( client->GetQueueSessionPayloadUp() > SESSIONMAXTRANS ){
			if (thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_DEFAULT, false, _T("%s: Upload session ended due to max transfered amount. %s"), client->GetUserName(), CastItoXBytes(SESSIONMAXTRANS));
			return true;
		}
	}
	return false;
}

void CUploadQueue::DeleteAll(){
	waitinglist.RemoveAll();
	uploadinglist.RemoveAll();
    // PENDING: Remove from UploadBandwidthThrottler as well!
}

uint16 CUploadQueue::GetWaitingPosition(CUpDownClient* client)
{
	if (!IsOnUploadQueue(client))
		return 0;
	UINT rank = 1;
	uint32 myscore = client->GetScore(false);
	for (POSITION pos = waitinglist.GetHeadPosition(); pos != 0; ){
		if (waitinglist.GetNext(pos)->GetScore(false) > myscore)
			rank++;
	}
	return rank;
}

VOID CALLBACK CUploadQueue::UploadTimer(HWND hwnd, UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		// Barry - Don't do anything if the app is shutting down - can cause unhandled exceptions
		if (!theApp.emuledlg->IsRunning())
			return;

        // Elandal:ThreadSafeLogging -->
        // other threads may have queued up log lines. This prints them.
        theApp.HandleDebugLogQueue();
        theApp.HandleLogQueue();
        // Elandal: ThreadSafeLogging <--

        // Send allowed data rate to UploadBandWidthThrottler in a thread safe way
        theApp.uploadBandwidthThrottler->SetAllowedDataRate(thePrefs.GetMaxUpload()*1024);
		// ZZ:UploadSpeedSense -->
		theApp.lastCommonRouteFinder->SetPrefs(thePrefs.IsDynUpEnabled(), theApp.uploadqueue->GetDatarate(), thePrefs.GetMinUpload()*1024, thePrefs.GetMaxUpload()*1024, thePrefs.IsDynUpUseMillisecondPingTolerance(), (thePrefs.GetDynUpPingTolerance() > 100)?((thePrefs.GetDynUpPingTolerance()-100)/100.0f):0, thePrefs.GetDynUpPingToleranceMilliseconds(), thePrefs.GetDynUpGoingUpDivider(), thePrefs.GetDynUpGoingDownDivider(), thePrefs.GetDynUpNumberOfPings(), 20); // PENDING: Hard coded min pLowestPingAllowed
		// ZZ:UploadSpeedSense <--

        theApp.uploadqueue->Process();
		theApp.downloadqueue->Process();
		if (thePrefs.ShowOverhead()){
			theStats.CompUpDatarateOverhead();
			theStats.CompDownDatarateOverhead();
		}
		counter++;

		// one second
		if (counter >= 10){
			counter=0;

			// try to use different time intervals here to not create any disk-IO bottle necks by saving all files at once
			theApp.clientcredits->Process();	// 13 minutes
			theApp.serverlist->Process();		// 17 minutes
			theApp.knownfiles->Process();		// 11 minutes
			theApp.friendlist->Process();		// 19 minutes
			theApp.clientlist->Process();
			theApp.sharedfiles->Process();
			Kademlia::CKademlia::process();

			if( theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsSingleConnect() )
				theApp.serverconnect->TryAnotherConnectionrequest();

			theApp.listensocket->UpdateConnectionsStatus();
			if (thePrefs.WatchClipboard4ED2KLinks())
				theApp.SearchClipboard();		

			if (theApp.serverconnect->IsConnecting())
				theApp.serverconnect->CheckForTimeout();

			// -khaos--+++> Update connection stats...
			iupdateconnstats++;
			// 2 seconds
			if (iupdateconnstats>=2) {
				iupdateconnstats=0;
				theStats.UpdateConnectionStats((float)theApp.uploadqueue->GetDatarate()/1024, (float)theApp.downloadqueue->GetDatarate()/1024);
			}
			// <-----khaos-

			// display graphs
			if (thePrefs.GetTrafficOMeterInterval()>0) {
				igraph++;

				if (igraph >= (uint32)(thePrefs.GetTrafficOMeterInterval()) ) {
					igraph=0;
					//theApp.emuledlg->statisticswnd->SetCurrentRate((float)(theApp.uploadqueue->Getavgupload()/theApp.uploadqueue->Getavg())/1024,(float)(theApp.uploadqueue->Getavgdownload()/theApp.uploadqueue->Getavg())/1024);
					theApp.emuledlg->statisticswnd->SetCurrentRate((float)(theApp.uploadqueue->GetDatarate())/1024,(float)(theApp.downloadqueue->GetDatarate())/1024);
					//theApp.uploadqueue->Zeroavg();
				}
			}
			if (theApp.emuledlg->activewnd == theApp.emuledlg->statisticswnd && theApp.emuledlg->IsWindowVisible() )  {
				// display stats
				if (thePrefs.GetStatsInterval()>0) {
					istats++;

					if (istats >= (uint32)(thePrefs.GetStatsInterval()) ) {
						istats=0;
						theApp.emuledlg->statisticswnd->ShowStatistics();
					}
				}
			}
			//save rates every second
			theStats.RecordRate();
			// mobilemule sockets
			theApp.mmserver->Process();

			// ZZ:UploadSpeedSense -->
            theApp.emuledlg->ShowPing();

            bool gotEnoughHosts = theApp.clientlist->GiveClientsForTraceRoute();
            if(gotEnoughHosts == false) {
                theApp.serverlist->GiveServersForTraceRoute();
            }
			// ZZ:UploadSpeedSense <--

            sec++;
			// 5 seconds
			if (sec>=5) {
#ifdef _DEBUG
				if (thePrefs.m_iDbgHeap > 0 && !AfxCheckMemory())
					AfxDebugBreak();
#endif

				sec = 0;
				theApp.listensocket->Process();
				theApp.OnlineSig(); // Added By Bouc7 
				theApp.emuledlg->ShowTransferRate();
				
				if (!thePrefs.TransferFullChunks())
					theApp.uploadqueue->UpdateMaxClientScore();

				// update cat-titles with downloadinfos only when needed
				if (thePrefs.ShowCatTabInfos() && 
					theApp.emuledlg->activewnd == theApp.emuledlg->transferwnd && 
					theApp.emuledlg->IsWindowVisible()) 
						theApp.emuledlg->transferwnd->UpdateCatTabTitles(false);
				
				if (thePrefs.IsSchedulerEnabled())
					theApp.scheduler->Check();
			}

			statsave++;
			// 60 seconds
			if (statsave>=60) {
				statsave=0;

				if (thePrefs.GetWSIsEnabled())
					theApp.webserver->UpdateSessionCount();

				theApp.serverconnect->KeepConnectionAlive();
			}

			_uSaveStatistics++;
			if (_uSaveStatistics >= thePrefs.GetStatsSaveInterval())
			{
				_uSaveStatistics = 0;
				thePrefs.SaveStats();
			}
		}

		// need more accuracy here. don't rely on the 'sec' and 'statsave' helpers.
		thePerfLog.LogSamples();
	}
	CATCH_DFLT_EXCEPTIONS(_T("CUploadQueue::UploadTimer"))
}

CUpDownClient* CUploadQueue::GetNextClient(const CUpDownClient* lastclient){
	if (waitinglist.IsEmpty())
		return 0;
	if (!lastclient)
		return waitinglist.GetHead();
	POSITION pos = waitinglist.Find(const_cast<CUpDownClient*>(lastclient));
	if (!pos){
		TRACE("Error: CUploadQueue::GetNextClient");
		return waitinglist.GetHead();
	}
	waitinglist.GetNext(pos);
	if (!pos)
		return NULL;
	else
		return waitinglist.GetAt(pos);
}

/*void CUploadQueue::FindSourcesForFileById(CUpDownClientPtrList* srclist, const uchar* filehash) {
	POSITION pos;
	pos = uploadinglist.GetHeadPosition();
	while(pos) 
	{
		CUpDownClient *potential = uploadinglist.GetNext(pos);
		if(md4cmp(potential->GetUploadFileID(), filehash) == 0)
			srclist->AddTail(potential);
	}

	pos = waitinglist.GetHeadPosition();
	while(pos) 
	{
		CUpDownClient *potential = waitinglist.GetNext(pos);
		if(md4cmp(potential->GetUploadFileID(), filehash) == 0)
			srclist->AddTail(potential);
	}
}
*/