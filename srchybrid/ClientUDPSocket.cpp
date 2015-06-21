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
#include "ClientUDPSocket.h"
#include "Packets.h"
#include "DownloadQueue.h"
#include "PartFile.h"
#include "SharedFileList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "ClientList.h"
#include <zlib/zlib.h>
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "kademlia/io/IOException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CClientUDPSocket

CClientUDPSocket::CClientUDPSocket()
{
	m_bWouldBlock = false;
}

CClientUDPSocket::~CClientUDPSocket()
{
	POSITION pos = controlpacket_queue.GetHeadPosition();
	while (pos){
		UDPPack* p = controlpacket_queue.GetNext(pos);
		delete p->packet;
		delete p;
	}
}

void CClientUDPSocket::OnReceive(int nErrorCode)
{
	if (nErrorCode)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Client UDP socket, error on receive event: %s"), GetErrorMessage(nErrorCode, 1));
	}

	BYTE buffer[5000];
	SOCKADDR_IN sockAddr = {0};
	int iSockAddrLen = sizeof sockAddr;
	int length = ReceiveFrom(buffer, sizeof buffer, (SOCKADDR*)&sockAddr, &iSockAddrLen);
	if (length >= 1)
    {
		CString strError;
		try
		{
			switch (buffer[0])
			{
				case OP_EMULEPROT:
				{
					if (length >= 2)
						ProcessPacket(buffer+2, length-2, buffer[1], sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
					else
						throw CString("Packet too short");
					break;
				}
				case OP_KADEMLIAPACKEDPROT:
				{
					theApp.downloadqueue->AddDownDataOverheadKad(length);
					if (length >= 2)
					{
						uint32 nNewSize = length*10+300;
						byte* unpack = new byte[nNewSize];
						uLongf unpackedsize = nNewSize-2;
						uint16 result = uncompress(unpack+2, &unpackedsize, buffer+2, length-2);
						if (result == Z_OK)
						{
							unpack[0] = OP_KADEMLIAHEADER;
							unpack[1] = buffer[1];
							try
							{
								Kademlia::CKademlia::processPacket(unpack, unpackedsize+2, ntohl(sockAddr.sin_addr.S_un.S_addr), ntohs(sockAddr.sin_port));
							}
							catch(...)
							{
								delete[] unpack;
								throw;
							}
						}
						else
						{
							delete[] unpack;
							throw CString("Failed to uncompress Kademlia packet");
						}
						delete[] unpack;
					}
					else
						throw CString("Packet too short");
					break;
				}
				case OP_KADEMLIAHEADER:
				{
					theApp.downloadqueue->AddDownDataOverheadKad(length);
					if (length >= 2)
						Kademlia::CKademlia::processPacket(buffer, length, ntohl(sockAddr.sin_addr.S_un.S_addr), ntohs(sockAddr.sin_port));
					else
						throw CString("Packet too short");
					break;
				}
				default:
				{
					CString strError;
					strError.Format("Unknown protocol %02x", buffer[0]);
					throw strError;
				}
			}
		}
		catch(CFileException* error)
		{
			error->Delete();
			strError = "Invalid packet received";
		}
		catch(CMemoryException* error)
		{
			error->Delete();
			strError = "Memory exception";
		}
		catch(CString error)
		{
			strError = error;
		}
		catch(Kademlia::CIOException* error)
		{
			error->Delete();
			strError = "Invalid packet received";
		}
		catch(...)
		{
			strError = "Unknown exception";
			ASSERT(0);
		}

		if (thePrefs.GetVerbose() && !strError.IsEmpty())
		{
			CString strClientInfo;
			CUpDownClient* client;
			if (buffer[0] == OP_EMULEPROT)
				client = theApp.clientlist->FindClientByIP_UDP(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
			else
				client = theApp.clientlist->FindClientByIP_KadPort(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
			if (client)
				strClientInfo = client->DbgGetClientInfo();
			else
				strClientInfo.Format("%s:%u", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

			AddDebugLogLine(false, "Client UDP socket: prot=%02x  opcode=%02x  %s: %s", buffer[0], buffer[1], strError, strClientInfo);
		}
    }
	else if (length == SOCKET_ERROR)
	{
		if (thePrefs.GetVerbose())
		{
			DWORD dwError = WSAGetLastError();
			CString strClientInfo;
			if (iSockAddrLen > 0 && sockAddr.sin_addr.S_un.S_addr != 0 && sockAddr.sin_addr.S_un.S_addr != INADDR_NONE)
				strClientInfo.Format(" from %s:%u", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
			AddDebugLogLine(false, _T("Error: Client UDP socket, failed to receive data%s: %s"), strClientInfo, GetErrorMessage(dwError, 1));
		}
	}
}

bool CClientUDPSocket::ProcessPacket(BYTE* packet, uint16 size, uint8 opcode, uint32 ip, uint16 port)
{
	switch(opcode)
	{
		case OP_REASKFILEPING:
		{
			theApp.downloadqueue->AddDownDataOverheadFileRequest(size);
			CSafeMemFile data_in(packet, size);
			uchar reqfilehash[16];
			data_in.ReadHash16(reqfilehash);
			CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(reqfilehash);
			if (!reqfile)
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0)
				{
					DebugRecv("OP_ReaskFilePing", NULL, (char*)reqfilehash, ip);
					DebugSend("OP__FileNotFound", NULL);
				}

				Packet* response = new Packet(OP_FILENOTFOUND,0,OP_EMULEPROT);
				theApp.uploadqueue->AddUpDataOverheadFileRequest(response->size);
				SendPacket(response, ip, port);
				break;
			}
			CUpDownClient* sender = theApp.uploadqueue->GetWaitingClientByIP_UDP(ip, port);
			if (sender)
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", sender, (char*)reqfilehash, ip);

				//Make sure we are still thinking about the same file
				if (md4cmp(reqfilehash, sender->GetUploadFileID()) == 0)
				{
					sender->AddAskedCount();
					sender->SetLastUpRequest();
					//I messed up when I first added extended info to UDP
					//I should have originally used the entire ProcessExtenedInfo the first time.
					//So now I am forced to check UDPVersion to see if we are sending all the extended info.
					//For now on, we should not have to change anything here if we change
					//anything to the extended info data as this will be taken care of in ProcessExtendedInfo()
					//Update extended info. 
					if (sender->GetUDPVersion() > 3)
					{
						sender->ProcessExtendedInfo(&data_in, reqfile);
					}
					//Update our complete source counts.
					else if (sender->GetUDPVersion() > 2)
					{
						uint16 nCompleteCountLast= sender->GetUpCompleteSourcesCount();
						uint16 nCompleteCountNew = data_in.ReadUInt16();
						sender->SetUpCompleteSourcesCount(nCompleteCountNew);
						if (nCompleteCountLast != nCompleteCountNew)
						{
							reqfile->UpdatePartsInfo();
						}
					}
					CSafeMemFile data_out(128);
					if(sender->GetUDPVersion() > 3)
					{
						if (reqfile->IsPartFile())
							((CPartFile*)reqfile)->WritePartStatus(&data_out);
						else
							data_out.WriteUInt16(0);
					}
					data_out.WriteUInt16(theApp.uploadqueue->GetWaitingPosition(sender));
					if (thePrefs.GetDebugClientUDPLevel() > 0)
						DebugSend("OP__ReaskAck", sender);
					Packet* response = new Packet(&data_out, OP_EMULEPROT);
					response->opcode = OP_REASKACK;
					theApp.uploadqueue->AddUpDataOverheadFileRequest(response->size);
					theApp.clientudp->SendPacket(response, ip, port);
				}
				else
				{
					AddDebugLogLine(false, "Client UDP socket; ReaskFilePing; reqfile does not match");
					TRACE("reqfile:         %s\n", DbgGetFileInfo(reqfile->GetFileHash()));
					TRACE("sender->reqfile: %s\n", sender->reqfile ? DbgGetFileInfo(sender->reqfile->GetFileHash()) : "(null)");
				}
			}
			else
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", NULL, (char*)reqfilehash, ip);

				if (((uint32)theApp.uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
				{
					if (thePrefs.GetDebugClientUDPLevel() > 0)
						DebugSend("OP__QueueFull", NULL);
					Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
					theApp.uploadqueue->AddUpDataOverheadFileRequest(response->size);
					SendPacket(response, ip, port);
				}
			}
			break;
		}
		case OP_QUEUEFULL:
		{
			theApp.downloadqueue->AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_QueueFull", sender, NULL, ip);
			if (sender){
				sender->SetRemoteQueueFull(true);
				sender->UDPReaskACK(0);
			}
			break;
		}
		case OP_REASKACK:
		{
			theApp.downloadqueue->AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ReaskAck", sender, NULL, ip);
			if (sender){
				CSafeMemFile data_in((BYTE*)packet,size);
				if ( sender->GetUDPVersion() > 3 )
				{
					sender->ProcessFileStatus(true, &data_in, sender->reqfile);
				}
				uint16 nRank = data_in.ReadUInt16();
				sender->SetRemoteQueueFull(false);
				sender->UDPReaskACK(nRank);
				sender->AddAskedCountDown();
			}
			break;
		}
		case OP_FILENOTFOUND:
		{
			theApp.downloadqueue->AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_FileNotFound", sender, NULL, ip);
			if (sender){
				sender->UDPReaskFNF(); // may delete 'sender'!
				sender = NULL;
			}
			break;
		}
		default:
			theApp.downloadqueue->AddDownDataOverheadOther(size);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port);
				Debug("Unknown client UDP packet: host=%s:%u (%s) opcode=0x%02x  size=%u\n", inet_ntoa(*(in_addr*)&ip), port, sender ? (LPCSTR)sender->DbgGetClientInfo() : (LPCSTR)"", opcode, size);
			}
			return false;
	}
	return true;
}

void CClientUDPSocket::OnSend(int nErrorCode){
	if (nErrorCode){
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Client UDP socket, error on send event: %s"), GetErrorMessage(nErrorCode, 1));
		return;
	}
	m_bWouldBlock = false;
	while (controlpacket_queue.GetHeadPosition() != 0 && !IsBusy()){
		UDPPack* cur_packet = controlpacket_queue.GetHead();
		if( GetTickCount() - cur_packet->dwTime < UDPMAXQUEUETIME )
		{
			char* sendbuffer = new char[cur_packet->packet->size+2];
			memcpy(sendbuffer,cur_packet->packet->GetUDPHeader(),2);
			memcpy(sendbuffer+2,cur_packet->packet->pBuffer,cur_packet->packet->size);
			if (!SendTo(sendbuffer, cur_packet->packet->size+2, cur_packet->dwIP, cur_packet->nPort)){
				controlpacket_queue.RemoveHead();
				delete cur_packet->packet;
				delete cur_packet;
			}
			delete[] sendbuffer;
		}
		else
		{
			controlpacket_queue.RemoveHead();
			delete cur_packet->packet;
			delete cur_packet;
		}
	}
}

int CClientUDPSocket::SendTo(char* lpBuf,int nBufLen,uint32 dwIP, uint16 nPort){
	in_addr host;
	host.S_un.S_addr = dwIP;
	uint32 result = CAsyncSocket::SendTo(lpBuf,nBufLen,nPort,inet_ntoa(host));
	if (result == (uint32)SOCKET_ERROR){
		uint32 error = GetLastError();
		if (error == WSAEWOULDBLOCK){
			m_bWouldBlock = true;
			return -1;
		}
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Client UDP socket, failed to send data to %s:%u: %s"), inet_ntoa(host), nPort, GetErrorMessage(error, 1));
	}
	return 0;
}

bool CClientUDPSocket::SendPacket(Packet* packet, uint32 dwIP, uint16 nPort){
	UDPPack* newpending = new UDPPack;
	newpending->dwIP = dwIP;
	newpending->nPort = nPort;
	newpending->packet = packet;
	newpending->dwTime = GetTickCount();
	if (IsBusy()){
		controlpacket_queue.AddTail(newpending);
		return true;
	}
	char* sendbuffer = new char[packet->size+2];
	memcpy(sendbuffer,packet->GetUDPHeader(),2);
	memcpy(sendbuffer+2,packet->pBuffer,packet->size);
	if (SendTo(sendbuffer, packet->size+2, dwIP, nPort)){
		controlpacket_queue.AddTail(newpending);
	}
	else{
		delete newpending->packet;
		delete newpending;
	}
	delete[] sendbuffer;
	return true;
}

bool CClientUDPSocket::Create(){
	if (thePrefs.GetUDPPort())
		return CAsyncSocket::Create(thePrefs.GetUDPPort(),SOCK_DGRAM,FD_READ|FD_WRITE);
	else
		return true;
}