//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

class CServer;

class CServerList
{
	friend class CServerListCtrl;
public:
	CServerList();
	~CServerList();

	bool		Init();
	void		Process();
	void		Sort();
	void		MoveServerDown(const CServer* pServer);
	void		AutoUpdate();
	bool		AddServerMetToList(const CString& rstrFile, bool bMerge);
	void		AddServersFromTextFile(const CString& rstrFilename);
	bool		SaveServermetToFile();

	bool		AddServer(const CServer* pServer);
	void		RemoveServer(const CServer* pServer);
	void		RemoveAllServers();

	uint32		GetServerCount() const { return list.GetCount(); }
	CServer*	GetServerAt(uint32 pos) const { return list.GetAt(list.FindIndex(pos)); }
	CServer*	GetNextServer(const CServer* lastserver) const;
	CServer*	GetServerByAddress(LPCTSTR address, uint16 port) const;
	CServer*	GetServerByIP(uint32 nIP) const;
	CServer*	GetServerByIP(uint32 nIP, uint16 nPort) const;

	void		SetServerPosition(uint32 newPosition);
	uint32		GetServerPostion() const { return serverpos; }
	CServer*	GetNextServer();

	void		ResetSearchServerPos() { searchserverpos = 0; }
	CServer*	GetNextSearchServer();

	void		ServerStats();
	CServer*	GetNextStatServer();

	bool		IsGoodServerIP(const CServer* pServer) const;
	void		GetStatus(uint32& total, uint32& failed, uint32& user, uint32& file, uint32& lowiduser, 
						  uint32& totaluser, uint32& totalfile, float& occ) const;
	void		GetAvgFile(uint32& average) const;
	void		GetUserFileStatus(uint32& user, uint32& file) const;
	uint32		GetDeletedServerCount() const { return delservercount; }

    bool        GiveServersForTraceRoute();

private:
	uint32		serverpos;
	uint32		searchserverpos;
	uint32		statserverpos;
	uint8		version;
	uint32		servercount;
	CTypedPtrList<CPtrList, CServer*> list;
	uint32		delservercount;
	uint32		m_nLastSaved;
};
