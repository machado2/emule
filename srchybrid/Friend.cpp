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
#include "Friend.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "Packets.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


const char CFriend::sm_abyNullHash[16] = {0};

CFriend::CFriend(void)
{
	m_dwLastSeen = 0;
	m_dwLastUsedIP = 0;
	m_nLastUsedPort = 0;
	m_dwLastChatted = 0;
	(void)m_strName;
	m_LinkedClient = 0;
	md4cpy(m_abyUserhash, sm_abyNullHash);
	m_dwHasHash = 0;
}

//Added this to work with the IRC.. Probably a better way to do it.. But wanted this in the release..
CFriend::CFriend(const uchar* abyUserhash, uint32 dwLastSeen, uint32 dwLastUsedIP, uint32 nLastUsedPort, 
				 uint32 dwLastChatted, LPCTSTR pszName, uint32 dwHasHash){
	m_dwLastSeen = dwLastSeen;
	m_dwLastUsedIP = dwLastUsedIP;
	m_nLastUsedPort = nLastUsedPort;
	m_dwLastChatted = dwLastChatted;
	if( dwHasHash && abyUserhash){
		md4cpy(m_abyUserhash,abyUserhash);
		m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;
	}
	else{
		md4cpy(m_abyUserhash, sm_abyNullHash);
		m_dwHasHash = 0;
	}
	m_strName = pszName;
	m_LinkedClient = 0;
}

CFriend::CFriend(CUpDownClient* client){
	ASSERT ( client );
	m_dwLastSeen = time(NULL);
	m_dwLastUsedIP = client->GetIP();
	m_nLastUsedPort = client->GetUserPort();
	m_dwLastChatted = 0;
	m_strName = client->GetUserName();
	md4cpy(m_abyUserhash,client->GetUserHash());
	m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;
	m_LinkedClient = client;
}

CFriend::~CFriend(void)
{
}

void CFriend::LoadFromFile(CFileDataIO* file)
{
	file->ReadHash16(m_abyUserhash);
	m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;
	m_dwLastUsedIP = file->ReadUInt32();
	m_nLastUsedPort = file->ReadUInt16();
	m_dwLastSeen = file->ReadUInt32();
	m_dwLastChatted = file->ReadUInt32();

	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file);
		switch(newtag->tag.specialtag){
			case FF_NAME:{
				m_strName = newtag->tag.stringvalue;
				break;
			}
		}	
		delete newtag;
	}
}

void CFriend::WriteToFile(CFileDataIO* file)
{
	if (!m_dwHasHash)
		md4cpy(m_abyUserhash, sm_abyNullHash);
	file->WriteHash16(m_abyUserhash);
	file->WriteUInt32(m_dwLastUsedIP);
	file->WriteUInt16(m_nLastUsedPort);
	file->WriteUInt32(m_dwLastSeen);
	file->WriteUInt32(m_dwLastChatted);

	UINT tagcount = 0;
	if (!m_strName.IsEmpty())
		tagcount++;
	file->WriteUInt32(tagcount);
	if (!m_strName.IsEmpty()){
		CTag nametag(FF_NAME, m_strName);
		nametag.WriteTagToFile(file);
	}
}