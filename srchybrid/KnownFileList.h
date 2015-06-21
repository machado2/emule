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
#include "MapKey.h"
#include "Loggable.h"

class CKnownFile;

class CKnownFileList : public CLoggable
{
	friend class CSharedFilesWnd;
	friend class CFileStatistic;
public:
	CKnownFileList();
	~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();

	CKnownFile* FindKnownFile(LPCTSTR filename, uint32 date, uint32 size) const;
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

private:
	uint16 	requested;
	uint16 	accepted;
	uint64 	transferred;
	uint32 	m_nLastSaved;
	CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> m_Files_map;
};
