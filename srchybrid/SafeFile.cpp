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
#include "stdafx.h"
#include "SafeFile.h"
#include "Packets.h"
#include "kademlia/utils/UInt128.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


///////////////////////////////////////////////////////////////////////////////
// CFileDataIO

uint8 CFileDataIO::ReadUInt8()
{
	uint8 nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

uint16 CFileDataIO::ReadUInt16()
{
	uint16 nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

uint32 CFileDataIO::ReadUInt32()
{
	uint32 nVal;
	Read(&nVal, sizeof nVal);
	return nVal;
}

void CFileDataIO::ReadUInt128(Kademlia::CUInt128 *pVal)
{
	Read(pVal->getDataPtr(), 16);
}

void CFileDataIO::ReadHash16(uchar* pVal)
{
	Read(pVal, 16);
}

CString CFileDataIO::ReadString()
{
	UINT uLen = ReadUInt16();
	CStringA str;
	Read(str.GetBuffer(uLen), uLen);
	str.ReleaseBuffer(uLen);
#ifdef _UNICODE
	return CString(str);
#else
	return str;
#endif
}

void CFileDataIO::WriteUInt8(uint8 nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt16(uint16 nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt32(uint32 nVal)
{
	Write(&nVal, sizeof nVal);
}

void CFileDataIO::WriteUInt128(const Kademlia::CUInt128 *pVal)
{
	Write(pVal->getData(), 16);
}

void CFileDataIO::WriteHash16(const uchar* pVal)
{
	Write(pVal, 16);
}

void CFileDataIO::WriteString(const CString& rstr, bool bOptUTF8)
{
#ifdef _UNICODE
#else
	UINT uLen = rstr.GetLength();
	WriteUInt16(uLen);
	Write((LPCSTR)rstr, uLen);
#endif
}

void CFileDataIO::WriteString(LPCSTR psz)
{
	UINT uLen = strlen(psz);
	WriteUInt16(uLen);
	Write(psz, uLen);
}


///////////////////////////////////////////////////////////////////////////////
// CSafeFile

UINT CSafeFile::Read(void* lpBuf, UINT nCount)
{
	if (GetPosition() + nCount > GetLength())
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return CFile::Read(lpBuf, nCount);
}

void CSafeFile::Write(const void* lpBuf, UINT nCount)
{
	CFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CFile::Seek(lOff, nFrom);
}

ULONGLONG CSafeFile::GetPosition() const
{
	return CFile::GetPosition();
}


///////////////////////////////////////////////////////////////////////////////
// CSafeMemFile

UINT CSafeMemFile::Read(void* lpBuf, UINT nCount)
{
	if (m_nPosition + nCount > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return CMemFile::Read(lpBuf, nCount);
}

void CSafeMemFile::Write(const void* lpBuf, UINT nCount)
{
	CMemFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeMemFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CMemFile::Seek(lOff, nFrom);
}

uint8 CSafeMemFile::ReadUInt8()
{
	if (m_nPosition + sizeof(uint8) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return *(m_lpBuffer + m_nPosition++);
}

uint16 CSafeMemFile::ReadUInt16()
{
	if (m_nPosition + sizeof(uint16) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint16 nResult = *((uint16*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(uint16);
	return nResult;
}

uint32 CSafeMemFile::ReadUInt32()
{
	if (m_nPosition + sizeof(uint32) > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint32 nResult = *((uint32*)(m_lpBuffer + m_nPosition));
	m_nPosition += sizeof(uint32);
	return nResult;
}

void CSafeMemFile::ReadUInt128(Kademlia::CUInt128* pVal)
{
	if (m_nPosition + sizeof(uint32)*4 > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	uint32* pUInt32Val = (uint32*)pVal->getDataPtr();
	const uint32* pUInt32 = (uint32*)(m_lpBuffer + m_nPosition);
	pUInt32Val[0] = pUInt32[0];
	pUInt32Val[1] = pUInt32[1];
	pUInt32Val[2] = pUInt32[2];
	pUInt32Val[3] = pUInt32[3];
	m_nPosition += sizeof(uint32)*4;
}

void CSafeMemFile::ReadHash16(uchar* pVal)
{
	if (m_nPosition + sizeof(uint32)*4 > m_nFileSize)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	const uint32* pUInt32 = (uint32*)(m_lpBuffer + m_nPosition);
	((uint32*)pVal)[0] = pUInt32[0];
	((uint32*)pVal)[1] = pUInt32[1];
	((uint32*)pVal)[2] = pUInt32[2];
	((uint32*)pVal)[3] = pUInt32[3];
	m_nPosition += sizeof(uint32)*4;
}

void CSafeMemFile::WriteUInt8(uint8 nVal)
{
	if (m_nPosition + sizeof(uint8) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint8));
	*(m_lpBuffer + m_nPosition++) = nVal;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt16(uint16 nVal)
{
	if (m_nPosition + sizeof(uint16) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint16));
	*((uint16*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(uint16);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt32(uint32 nVal)
{
	if (m_nPosition + sizeof(uint32) > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32));
	*((uint32*)(m_lpBuffer + m_nPosition)) = nVal;
	m_nPosition += sizeof(uint32);
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteUInt128(const Kademlia::CUInt128* pVal)
{
	if (m_nPosition + sizeof(uint32)*4 > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32)*4);
	uint32* pUInt32 = (uint32*)(m_lpBuffer + m_nPosition);
	const uint32* pUInt32Val = (uint32*)pVal->getData();
	pUInt32[0] = pUInt32Val[0];
	pUInt32[1] = pUInt32Val[1];
	pUInt32[2] = pUInt32Val[2];
	pUInt32[3] = pUInt32Val[3];
	m_nPosition += sizeof(uint32)*4;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

void CSafeMemFile::WriteHash16(const uchar* pVal)
{
	if (m_nPosition + sizeof(uint32)*4 > m_nBufferSize)
		GrowFile(m_nPosition + sizeof(uint32)*4);
	uint32* pUInt32 = (uint32*)(m_lpBuffer + m_nPosition);
	pUInt32[0] = ((uint32*)pVal)[0];
	pUInt32[1] = ((uint32*)pVal)[1];
	pUInt32[2] = ((uint32*)pVal)[2];
	pUInt32[3] = ((uint32*)pVal)[3];
	m_nPosition += sizeof(uint32)*4;
	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;
}

ULONGLONG CSafeMemFile::GetPosition() const
{
	return CMemFile::GetPosition();
}


///////////////////////////////////////////////////////////////////////////////
// CSafeBufferedFile

UINT CSafeBufferedFile::Read(void* lpBuf, UINT nCount)
{
	// that's terrible slow
//	if (GetPosition()+nCount > this->GetLength())
//		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	UINT uRead = CStdioFile::Read(lpBuf,nCount);
	if (uRead != nCount)
		AfxThrowFileException(CFileException::endOfFile, 0, GetFileName());
	return uRead;
}

void CSafeBufferedFile::Write(const void* lpBuf, UINT nCount)
{
	CStdioFile::Write(lpBuf, nCount);
}

ULONGLONG CSafeBufferedFile::Seek(LONGLONG lOff, UINT nFrom)
{
	return CStdioFile::Seek(lOff, nFrom);
}

ULONGLONG CSafeBufferedFile::GetPosition() const
{
	return CStdioFile::GetPosition();
}

int CSafeBufferedFile::printf(LPCTSTR pszFmt, ...)
{
	va_list args;
	va_start(args, pszFmt);
	int iResult = _vftprintf(m_pStream, pszFmt, args);
	va_end(args);
	if (iResult < 0)
		AfxThrowFileException(CFileException::generic, _doserrno, m_strFileName);
	return iResult;
}
