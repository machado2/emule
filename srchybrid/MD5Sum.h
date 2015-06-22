#pragma once

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

class MD5Sum
{
public:
	MD5Sum();
	MD5Sum(CString sSource);
	MD5Sum(uchar* pachSource, uint32 nLen);
	CString Calculate(CString sSource)				{return Calculate((uchar*)sSource.GetBuffer(0), sSource.GetLength());}
	CString Calculate(uchar* pachSource, uint32 nLen);
	CString GetHash();
	uchar*	GetRawHash()							{return m_rawHash;}

private:
	CString			m_sHash;
	unsigned char	m_rawHash[16];
};
