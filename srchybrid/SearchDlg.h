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
#include "SearchListCtrl.h"
#include "ClosableTabCtrl.h"
#include "IconStatic.h"
#include "RichEditCtrlX.h"
#include "ComboBoxEx2.h"
#include "ListCtrlEditable.h"

class CCustomAutoComplete;
class CJigleSOAPThread;
class Packet;
class CSafeMemFile;


///////////////////////////////////////////////////////////////////////////////
// ESearchType

enum ESearchType
{
	//NOTE: The numbers are *equal* to the entries in the comboxbox -> TODO: use item data
	SearchTypeServer = 0,
	SearchTypeGlobal,
	SearchTypeKademlia,
	SearchTypeJigleSOAP,
	SearchTypeJigle,
	SearchTypeFileDonkey
};


///////////////////////////////////////////////////////////////////////////////
// SSearchParams

struct SSearchParams
{
	SSearchParams()
	{
		dwSearchID = (DWORD)-1;
		eType = SearchTypeServer;
		bClientSharedFiles = false;
		ulMinSize = 0;
		ulMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		ulMinBitrate = 0;
		ulMinLength = 0;
		bMatchKeywords = false;
	}
	DWORD dwSearchID;
	bool bClientSharedFiles;
	CString strExpression;
	ESearchType eType;
	CString strFileType;
	CString strMinSize;
	ULONG ulMinSize;
	CString strMaxSize;
	ULONG ulMaxSize;
	UINT uAvailability;
	CString strExtension;
	UINT uComplete;
	CString strCodec;
	ULONG ulMinBitrate;
	ULONG ulMinLength;
	CString strTitle;
	CString strAlbum;
	CString strArtist;
	bool bMatchKeywords;
};


///////////////////////////////////////////////////////////////////////////////
// CSearchDlg dialog
class CSearchDlg : public CResizableDialog, public CLoggable
{
	DECLARE_DYNAMIC(CSearchDlg)

public:
	CSearchDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchDlg();
	
	enum { IDD = IDD_SEARCH };

	CSearchListCtrl searchlistctrl;
	CClosableTabCtrl searchselect;

	void	Localize();

	bool	DoNewSearch(SSearchParams* pParams);
	bool	DoNewKadSearch(SSearchParams* pParams);
	void	CancelSearch();

	void	DownloadSelected();
	void	DownloadSelected(bool paused);

	bool	CanDeleteSearch(uint32 nSearchID) const;
	bool	CanDeleteAllSearches() const;
	void	DeleteSearch(uint32 nSearchID);
	void	DeleteAllSearchs();

	void	LocalSearchEnd(uint16 count, bool bMoreResultsAvailable);
	void	AddUDPResult(uint16 count);

	bool	CreateNewTab(SSearchParams* pParams);
	void	ShowSearchSelector(bool visible);
	uint8	GetSelectedCat()							{return m_cattabs.GetCurSel();}
	void	UpdateCatTabs();
	void	SaveAllSettings();
	BOOL	SaveSearchStrings();
	LRESULT ProcessJigleSearchResponse(WPARAM wParam, LPARAM lParam);
	void	ResetHistory();

protected:
	void StartNewSearch(bool bKademlia = false);
	void StartNewSearchKad();
	CString	CreateWebQuery();
	void UpdateControls();
	void ShowResults(const SSearchParams* pParams);
	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclkSearchlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeSearchname();
	afx_msg void OnDestroy();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelendokCombo1();
	afx_msg void OnBnClickedMore();
	afx_msg void OnSysColorChange();
	afx_msg void OnBnClickedStarts();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedCancels();
	afx_msg void OnBnClickedSdownload();
	afx_msg void OnBnClickedClearall();
	afx_msg void OnBnClickedSearchReset();
	afx_msg void OnSearchKeyDown();
	afx_msg void OnDDClicked();
	afx_msg LRESULT OnCloseTab(WPARAM wparam, LPARAM lparam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();

private:
	Packet*		searchpacket;
	UINT_PTR	global_search_timer;
	UINT		m_uTimerLocalServer;
	CProgressCtrl searchprogress;
	bool		canceld;
	uint16		servercount;
	bool		globsearch;
	uint32		m_nSearchID;
	CRichEditCtrlX m_ctlName;
	CComboBoxEx2 methodBox;
	CComboBox	Stypebox;
	CEditableListCtrl m_ctlOpts;
	CImageList	m_imlSearchResults;
	CTabCtrl	m_cattabs;
	CCustomAutoComplete* m_pacSearchString;
	HICON		icon_search;
	CIconStatic m_ctrlSearchFrm;
	CImageList	m_imlSearchMethods;
	LCID		m_uLangID;
	CButton		m_ctlMore;
	int			m_iSentMoreReq;
	CJigleSOAPThread* m_pJigleThread;

	CString ToQueryString(CString str);
	bool DoNewJigleSearch(SSearchParams* pParams);
};

bool GetSearchPacket(CSafeMemFile* data,
					 const CString& strSearchString, const CString& strLocalizedType,
					 ULONG ulMinSize, ULONG ulMaxSize, UINT uAvailability, const CString& strExtension, 
					 UINT uComplete, ULONG ulMinBitrate, ULONG ulMinLength, const CString& strCodec,
					 const CString& strTitle, const CString& strAlbum, const CString& strArtist,
					 bool bAllowEmptySearchString, bool bEd2k);
