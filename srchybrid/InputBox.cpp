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
#include "resource.h"
#include "InputBox.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// InputBox dialog

IMPLEMENT_DYNAMIC(InputBox, CDialog)
InputBox::InputBox(CWnd* pParent /*=NULL*/)
	: CDialog(InputBox::IDD, pParent)
{
	m_label="";
	m_title="";
	m_default="";
	m_return="";
	m_cancel=true;
	m_bFilenameMode=false;
}

InputBox::~InputBox()
{
}

void InputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(InputBox, CDialog)
	ON_BN_CLICKED(IDC_CLEANFILENAME, OnCleanFilename)
END_MESSAGE_MAP()

void InputBox::OnOK()
{	char buffer[510];
	m_cancel=false;
	if(GetDlgItem(IDC_TEXT)->GetWindowTextLength())
	{ 
		GetDlgItem(IDC_TEXT)->GetWindowText(buffer,510);
		m_return.Format("%s",buffer);
	}
	CDialog::OnOK();
}

void InputBox::SetLabels(CString title,CString label,CString defaultStr){
	m_label=label;
	m_title=title;
	m_default=defaultStr;
}

CString InputBox::GetInput(){
	return m_return;
}

BOOL InputBox::OnInitDialog(){
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	GetDlgItem(IDC_IBLABEL)->SetWindowText( m_label);
	GetDlgItem(IDC_TEXT)->SetWindowText(m_default);
	SetWindowText(m_title);

	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_CLEANFILENAME,GetResString(IDS_CLEANUP));
	GetDlgItem(IDC_CLEANFILENAME)->ShowWindow( m_bFilenameMode?SW_NORMAL:SW_HIDE);

	return TRUE;
}

void InputBox::OnCleanFilename() {
	CString filename;
	GetDlgItem(IDC_TEXT)->GetWindowText(filename);
	GetDlgItem(IDC_TEXT)->SetWindowText( CleanupFilename(filename) );
}