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
#include "EMSocket.h"

class CUpDownClient;
class CPacket;
class CTimerWnd;

class CClientReqSocket : public CEMSocket{
	DECLARE_DYNCREATE(CClientReqSocket)
	friend class CListenSocket;
public:
	CClientReqSocket(CUpDownClient* in_client = NULL);	
	void	SetClient(CUpDownClient* pClient);
	void	Disconnect(LPCTSTR pszReason);

	void	ResetTimeOutTimer();
	bool	CheckTimeOut();
	virtual UINT GetTimeOut();
	virtual void Safe_Delete();
	
	bool	Create();
	virtual void SendPacket(Packet* packet, bool delpacket = true, bool controlpacket = true, uint32 actualPayloadSize = 0);
    virtual SocketSentBytes Send(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend, bool onlyAllowedToSendControlPacket = false);

	void	DbgAppendClientInfo(CString& str);
	CString DbgGetClientInfo();

	CUpDownClient*	client;

protected:
	virtual void Close()	{CAsyncSocketEx::Close();} // deadlake PROXYSUPPORT - changed to AsyncSocketEx
	virtual	void OnInit();
	virtual void OnConnect(int nErrorCode);
	void		 OnClose(int nErrorCode);
	void		 OnSend(int nErrorCode);
	void		 OnReceive(int nErrorCode);
	void		 OnError(int nErrorCode);
	virtual bool PacketReceived(Packet* packet);
	int			 PacketReceivedSEH(Packet* packet);
	bool		 PacketReceivedCppEH(Packet* packet);

	void	Delete_Timed();
	virtual ~CClientReqSocket();

	bool	ProcessPacket(char* packet, uint32 size, UINT opcode);
	bool	ProcessExtPacket(char* packet, uint32 size, UINT opcode, UINT uRawSize);
	void	PacketToDebugLogLine(LPCTSTR protocol, const char* packet, uint32 size, UINT opcode, EDebugLogPriority dlpPriority);

	uint32	timeout_timer;
	bool	deletethis;
	uint32	deltimer;
};


class CListenSocket : public CAsyncSocketEx
{
public:
	CListenSocket();
	~CListenSocket();

	bool	StartListening();
	void	StopListening();
	virtual void OnAccept(int nErrorCode);
	void	Process();
	void	RemoveSocket(CClientReqSocket* todel);
	void	AddSocket(CClientReqSocket* toadd);
	uint16	GetOpenSockets()		{return socket_list.GetCount();}
	void	KillAllSockets();
	bool	TooManySockets(bool bIgnoreInterval = false);
	uint32	GetMaxConnectionReached()	{return maxconnectionreached;}
	bool    IsValidSocket(CClientReqSocket* totest);
	void	AddConnection();
	void	RecalculateStats();
	void	ReStartListening();
	void	Debug_ClientDeleted(CUpDownClient* deleted);

	void	UpdateConnectionsStatus();
	float	GetMaxConperFiveModifier();
	uint32	GetPeakConnections()		{ return peakconnections; }
	uint32	GetTotalConnectionChecks()	{ return totalconnectionchecks; }
	float	GetAverageConnections()		{ return averageconnections; }
	uint32	GetActiveConnections()		{ return activeconnections; }

private:
	bool bListening;
	CTypedPtrList<CPtrList, CClientReqSocket*> socket_list;
	uint16	opensockets;
	uint16	m_OpenSocketsInterval;
	uint32	maxconnectionreached;
	uint16	m_ConnectionStates[3];
	uint16	m_nPendingConnections;
	uint32	peakconnections;
	uint32	totalconnectionchecks;
	float	averageconnections;
	uint32	activeconnections;
};
