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
#include "PreferencesDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CPreferencesDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(111,OnSelChanged)
	ON_WM_CTLCOLOR()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndIRC.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndScheduler.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP; // deadlake PROXYSUPPORT
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	AddPage(&m_wndStats);
	AddPage(&m_wndIRC);
	AddPage(&m_wndSecurity);
	AddPage(&m_wndScheduler);
	AddPage(&m_wndWebServer);
	AddPage(&m_wndTweaks);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	m_nActiveWnd = 0;
	m_iPrevPage = -1;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CPropertySheet::OnDestroy();
	thePrefs.Save();
	m_nActiveWnd = GetActiveIndex();
}

BOOL CPreferencesDlg::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CPropertySheet::OnInitDialog();

	m_listbox.CreateEx(WS_EX_CLIENTEDGE,_T("Listbox"),0,WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_TABSTOP|LBS_HASSTRINGS|LBS_OWNERDRAWVARIABLE|WS_BORDER,CRect(0,0,0,0),this,111);
	::SendMessage(m_listbox.m_hWnd, WM_SETFONT, (WPARAM) ::GetStockObject(DEFAULT_GUI_FONT),0);
	m_groupbox.Create(0,BS_GROUPBOX|WS_CHILD|WS_VISIBLE|BS_FLAT,CRect(0,0,0,0),this,666);
	::SendMessage(m_groupbox.m_hWnd, WM_SETFONT, (WPARAM) ::GetStockObject(DEFAULT_GUI_FONT),0);
	InitWindowStyles(this);

	SetActivePage(m_nActiveWnd);
	Localize();	
	m_listbox.SetFocus();
	CString currenttext;
	int curSel=m_listbox.GetCurSel();
	m_listbox.GetText(curSel,currenttext);
	m_groupbox.SetWindowText(currenttext);
	m_iPrevPage = curSel;
	return bResult;
}

void CPreferencesDlg::OnSelChanged()
{
	int curSel=m_listbox.GetCurSel();
	if (!SetActivePage(curSel)){
		if (m_iPrevPage != -1){
			m_listbox.SetCurSel(m_iPrevPage);
			return;
		}
	}
	CString currenttext;
	m_listbox.GetText(curSel,currenttext);
	m_groupbox.SetWindowText(currenttext);
	m_listbox.SetFocus();
	m_iPrevPage = curSel;
}

void CPreferencesDlg::Localize()
{
	ImageList.DeleteImageList();
	ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	ImageList.Add(CTempIconLoader(_T("PREF_GENERAL")));
	ImageList.Add(CTempIconLoader(_T("PREF_DISPLAY")));
	ImageList.Add(CTempIconLoader(_T("PREF_CONNECTION")));
	ImageList.Add(CTempIconLoader(_T("PREF_PROXY")));
	ImageList.Add(CTempIconLoader(_T("PREF_SERVER")));
	ImageList.Add(CTempIconLoader(_T("PREF_FOLDERS")));
	ImageList.Add(CTempIconLoader(_T("PREF_FILES")));
	ImageList.Add(CTempIconLoader(_T("PREF_NOTIFICATIONS")));
	ImageList.Add(CTempIconLoader(_T("PREF_STATISTICS")));
	ImageList.Add(CTempIconLoader(_T("PREF_IRC")));
	ImageList.Add(CTempIconLoader(_T("PREF_SECURITY")));
	ImageList.Add(CTempIconLoader(_T("PREF_SCHEDULER")));
	ImageList.Add(CTempIconLoader(_T("PREF_WEBSERVER")));
	ImageList.Add(CTempIconLoader(_T("PREF_TWEAK")));
	m_listbox.SetImageList(&ImageList);

	CString title = GetResString(IDS_EM_PREFS); 
	title.Remove(_T('&')); 
	SetTitle(title); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	m_wndIRC.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	m_wndScheduler.Localize();
	m_wndProxy.Localize();

	TC_ITEM item; 
	item.mask = TCIF_TEXT; 

	CStringArray buffer; 
	buffer.Add(GetResString(IDS_PW_GENERAL)); 
	buffer.Add(GetResString(IDS_PW_DISPLAY)); 
	buffer.Add(GetResString(IDS_PW_CONNECTION)); 
	buffer.Add(GetResString(IDS_PW_PROXY)); 
	buffer.Add(GetResString(IDS_PW_SERVER)); 
	buffer.Add(GetResString(IDS_PW_DIR)); 
	buffer.Add(GetResString(IDS_PW_FILES)); 
	buffer.Add(GetResString(IDS_PW_EKDEV_OPTIONS)); 
	buffer.Add(GetResString(IDS_STATSSETUPINFO)); 
	buffer.Add(GetResString(IDS_IRC));
	buffer.Add(GetResString(IDS_SECURITY)); 
	buffer.Add(GetResString(IDS_SCHEDULER));
	buffer.Add(GetResString(IDS_PW_WS));
	buffer.Add(GetResString(IDS_PW_TWEAK)); 
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	buffer.Add(_T("Debug"));
#endif

	for (int i = 0; i < buffer.GetCount(); i++)
		buffer[i].Remove(_T('&'));

	m_listbox.ResetContent();
	int width = 0;
	CTabCtrl* tab = GetTabControl();
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject(m_listbox.GetFont());
	CSize sz;
	for(int i = 0; i < GetPageCount(); i++) 
	{ 
		item.pszText = buffer[i].GetBuffer(); 
		tab->SetItem (i, &item); 
		buffer[i].ReleaseBuffer();
		m_listbox.AddString(buffer[i].GetBuffer(),i);
		sz = dc.GetTextExtent(buffer[i]);
		if(sz.cx > width)
			width = sz.cx;
	}
	m_groupbox.SetWindowText(GetResString(IDS_PW_GENERAL));
	width+=50;
	CRect rectOld;
	m_listbox.GetWindowRect(&rectOld);
	int xoffset, yoffset;
	if(IsWindowVisible())
	{
		yoffset=0;
		xoffset=width-rectOld.Width();
	}
	else
	{
		xoffset=width-rectOld.Width()+10;
		tab->GetItemRect(0,rectOld);
		yoffset=-rectOld.Height();
	}
	GetWindowRect(rectOld);
	SetWindowPos(NULL,0,0,rectOld.Width()+xoffset,rectOld.Height()+yoffset,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	tab->GetWindowRect (rectOld);
	ScreenToClient (rectOld);
	tab->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	CPropertyPage* activepage = GetActivePage();
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	activepage->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	m_groupbox.SetWindowPos(NULL,rectOld.left,2,rectOld.Width()+4,rectOld.Height()+10,SWP_NOZORDER|SWP_NOACTIVATE);
	m_groupbox.GetWindowRect(rectOld);
	ScreenToClient(rectOld);
	m_listbox.SetWindowPos(NULL,6,rectOld.top+5,width,rectOld.Height()-4,SWP_NOZORDER|SWP_NOACTIVATE);
	int _PropSheetButtons[] = {IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
	CWnd* PropSheetButton;
	for (int i = 0; i < sizeof (_PropSheetButtons) / sizeof(_PropSheetButtons[0]); i++)
	{
		if ((PropSheetButton = GetDlgItem(_PropSheetButtons[i])) != NULL)
		{
			PropSheetButton->GetWindowRect (rectOld);
			ScreenToClient (rectOld);
			PropSheetButton->SetWindowPos (NULL, rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}
	tab->ShowWindow(SW_HIDE);
	m_listbox.SetCurSel(GetActiveIndex());		
	CenterWindow();
	Invalidate();
	RedrawWindow();
	dc.SelectObject(pOldFont); //restore default font object
}

HBRUSH CPreferencesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_groupbox.m_hWnd == pWnd->m_hWnd) 
	{
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
		hbr = GetSysColorBrush(COLOR_BTNFACE);
	}
	return hbr;
}

void CPreferencesDlg::OnHelp()
{
	int iCurSel = m_listbox.GetCurSel();
	if (iCurSel >= 0)
	{
		CPropertyPage* pPage = GetPage(iCurSel);
		if (pPage)
		{
			HELPINFO hi = {0};
			hi.cbSize = sizeof hi;
			hi.iContextType = HELPINFO_WINDOW;
			hi.iCtrlId = 0;
			hi.hItemHandle = pPage->m_hWnd;
			hi.dwContextId = 0;
			pPage->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			return;
		}
	}

	theApp.ShowHelp(0, HELP_CONTENTS);
}

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnHelp();
	return TRUE;
}

void CPreferencesDlg::OpenPage(UINT uResourceID)
{
	int iCurActiveWnd = m_nActiveWnd;
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == MAKEINTRESOURCE(uResourceID))
		{
			m_nActiveWnd = i;
			break;
		}
	}
	DoModal();
	m_nActiveWnd = iCurActiveWnd;
}
