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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "Loggable.h"
#include "BarShader.h"
#include <list>

class CTag;
class CxImage;
namespace Kademlia{
class CUInt128;
typedef std::list<CString> WordList;
};
class CUpDownClient;
class Packet;
class CFileDataIO;
typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CFileStatistic
{
	friend class CKnownFile;
	friend class CPartFile;
public:
	CFileStatistic()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
	}

	void	MergeFileStats( CFileStatistic* toMerge );
	void	AddRequest();
	void	AddAccepted();
	void	AddTransferred(uint64 bytes);

	uint16	GetRequests() const				{return requested;}
	uint16	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	uint16	GetAllTimeRequests() const		{return alltimerequested;}
	uint16	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	
	CKnownFile* fileParent;

private:
	uint16 requested;
	uint16 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
};

/*
					   CPartFile
					 /
		  CKnownFile
		/
CAbstractFile
		\
		  CSearchFile
*/
class CAbstractFile: public CObject, public CLoggable
{
	DECLARE_DYNAMIC(CAbstractFile)

public:
	CAbstractFile();
	virtual ~CAbstractFile() { }

	const CString& GetFileName() const { return m_strFileName; }
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bAutoSetFileType = true); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	// returns the ED2K file type (an ASCII string)
	const CStringA& GetFileType() const { return m_strFileType; }
	virtual void SetFileType(LPCSTR pszFileType);

	// returns the file type which is used to be shown in the GUI
	CString GetFileTypeDisplayStr() const;

	const uchar* GetFileHash() const { return m_abyFileHash; }
	void SetFileHash(const uchar* pucFileHash);
	bool HasNullHash() const;

	uint32 GetFileSize() const { return m_nFileSize; }
	virtual void SetFileSize(uint32 nFileSize) { m_nFileSize = nFileSize; }

	uint32 GetIntTagValue(uint8 tagname) const;
	uint32 GetIntTagValue(LPCSTR tagname) const;
	bool GetIntTagValue(uint8 tagname, uint32& ruValue) const;
	LPCSTR GetStrTagValueA(uint8 tagname) const;
	LPCSTR GetStrTagValueA(LPCSTR tagname) const;
	CString GetStrTagValue(uint8 tagname) const;
	CString GetStrTagValue(LPCSTR tagname) const;
	CTag* GetTag(uint8 tagname, uint8 tagtype) const;
	CTag* GetTag(LPCSTR tagname, uint8 tagtype) const;
	CTag* GetTag(uint8 tagname) const;
	CTag* GetTag(LPCSTR tagname) const;
	void AddTagUnique(CTag* pTag);
	const CArray<CTag*,CTag*>& GetTags() const { return taglist; }

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CString m_strFileName;
	uchar	m_abyFileHash[16];
	uint32	m_nFileSize;
	CString m_strComment;
	uint8	m_iRate;
	CStringA m_strFileType;
	CArray<CTag*,CTag*> taglist;
};

class CKnownFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	~CKnownFile();

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	const CString& GetPath() const { return m_strDirectory; }
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	virtual bool CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	virtual bool IsPartFile() const { return false; }
	virtual bool LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const { return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const { return m_tUtcLastModified; }

	virtual void SetFileSize(uint32 nFileSize);

	// local available part hashs
	uint16	GetHashCount() const { return hashlist.GetCount(); }
	uchar*	GetPartHash(uint16 part) const;
	const CArray<uchar*, uchar*>& GetHashset() const { return hashlist; }
	bool	SetHashset(const CArray<uchar*, uchar*>& aHashset);

	// nr. of part hashs according the file size wrt ED2K protocol
	UINT	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const { return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// file upload priority
	uint8	GetUpPriority(void) const { return m_iUpPriority; }
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const { return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority) { m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}

	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;

	// comment
	const CString& GetFileComment() /*const*/;
	void	SetFileComment(LPCTSTR pszComment);

	uint8	GetFileRate() /*const*/;
	void	SetFileRate(uint8 uRate);

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	uint32	GetKadFileSearchID() const { return kadFileSearchID; }
	void	SetKadFileSearchID(uint32 id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

	uint32	GetPublishedKadSrc() const { return m_PublishedKadSrc; }
	void	SetPublishedKadSrc();

	const Kademlia::WordList& GetKadKeywords() const { return wordlist; }

	uint32	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }
	void	SetLastPublishTimeKadSrc(uint32 val) { m_lastPublishTimeKadSrc = val; }

	int		PublishKey(Kademlia::CUInt128* nextID);
	bool	PublishSrc(Kademlia::CUInt128* nextID);

	// file sharing
	virtual Packet* CreateSrcInfoPacket(CUpDownClient* forClient) const;
	UINT	GetMetaDataVer() const { return m_uMetaDataVer; }
	void	UpdateMetaDataTags();
	void	RemoveMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CFileStatistic statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16, uint16> m_AvailPartFrequency;

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	void	CreateHashFromFile(FILE* file, int Length, uchar* Output) const { CreateHashFromInput(file, NULL, Length, Output, NULL); }
	void	CreateHashFromFile(CFile* file, int Length, uchar* Output) const { CreateHashFromInput(NULL, file, Length, Output, NULL); }
	void	CreateHashFromString(uchar* in_string, int Length, uchar* Output) const { CreateHashFromInput(NULL, NULL, Length, Output, in_string); }
	void	LoadComment();

	CArray<uchar*, uchar*> hashlist;
	CString m_strDirectory;
	CString m_strFilePath;

private:
	void	CreateHashFromInput(FILE* file, CFile* file2, int Length, uchar* Output, uchar* = NULL) const;

	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_bCommentLoaded;
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_PublishedKadSrc;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
};

// permission values for shared files
#define PERM_ALL		0
#define PERM_FRIENDS	1
#define PERM_NOONE		2

// constants for MD4Transform
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

// basic MD4 functions
#define MD4_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD4_G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

// rotates x left n bits
// 15-April-2003 Sony: use MSVC intrinsic to save a few cycles
#ifdef _MSC_VER
#pragma intrinsic(_rotl)
#define MD4_ROTATE_LEFT(x, n) _rotl((x), (n))
#else
#define MD4_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

// partial transformations
#define MD4_FF(a, b, c, d, x, s) \
{ \
  (a) += MD4_F((b), (c), (d)) + (x); \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_GG(a, b, c, d, x, s) \
{ \
  (a) += MD4_G((b), (c), (d)) + (x) + (uint32)0x5A827999; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_HH(a, b, c, d, x, s) \
{ \
  (a) += MD4_H((b), (c), (d)) + (x) + (uint32)0x6ED9EBA1; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

static void MD4Transform(uint32 Hash[4], uint32 x[16]);

