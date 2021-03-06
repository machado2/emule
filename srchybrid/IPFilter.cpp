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
#include "stdafx.h"
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include "emule.h"
#include "IPFilter.h"
#include "OtherFunctions.h"
#include "StringConversion.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	DFLT_FILTER_LEVEL	100 // default filter level if non specified

CIPFilter::CIPFilter()
{
	m_pLastHit = NULL;
	LoadFromDefaultFile(false);
}

CIPFilter::~CIPFilter()
{
	RemoveAllIPFilters();
}

void CIPFilter::AddIPRange(uint32 start, uint32 end, UINT level, const CString& desc)
{
	SIPFilter* newFilter = new SIPFilter;
	newFilter->start = start;
	newFilter->end = end;
	newFilter->level = level;
	newFilter->desc = desc;
	newFilter->hits = 0;
	m_iplist.Add(newFilter);
}

static int __cdecl CmpSIPFilterByStartAddr(const void* p1, const void* p2)
{
	const SIPFilter* rng1 = *(SIPFilter**)p1;
	const SIPFilter* rng2 = *(SIPFilter**)p2;
	return CompareUnsigned(rng1->start, rng2->start);
}

CString CIPFilter::GetDefaultFilePath() const
{
	return thePrefs.GetConfigDir() + DFLT_IPFILTER_FILENAME;
}

int CIPFilter::LoadFromDefaultFile(bool bShowResponse)
{
	RemoveAllIPFilters();
	return AddFromFile(GetDefaultFilePath(), bShowResponse);
}

int CIPFilter::AddFromFile(LPCTSTR pszFilePath, bool bShowResponse)
{
	DWORD dwStart = GetTickCount();
	FILE* readFile = _tfsopen(pszFilePath, _T("r"), _SH_DENYWR);
	if (readFile != NULL)
	{
		enum EIPFilterFileType
		{
			Unknown = 0,
			FilterDat = 1,		// ipfilter.dat/ip.prefix format
			PeerGuardian = 2,	// PeerGuardian text format
			PeerGuardian2 = 3	// PeerGuardian binary format
		} eFileType = Unknown;

		setvbuf(readFile, NULL, _IOFBF, 32768);

		TCHAR szNam[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		_tsplitpath(pszFilePath, NULL, NULL, szNam, szExt);
		if (_tcsicmp(szExt, _T(".p2p")) == 0 || (_tcsicmp(szNam, _T("guarding.p2p")) == 0 && _tcsicmp(szExt, _T(".txt")) == 0))
			eFileType = PeerGuardian;
		else if (_tcsicmp(szExt, _T(".prefix")) == 0)
			eFileType = FilterDat;
		else
		{
			_setmode(fileno(readFile), _O_BINARY);
			static const BYTE _aucP2Bheader[] = "\xFF\xFF\xFF\xFFP2B";
			BYTE aucHeader[sizeof _aucP2Bheader - 1];
			if (fread(aucHeader, sizeof aucHeader, 1, readFile) == 1)
			{
				if (memcmp(aucHeader, _aucP2Bheader, sizeof _aucP2Bheader - 1)==0)
					eFileType = PeerGuardian2;
				else
				{
					fseek(readFile, 0, SEEK_SET);
					_setmode(fileno(readFile), _O_TEXT); // ugly!
				}
			}
		}

		int iFoundRanges = 0;
		int iLine = 0;
		if (eFileType == PeerGuardian2)
		{
			// Version 1: strings are ISO-8859-1 encoded
			// Version 2: strings are UTF-8 encoded
			uint8 nVersion;
			if (fread(&nVersion, sizeof nVersion, 1, readFile)==1 && (nVersion==1 || nVersion==2))
			{
				while (!feof(readFile))
				{
					CHAR szName[256];
					int iLen = 0;
					for (;;) // read until NUL or EOF
					{
						int iChar = getc(readFile);
						if (iChar == EOF)
							break;
						if (iLen < sizeof szName - 1)
							szName[iLen++] = (CHAR)iChar;
						if (iChar == '\0')
							break;
					}
					szName[iLen] = '\0';
					
					uint32 uStart;
					if (fread(&uStart, sizeof uStart, 1, readFile) != 1)
						break;
					uStart = ntohl(uStart);

					uint32 uEnd;
					if (fread(&uEnd, sizeof uEnd, 1, readFile) != 1)
						break;
					uEnd = ntohl(uEnd);

					iLine++;
					AddIPRange(uStart, uEnd, DFLT_FILTER_LEVEL, (nVersion == 2) ? OptUtf8ToStr(szName, iLen) : CString(szName));
					iFoundRanges++;
				}
			}
		}
		else
		{
			CString sbuffer;
			TCHAR szBuffer[1024];
			while (_fgetts(szBuffer, ARRSIZE(szBuffer), readFile) != NULL)
			{
				iLine++;
				sbuffer = szBuffer;
				
				// ignore comments & too short lines
				if (sbuffer.GetAt(0) == _T('#') || sbuffer.GetAt(0) == _T('/') || sbuffer.GetLength() < 5) {
					sbuffer.Trim(_T(" \t\r\n"));
					DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter: ignored line %u\n", iLine) : 0 );
					continue;
				}

				if (eFileType == Unknown)
				{
					// looks like html
					if (sbuffer.Find(_T('>')) > -1 && sbuffer.Find(_T('<')) > -1)
						sbuffer.Delete(0, sbuffer.ReverseFind(_T('>')) + 1);

					// check for <IP> - <IP> at start of line
					UINT u1, u2, u3, u4, u5, u6, u7, u8;
					if (_stscanf(sbuffer, _T("%u.%u.%u.%u - %u.%u.%u.%u"), &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
					{
						eFileType = FilterDat;
					}
					else
					{
						// check for <description> ':' <IP> '-' <IP>
						int iColon = sbuffer.Find(_T(':'));
						if (iColon > -1)
						{
							CString strIPRange = sbuffer.Mid(iColon + 1);
							UINT u1, u2, u3, u4, u5, u6, u7, u8;
							if (_stscanf(strIPRange, _T("%u.%u.%u.%u - %u.%u.%u.%u"), &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
							{
								eFileType = PeerGuardian;
							}
						}
					}
				}

				bool bValid = false;
				uint32 start = 0;
				uint32 end = 0;
				UINT level = 0;
				CString desc;
				if (eFileType == FilterDat)
					bValid = ParseFilterLine1(sbuffer, start, end, level, desc);
				else if (eFileType == PeerGuardian)
					bValid = ParseFilterLine2(sbuffer, start, end, level, desc);

				// add a filter
				if (bValid)
				{
					AddIPRange(start, end, level, desc);
					iFoundRanges++;
				}
				else
				{
					sbuffer.Trim(_T(" \t\r\n"));
					DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter: ignored line %u\n", iLine) : 0 );
				}
			}
		}
		fclose(readFile);

		// sort the IP filter list by IP range start addresses
		qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByStartAddr);

		// merge overlapping and adjacent filter ranges
		int iDuplicate = 0;
		int iMerged = 0;
		if (m_iplist.GetCount() >= 2)
		{
			SIPFilter* pPrv = m_iplist[0];
			int i = 1;
			while (i < m_iplist.GetCount())
			{
				SIPFilter* pCur = m_iplist[i];
				if (   pCur->start >= pPrv->start && pCur->start <= pPrv->end	 // overlapping
					|| pCur->start == pPrv->end+1 && pCur->level == pPrv->level) // adjacent
				{
					if (pCur->start != pPrv->start || pCur->end != pPrv->end) // don't merge identical entries
					{
						//TODO: not yet handled, overlapping entries with different 'level'
						if (pCur->end > pPrv->end)
							pPrv->end = pCur->end;
						//pPrv->desc += _T("; ") + pCur->desc; // this may create a very very long description string...
						iMerged++;
					}
					else
					{
						// if we have identical entries, use the lowest 'level'
						if (pCur->level < pPrv->level)
							pPrv->level = pCur->level;
						iDuplicate++;
					}
					delete pCur;
					m_iplist.RemoveAt(i);
					continue;
				}
				pPrv = pCur;
				i++;
			}
		}

		AddLogLine(bShowResponse, GetResString(IDS_IPFILTERLOADED), m_iplist.GetCount());
		if (thePrefs.GetVerbose())
		{
			DWORD dwEnd = GetTickCount();
			AddDebugLogLine(false, _T("Loaded IP filters from \"%s\""), pszFilePath);
			AddDebugLogLine(false, _T("Parsed lines/entries:%u  Found IP ranges:%u  Duplicate:%u  Merged:%u  Time:%s"), iLine, iFoundRanges, iDuplicate, iMerged, CastSecondsToHM((dwEnd-dwStart+500)/1000));
		}
	}
	return m_iplist.GetCount();
}

void CIPFilter::SaveToDefaultFile()
{
	CString strFilePath = thePrefs.GetConfigDir() + DFLT_IPFILTER_FILENAME;
	FILE* fp = _tfsopen(strFilePath, _T("wt"), _SH_DENYWR);
	if (fp != NULL)
	{
		for (int i = 0; i < m_iplist.GetCount(); i++)
		{
			const SIPFilter* flt = m_iplist[i];

			TCHAR szStart[16];
			_tcscpy(szStart, ipstr(htonl(flt->start)));

			TCHAR szEnd[16];
			_tcscpy(szEnd, ipstr(htonl(flt->end)));

			if (_ftprintf(fp, _T("%-15s - %-15s , %3u , %s\n"), szStart, szEnd, flt->level, flt->desc) == 0 || ferror(fp))
			{
				CString strError;
				strError.Format(_T("Failed to save IP filter to file \"%s\" - %hs"), strFilePath, strerror(errno));
				throw strError;
			}
		}
		fclose(fp);
	}
	else
	{
		CString strError;
		strError.Format(_T("Failed to save IP filter to file \"%s\" - %hs"), strFilePath, strerror(errno));
		throw strError;
	}
}

bool CIPFilter::ParseFilterLine1(const CString& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CString& desc) const
{
	TCHAR szDesc[256];
	memset(szDesc, 0, sizeof szDesc);
	UINT u1, u2, u3, u4, u5, u6, u7, u8, uLevel = DFLT_FILTER_LEVEL;
	int iItems = _stscanf(sbuffer, _T("%u.%u.%u.%u - %u.%u.%u.%u , %u , %255c"), &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8, &uLevel, szDesc);
	if (iItems < 8)
		return false;

	((BYTE*)&ip1)[0] = u4;
	((BYTE*)&ip1)[1] = u3;
	((BYTE*)&ip1)[2] = u2;
	((BYTE*)&ip1)[3] = u1;

	((BYTE*)&ip2)[0] = u8;
	((BYTE*)&ip2)[1] = u7;
	((BYTE*)&ip2)[2] = u6;
	((BYTE*)&ip2)[3] = u5;

	if (iItems == 8)
	{
		level = DFLT_FILTER_LEVEL;	// set default level
		return true;
	}

	level = uLevel;

	if (iItems == 9)
		return true;	 // no description available

	ASSERT( iItems == 10 );
	desc = szDesc;
	desc.Trim();

	return true;
}

bool CIPFilter::ParseFilterLine2(const CString& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CString& desc) const
{
	int iPos = sbuffer.ReverseFind(_T(':'));
	if (iPos < 0)
		return false;

	desc = sbuffer.Left(iPos);
	desc.Replace(_T("PGIPDB"), _T(""));
	desc.Trim();

	CString strIPRange = sbuffer.Mid(iPos + 1, sbuffer.GetLength() - iPos);
	UINT u1, u2, u3, u4, u5, u6, u7, u8;
	if (_stscanf(strIPRange, _T("%u.%u.%u.%u - %u.%u.%u.%u"), &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) != 8)
		return false;

	((BYTE*)&ip1)[0] = u4;
	((BYTE*)&ip1)[1] = u3;
	((BYTE*)&ip1)[2] = u2;
	((BYTE*)&ip1)[3] = u1;

	((BYTE*)&ip2)[0] = u8;
	((BYTE*)&ip2)[1] = u7;
	((BYTE*)&ip2)[2] = u6;
	((BYTE*)&ip2)[3] = u5;

	level = DFLT_FILTER_LEVEL;

	return true;
}

void CIPFilter::RemoveAllIPFilters()
{
	for (int i = 0; i < m_iplist.GetCount(); i++)
		delete m_iplist[i];
	m_iplist.RemoveAll();
	m_pLastHit = NULL;
}

bool CIPFilter::IsFiltered(uint32 ip) /*const*/
{
	return IsFiltered(ip, thePrefs.GetIPFilterLevel());
}

static int __cdecl CmpSIPFilterByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const SIPFilter* pIPFilter = *(SIPFilter**)pvElement;

	if (ip < pIPFilter->start)
		return -1;
	if (ip > pIPFilter->end)
		return 1;
	return 0;
}

bool CIPFilter::IsFiltered(uint32 ip, UINT level) /*const*/
{
	if (m_iplist.GetCount() == 0 || ip == 0)
		return false;
	
	ip = htonl(ip);
	/*for (int i = 0; i < m_iplist.GetCount(); i++)
	{
		SIPFilter* search = m_iplist[i];
		if (ip < search->start)
			return false;
		if (search->level < level && ip >= search->start && ip <= search->end)
		{
			search->hits++;
			m_pLastHit = search;
			return true;
		}
	}*/

	// to speed things up we use a binary search
	//	*)	the IP filter list must be sorted by IP range start addresses
	//	*)	the IP filter list is not allowed to contain overlapping IP ranges (see also the IP range merging code when
	//		loading the list)
	//	*)	the filter 'level' is ignored during the binary search and is evaluated only for the found element
	//
	// TODO: this can still be improved even more:
	//	*)	use a pre assembled list of IP ranges which contains only the IP ranges for the currently used filter level
	//	*)	use a dumb plain array for storing the IP range structures. this will give more cach hits when processing
	//		the list. but(!) this would require to also use a dumb SIPFilter structure (don't use data items with ctors).
	//		otherwise the creation of the array would be rather slow.
	SIPFilter** ppFound = (SIPFilter**)bsearch(&ip, m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if (ppFound && (*ppFound)->level < level)
	{
		(*ppFound)->hits++;
		m_pLastHit = *ppFound;
		return true;
	}

	return false;
}

LPCTSTR CIPFilter::GetLastHit() const
{
	return m_pLastHit ? m_pLastHit->desc : _T("Not available");
}

const CIPFilterArray& CIPFilter::GetIPFilter() const
{
	return m_iplist;
}

bool CIPFilter::RemoveIPFilter(const SIPFilter* pFilter)
{
	for (int i = 0; i < m_iplist.GetCount(); i++)
	{
		if (m_iplist[i] == pFilter)
		{
			delete m_iplist[i];
			m_iplist.RemoveAt(i);
			return true;
		}
	}
	return false;
}
