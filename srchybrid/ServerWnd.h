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
#include "ResizableLib\ResizableDialog.h"
#include "ServerListCtrl.h"
#include "LogEditCtrl.h"
#include "IconStatic.h"
#include "RichEditCtrlX.h"
#include "ClosableTabCtrl.h"

class CHTRichEditCtrl;
class CCustomAutoComplete;

class CServerWnd : public CResizableDialog, public CLoggable
{
	DECLARE_DYNAMIC(CServerWnd)

public:
	CServerWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerWnd();
	void Localize();
	bool UpdateServerMetFromURL(CString strURL);
	void ToggleDebugWindow();
	void UpdateMyInfo();
	void UpdateLogTabSelection();
	void SaveAllSettings();
	BOOL SaveServerMetStrings();
	void ShowServerInfo();
	void UpdateControlsState();
	void ResetHistory();
	void PasteServerFromClipboard();
	bool AddServer(uint16 uPort, CString strIP, CString strName = _T(""), bool bShowErrorMB = true);

// Dialog Data
	enum { IDD = IDD_SERVER };

	enum ELogPaneItems
	{
		PaneServerInfo	= 0, // those are CTabCtrl item indices
		PaneLog			= 1,
		PaneVerboseLog	= 2
	};

	CServerListCtrl serverlistctrl;
	CHTRichEditCtrl* servermsgbox;
	CLogEditCtrl logbox;
	CLogEditCtrl debuglog;
	CClosableTabCtrl StatusSelector;

protected:
	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAddserver();
	afx_msg void OnBnClickedUpdateservermetfromurl();
	afx_msg void OnBnClickedResetLog();
	afx_msg void OnBnConnect();
	afx_msg void OnTcnSelchangeTab3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnLinkServerBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnDDClicked();

private:
	CIconStatic m_ctrlNewServerFrm;
	CIconStatic m_ctrlUpdateServerFrm;
	CIconStatic m_ctrlMyInfo;
	CImageList m_imlLogPanes;
	HICON icon_srvlist;
	bool	debug;
	CRichEditCtrlX m_MyInfo;
	CHARFORMAT m_cfDef;
	CHARFORMAT m_cfBold;
	CCustomAutoComplete* m_pacServerMetURL;
	CString m_strClickNewVersion;
	LCID m_uLangID;
};