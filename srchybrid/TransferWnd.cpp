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
#include "emule.h"
#include "SearchDlg.h"
#include "TransferWnd.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "MenuCmds.h"
#include "PartFile.h"
#include "CatDialog.h"
#include "InputBox.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CTransferWnd dialog

IMPLEMENT_DYNAMIC(CTransferWnd, CDialog)
CTransferWnd::CTransferWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CTransferWnd::IDD, pParent)
{
	icon_download = NULL;
	m_uWnd2 = DFLT_TRANSFER_WND2;
	m_pLastMousePoint.x = -1;
	m_pLastMousePoint.y = -1;
	m_nLastCatTT = -1;
}

CTransferWnd::~CTransferWnd()
{
	if (icon_download)
		VERIFY( DestroyIcon(icon_download) );
}

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableDialog)
	ON_NOTIFY(LVN_HOTTRACK, IDC_UPLOADLIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_QUEUELIST, OnHoverUploadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_DOWNLOADLIST, OnHoverDownloadList)
	ON_NOTIFY(LVN_HOTTRACK, IDC_CLIENTLIST , OnHoverUploadList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DLTAB, OnTcnSelchangeDltab)
	ON_NOTIFY(NM_RCLICK, IDC_DLTAB, OnNMRclickDltab)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_DOWNLOADLIST, OnLvnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_DOWNLOADLIST, OnLvnKeydownDownloadlist)
	ON_NOTIFY(UM_TABMOVED, IDC_DLTAB, OnTabMovement)
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()


BOOL CTransferWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	uploadlistctrl.Init();
	downloadlistctrl.Init();
	queuelistctrl.Init();
	clientlistctrl.Init();

	if (thePrefs.GetRestoreLastMainWndDlg())
		m_uWnd2 = thePrefs.GetTransferWnd2();
	ShowWnd2(m_uWnd2);

	SetAllIcons();

    Localize(); // i_a 

	m_uplBtn.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_uplBtn.SetFlat();
	m_uplBtn.DrawFlatFocus(TRUE);
	m_uplBtn.SetLeftAlign(true);
	
	AddAnchor(IDC_DOWNLOADLIST,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition() ));
	AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_UPLOAD_ICO,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUECOUNT,BOTTOM_LEFT);
	AddAnchor(IDC_TSTATIC1,BOTTOM_LEFT);
	AddAnchor(IDC_QUEUE_REFRESH_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDC_DLTAB,CSize(50,0) ,TOP_RIGHT);

	// splitting functionality
	CRect rc,rcSpl,rcDown;

	GetWindowRect(rc);
	ScreenToClient(rc);

	rcSpl=rc; rcSpl.top=rc.bottom-100 ; rcSpl.bottom=rcSpl.top+5;rcSpl.left=55;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER);
	SetInitLayout();
	
	//cats
	rightclickindex=-1;

	downloadlistactive=true;
	m_bIsDragging=false;

	// show & cat-tabs
	_stprintf(thePrefs.GetCategory(0)->title, _T("%s"), GetCatTitle(thePrefs.GetCategory(0)->filter));
	_stprintf(thePrefs.GetCategory(0)->incomingpath, _T("%s"), thePrefs.GetIncomingDir());
	thePrefs.GetCategory(0)->care4all=true;

	for (int ix=0;ix<thePrefs.GetCatCount();ix++)
		m_dlTab.InsertItem(ix,thePrefs.GetCategory(ix)->title );

	// create tooltip control for download categories
	m_tooltipCats.Create(this, TTS_NOPREFIX);
	m_dlTab.SetToolTips(&m_tooltipCats);
	UpdateCatTabTitles();
	UpdateTabToolTips();
	m_tooltipCats.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_tooltipCats.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_tooltipCats.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	m_tooltipCats.Activate(TRUE);

	UpdateListCount(m_uWnd2);
	VerifyCatTabSize();

	return true;
}

void CTransferWnd::ShowQueueCount(uint32 number){
	TCHAR buffer[100];
	_stprintf(buffer,_T("%u (%u ") + GetResString(IDS_BANNED).MakeLower() + _T(")"), number,theApp.clientlist->GetBannedCount() );
	GetDlgItem(IDC_QUEUECOUNT)->SetWindowText(buffer);
}

void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPLOADLIST, uploadlistctrl);
	DDX_Control(pDX, IDC_DOWNLOADLIST, downloadlistctrl);
	DDX_Control(pDX, IDC_QUEUELIST, queuelistctrl);
	DDX_Control(pDX, IDC_CLIENTLIST, clientlistctrl);
	DDX_Control(pDX, IDC_UPLOAD_ICO, m_uplBtn);
	DDX_Control(pDX, IDC_DLTAB, m_dlTab);
}

void CTransferWnd::SetInitLayout() {
	CRect rcDown,rcSpl,rcW;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	LONG splitpos=(thePrefs.GetSplitterbarPosition()*rcW.Height())/100;

	GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=splitpos-5;
	downloadlistctrl.MoveWindow(rcDown);
	
	GetDlgItem(IDC_UPLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	uploadlistctrl.MoveWindow(rcDown);

	GetDlgItem(IDC_QUEUELIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	queuelistctrl.MoveWindow(rcDown);

	GetDlgItem(IDC_CLIENTLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);
	rcDown.right=rcW.right-7;
	rcDown.bottom=rcW.bottom-20;
	rcDown.top=splitpos+20;
	clientlistctrl.MoveWindow(rcDown);

	rcSpl=rcDown;
	rcSpl.top=rcDown.bottom+4;rcSpl.bottom=rcSpl.top+7;rcSpl.left=(rcDown.right/2)-50;rcSpl.right=rcSpl.left+100;
	m_wndSplitter.MoveWindow(rcSpl,true);

	DoResize(0);
}

void CTransferWnd::DoResize(int delta)
{
	CSplitterControl::ChangeHeight(&downloadlistctrl, delta);
	CSplitterControl::ChangeHeight(&uploadlistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&queuelistctrl, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(&clientlistctrl, -delta, CW_BOTTOMALIGN);

	UpdateSplitterRange();

	Invalidate();
	UpdateWindow();
}

// setting splitter range limits
void CTransferWnd::UpdateSplitterRange()
{
	CRect rcDown,rcUp,rcW,rcSpl;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
	ScreenToClient(rcDown);

	GetDlgItem(IDC_UPLOADLIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	GetDlgItem(IDC_QUEUELIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	GetDlgItem(IDC_CLIENTLIST)->GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	thePrefs.SetSplitterbarPosition((rcDown.bottom*100)/rcW.Height());

	RemoveAnchor(IDC_DOWNLOADLIST);
	RemoveAnchor(IDC_UPLOADLIST);
	RemoveAnchor(IDC_QUEUELIST);
	RemoveAnchor(IDC_CLIENTLIST);
	AddAnchor(IDC_DOWNLOADLIST,TOP_LEFT,CSize(100,thePrefs.GetSplitterbarPosition() ));
	AddAnchor(IDC_UPLOADLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_QUEUELIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);
	AddAnchor(IDC_CLIENTLIST,CSize(0,thePrefs.GetSplitterbarPosition()),BOTTOM_RIGHT);

	m_wndSplitter.SetRange(rcDown.top+50 , rcUp.bottom-40);
}

LRESULT CTransferWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
		// arrange transferwindow layout
		case WM_PAINT:
			if (m_wndSplitter) {
				CRect rcDown,rcSpl,rcW;

				GetWindowRect(rcW);
				ScreenToClient(rcW);

				GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
				ScreenToClient(rcDown);

				if (rcW.Height()>0) {
					// splitter paint update
					rcSpl=rcDown;
					rcSpl.top=rcDown.bottom+8;rcSpl.bottom=rcSpl.top+5;rcSpl.left=190;
					GetDlgItem(IDC_UPLOAD_ICO)->MoveWindow(10,rcSpl.top-4,170,18);
					m_wndSplitter.MoveWindow(rcSpl,true);
					UpdateSplitterRange();
				}
			}
			break;
		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER)
			{	
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize(pHdr->delta);
			}
			break;
		case WM_WINDOWPOSCHANGED : 
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);

				if (m_wndSplitter && rcW.Height()>0) Invalidate();
				break;
			}
		case WM_SIZE:
			if (m_wndSplitter) {
				CRect rcDown,rcSpl,rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (rcW.Height()>0){
					GetDlgItem(IDC_DOWNLOADLIST)->GetWindowRect(rcDown);
					ScreenToClient(rcDown);

					long splitpos=(thePrefs.GetSplitterbarPosition()*rcW.Height())/100;

					rcSpl.right=rcDown.right;rcSpl.top=splitpos+10;rcSpl.bottom=rcSpl.top+7;rcSpl.left=(rcDown.right/2)-50;rcSpl.right=rcSpl.left+100;
					m_wndSplitter.MoveWindow(rcSpl,true);
				}
			}
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

// CTransferWnd message handlers
BOOL CTransferWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message== WM_LBUTTONDBLCLK && pMsg->hwnd== GetDlgItem(IDC_DLTAB)->m_hWnd) {
		OnDblclickDltab();
		return TRUE;
	}

	if (pMsg->message==WM_MOUSEMOVE) {
		POINT point;
		::GetCursorPos(&point);
		if (point.x!=m_pLastMousePoint.x || point.y!=m_pLastMousePoint.y) {
			m_pLastMousePoint=point;
			// handle tooltip updating, when mouse is moved from one item to another
			CPoint pt(point);
			m_nDropIndex=GetTabUnderMouse(&pt);
			if (m_nDropIndex!=m_nLastCatTT) {
				m_nLastCatTT=m_nDropIndex;
			    if (m_nDropIndex!=-1)
				    UpdateTabToolTips(m_nDropIndex);
			    //m_tooltipCats.Update();
			}
		}
	}

	if (pMsg->message == WM_MBUTTONUP){
		if (downloadlistactive)
			downloadlistctrl.ShowSelectedFileDetails();
		else{
			switch (m_uWnd2){
				case 2:
					queuelistctrl.ShowSelectedUserDetails();
					break;
				case 1:
					uploadlistctrl.ShowSelectedUserDetails();
					break;
				case 0:
					clientlistctrl.ShowSelectedUserDetails();
					break;
			}
		}
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

int CTransferWnd::GetItemUnderMouse(CListCtrl* ctrl)
{
	CPoint pt;
	::GetCursorPos(&pt);
	ctrl->ScreenToClient(&pt);
	LVHITTESTINFO hit, subhit;
	hit.pt = pt;
	subhit.pt = pt;
	ctrl->SubItemHitTest(&subhit);
	int sel = ctrl->HitTest(&hit);
	if (sel != LB_ERR && (hit.flags & LVHT_ONITEM))
	{
		if (subhit.iSubItem == 0)
			return sel;
	}
	return LB_ERR;
}

void CTransferWnd::UpdateListCount(uint8 listindex, int iCount /*=-1*/)
{
	if (m_uWnd2 != listindex)
		return;

	CString buffer;
	switch (m_uWnd2){
        case 1: {
            uint32 itemCount = iCount == -1 ? uploadlistctrl.GetItemCount() : iCount;
            uint32 activeCount = theApp.uploadqueue->GetActiveUploadsCount();
            if(activeCount >= itemCount) {
                buffer.Format(_T(" (%i)"), itemCount);
            } else {
                buffer.Format(_T(" (%i/%i)"), activeCount, itemCount);
            }
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS)+buffer);
			break;
        }
		case 2:
			buffer.Format(_T(" (%i)"), iCount == -1 ? queuelistctrl.GetItemCount() : iCount);
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE)+buffer);
			break;
		default:
			buffer.Format(_T(" (%i)"), iCount == -1 ? clientlistctrl.GetItemCount() : iCount);
			GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST)+buffer);
	}
}

void CTransferWnd::SwitchUploadList()
{
	if( m_uWnd2 == 1){
		SetWnd2(2);
		if( thePrefs.IsQueueListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		queuelistctrl.Visable();
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE));
	}
	else if( m_uWnd2 == 2){
		SetWnd2(0);
		if( thePrefs.IsKnownClientListDisabled()){
			SwitchUploadList();
			return;
		}
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST));
	}
	else{
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		SetWnd2(1);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
	}
	UpdateListCount(m_uWnd2);
	SetWnd2Icon();
}

void CTransferWnd::ShowWnd2(uint8 uWnd2)
{
	if (uWnd2 == 2 && !thePrefs.IsQueueListDisabled())
	{
		uploadlistctrl.Hide();
		clientlistctrl.Hide();
		queuelistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_ONQUEUE));
		SetWnd2(uWnd2);
	}
	else if (uWnd2 == 0 && !thePrefs.IsKnownClientListDisabled())
	{
		uploadlistctrl.Hide();
		queuelistctrl.Hide();
		clientlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_CLIENTLIST));
		SetWnd2(uWnd2);
	}
	else
	{
		queuelistctrl.Hide();
		clientlistctrl.Hide();
		uploadlistctrl.Visable();
		GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
		SetWnd2(1);
	}
	SetWnd2Icon();
}

void CTransferWnd::SetWnd2(uint8 uWnd2)
{
	m_uWnd2 = uWnd2;
	thePrefs.SetTransferWnd2(m_uWnd2);
}

void CTransferWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CTransferWnd::SetAllIcons()
{
	if (icon_download)
		VERIFY( DestroyIcon(icon_download) );
	icon_download = theApp.LoadIcon(_T("Download"), 16, 16);
	((CStatic*)GetDlgItem(IDC_DOWNLOAD_ICO))->SetIcon(icon_download);
	SetWnd2Icon();
}

void CTransferWnd::SetWnd2Icon()
{
	if (m_uWnd2 == 2)
		m_uplBtn.SetIcon(_T("ClientsOnQueue"));
	else if (m_uWnd2 == 1)
		m_uplBtn.SetIcon(_T("Upload"));
	else
		m_uplBtn.SetIcon(_T("ClientsKnown"));
}

void CTransferWnd::Localize()
{
	GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(GetResString(IDS_TW_DOWNLOADS));
	GetDlgItem(IDC_UPLOAD_ICO)->SetWindowText(GetResString(IDS_TW_UPLOADS));
	GetDlgItem(IDC_TSTATIC1)->SetWindowText(GetResString(IDS_TW_QUEUE));
	GetDlgItem(IDC_QUEUE_REFRESH_BUTTON)->SetWindowText(GetResString(IDS_SV_UPDATE));

	uploadlistctrl.Localize();
	queuelistctrl.Localize();
	downloadlistctrl.Localize();
	clientlistctrl.Localize();

	UpdateListCount(m_uWnd2);
}

void CTransferWnd::OnBnClickedQueueRefreshButton()
{
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);

	while( update ){
		theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient( update);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}

void CTransferWnd::OnHoverUploadList(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistactive=false;
	*pResult = 0;
}

void CTransferWnd::OnHoverDownloadList(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistactive=true;
	*pResult = 0;
}

void CTransferWnd::OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	downloadlistctrl.ChangeCategory(m_dlTab.GetCurSel());
	*pResult = 0;
}

// Ornis' download categories
void CTransferWnd::OnNMRclickDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	UINT flag;
	CTitleMenu menu;
	POINT point;
	::GetCursorPos(&point);

	CPoint pt(point);
	rightclickindex=GetTabUnderMouse(&pt);

	if (rightclickindex==-1)
		return;

	CMenu m_CatMenu;
	m_CatMenu.CreateMenu();

	CMenu m_PrioMenu;
	m_PrioMenu.CreateMenu();

// ZZ:DownloadManager -->
    Category_Struct* category_Struct = thePrefs.GetCategory(rightclickindex);

	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
    m_PrioMenu.CheckMenuItem(MP_PRIOLOW, category_Struct && category_Struct->prio == PR_LOW ? MF_CHECKED : MF_UNCHECKED);

	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
    m_PrioMenu.CheckMenuItem(MP_PRIONORMAL, category_Struct && category_Struct->prio != PR_LOW && category_Struct->prio != PR_HIGH ? MF_CHECKED : MF_UNCHECKED);

	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
    m_PrioMenu.CheckMenuItem(MP_PRIOHIGH, category_Struct && category_Struct->prio == PR_HIGH ? MF_CHECKED : MF_UNCHECKED);

    //m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
// <-- ZZ:DownloadManager


	menu.CreatePopupMenu();
	if (rightclickindex)
		menu.AddMenuTitle(GetResString(IDS_CAT)+_T(" (")+thePrefs.GetCategory(rightclickindex)->title+_T(")") ,true );
	else
		menu.AddMenuTitle(GetResString(IDS_CAT),true);

	m_isetcatmenu=rightclickindex;
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0,GetResString(IDS_ALL) );

	flag=(!thePrefs.GetCategory(rightclickindex)->care4all && rightclickindex ) ? MF_GRAYED:MF_STRING;
	m_CatMenu.AppendMenu(flag,MP_CAT_SET0+1,GetResString(IDS_ALLOTHERS) );

	// selector for regular expression view filter

	if (rightclickindex) {
		if (thePrefs.IsExtControlsEnabled())
			m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+18, GetResString(IDS_REGEXPRESSION) );

		flag=MF_STRING;
		if (thePrefs.GetCategory(rightclickindex)->care4all)
			flag=flag|MF_CHECKED | MF_BYCOMMAND;
		if (thePrefs.IsExtControlsEnabled() )
			m_CatMenu.AppendMenu(flag,MP_CAT_SET0+17,GetResString(IDS_CARE4ALL) );
	}

	m_CatMenu.AppendMenu(MF_SEPARATOR);
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+2,GetResString(IDS_STATUS_NOTCOMPLETED) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+3,GetResString(IDS_DL_TRANSFCOMPL) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+4,GetResString(IDS_WAITING) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+5,GetResString(IDS_DOWNLOADING) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+6,GetResString(IDS_ERRORLIKE) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+7,GetResString(IDS_PAUSED) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+8,GetResString(IDS_SEENCOMPL) );
	m_CatMenu.AppendMenu(MF_SEPARATOR);
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+10,GetResString(IDS_VIDEO) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+11,GetResString(IDS_AUDIO) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+12,GetResString(IDS_SEARCH_ARC) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+13,GetResString(IDS_SEARCH_CDIMG) );

	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+14,GetResString(IDS_SEARCH_DOC) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+15,GetResString(IDS_SEARCH_PICS) );
	m_CatMenu.AppendMenu(MF_STRING,MP_CAT_SET0+16,GetResString(IDS_SEARCH_PRG) );

	if (thePrefs.IsExtControlsEnabled()) {
		m_CatMenu.AppendMenu(MF_SEPARATOR);
		m_CatMenu.AppendMenu( thePrefs.GetCatFilter(rightclickindex)>0?MF_STRING:MF_GRAYED,MP_CAT_SET0+19,GetResString(IDS_NEGATEFILTER) );
		if ( thePrefs.GetCatFilterNeg(rightclickindex))
			m_CatMenu.CheckMenuItem( MP_CAT_SET0+19 ,MF_CHECKED | MF_BYCOMMAND);
	}
	
	m_CatMenu.CheckMenuItem( MP_CAT_SET0+thePrefs.GetCatFilter(rightclickindex) ,MF_CHECKED | MF_BYCOMMAND);

	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CatMenu.m_hMenu, GetResString(IDS_CHANGECATVIEW),_T("SEARCHPARAMS") );
	menu.AppendMenu(MF_SEPARATOR);

	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY), _T("FILEPRIORITY") );

	menu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL),_T("DELETE") );
	menu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_DL_STOP),_T("STOP"));
	menu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	menu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	menu.AppendMenu(MF_STRING,MP_RESUMENEXT, GetResString(IDS_DL_RESUMENEXT), _T("RESUME"));
	
// ZZ:DownloadManager -->
    if(rightclickindex != 0 && thePrefs.IsExtControlsEnabled()) {
        menu.AppendMenu(MF_STRING,MP_DOWNLOAD_ALPHABETICAL, GetResString(IDS_DOWNLOAD_ALPHABETICAL));	
        menu.CheckMenuItem(MP_DOWNLOAD_ALPHABETICAL, category_Struct && category_Struct->downloadInAlphabeticalOrder ? MF_CHECKED : MF_UNCHECKED);
    }
// <-- ZZ:DownloadManager

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC), _T("FOLDERS") );

	flag=(rightclickindex==0) ? MF_GRAYED:MF_STRING;
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD));
	menu.AppendMenu(flag,MP_CAT_EDIT,GetResString(IDS_CAT_EDIT));
	menu.AppendMenu(flag,MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE));

	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( m_PrioMenu.DestroyMenu() );
	VERIFY( m_CatMenu.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );

	*pResult = 0;
}

void CTransferWnd::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    int iSel = downloadlistctrl.GetSelectionMark();
	if (iSel==-1) return;
	if (((CtrlItem_Struct*)downloadlistctrl.GetItemData(iSel))->type != FILE_TYPE) return;
	
	m_bIsDragging = true;

	POINT pt;
	::GetCursorPos(&pt);

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	m_nDragIndex = pNMLV->iItem;
	m_pDragImage = downloadlistctrl.CreateDragImage( downloadlistctrl.GetSelectionMark() ,&pt);
    m_pDragImage->BeginDrag( 0, CPoint(0,0) );
    m_pDragImage->DragEnter( GetDesktopWindow(), pNMLV->ptAction );
    SetCapture();
	m_nDropIndex = -1;

	*pResult = 0;
}

void CTransferWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if( !(nFlags & MK_LBUTTON) ) m_bIsDragging = false;

	if (m_bIsDragging){
		CPoint pt(point);           //get our current mouse coordinates
		ClientToScreen(&pt);        //convert to screen coordinates

		m_nDropIndex=GetTabUnderMouse(&pt);

		if (m_nDropIndex>0 && thePrefs.GetCategory(m_nDropIndex)->care4all)	// not droppable
			m_dlTab.SetCurSel(-1);
		else
		m_dlTab.SetCurSel(m_nDropIndex);

		m_dlTab.Invalidate();
		
		::GetCursorPos(&pt);
		pt.y-=10;
		m_pDragImage->DragMove(pt); //move the drag image to those coordinates
	}
}

void CTransferWnd::OnLButtonUp(UINT nFlags, CPoint point)
{

	if (m_bIsDragging)
	{
		ReleaseCapture ();
		m_bIsDragging = false;
		m_pDragImage->DragLeave (GetDesktopWindow ());
		m_pDragImage->EndDrag ();
		delete m_pDragImage;
		
		if (m_nDropIndex>-1 && (downloadlistctrl.curTab==0 ||
				(downloadlistctrl.curTab>0 && m_nDropIndex!=downloadlistctrl.curTab) )) {

			CPartFile* file;

			// for multiselections
			CTypedPtrList <CPtrList,CPartFile*> selectedList; 
			POSITION pos = downloadlistctrl.GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = downloadlistctrl.GetNextSelectedItem(pos);
				if(index > -1 && (((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->type == FILE_TYPE))
					selectedList.AddTail( (CPartFile*)((CtrlItem_Struct*)downloadlistctrl.GetItemData(index))->value );
			}

			while (!selectedList.IsEmpty())
			{
				file = selectedList.GetHead();
				selectedList.RemoveHead();
				file->SetCategory(m_nDropIndex);
			}


			m_dlTab.SetCurSel(downloadlistctrl.curTab);
			//if (m_dlTab.GetCurSel()>0 || (thePrefs.GetAllcatType()==1 && m_dlTab.GetCurSel()==0) )
			downloadlistctrl.UpdateCurrentCategoryView();

			UpdateCatTabTitles();

		} else m_dlTab.SetCurSel(downloadlistctrl.curTab);
		downloadlistctrl.Invalidate();
	}
}

BOOL CTransferWnd::OnCommand(WPARAM wParam,LPARAM lParam ){ 

	// category filter menuitems
	if (wParam>=MP_CAT_SET0 && wParam<=MP_CAT_SET0+99) {
		if (wParam==MP_CAT_SET0+17) {
			thePrefs.GetCategory(m_isetcatmenu)->care4all=!thePrefs.GetCategory(m_isetcatmenu)->care4all;
		}else if (wParam==MP_CAT_SET0+19) {
			// negate
			thePrefs.SetCatFilterNeg(m_isetcatmenu, (!thePrefs.GetCatFilterNeg(m_isetcatmenu)) );
		} else {	// set the view filter
			if (wParam-MP_CAT_SET0<1)	// dont negate all filter
				thePrefs.SetCatFilterNeg(m_isetcatmenu, false);

			thePrefs.SetCatFilter(m_isetcatmenu,wParam-MP_CAT_SET0);
			m_nLastCatTT=-1;
		}

		// set to regexp but none is set for that category?
		if (wParam==MP_CAT_SET0+18 && thePrefs.GetCategory(m_isetcatmenu)->regexp.IsEmpty() ) {
			m_nLastCatTT=-1;
			CCatDialog dialog(rightclickindex);
			dialog.DoModal();

			// still no regexp?
			if (thePrefs.GetCategory(m_isetcatmenu)->regexp.IsEmpty()) {
				thePrefs.SetCatFilter(m_isetcatmenu,0);
			}

		}

		downloadlistctrl.UpdateCurrentCategoryView();
		EditCatTabLabel(m_isetcatmenu);
		thePrefs.SaveCats();
		return TRUE;
	}
	switch (wParam){ 
		case MP_CAT_ADD: {
			m_nLastCatTT=-1;
			int newindex=AddCategory(_T("?"),thePrefs.GetIncomingDir(),_T(""),_T(""),false);
			CCatDialog dialog(newindex);
			dialog.DoModal();
			if (dialog.WasCancelled())
				thePrefs.RemoveCat(newindex);
			else {
				theApp.emuledlg->searchwnd->UpdateCatTabs();
				m_dlTab.InsertItem(newindex,thePrefs.GetCategory(newindex)->title);
				EditCatTabLabel(newindex);
				thePrefs.SaveCats();
				VerifyCatTabSize();
			}
			break;
		}
		case MP_CAT_EDIT: {
			m_nLastCatTT=-1;
			CCatDialog dialog(rightclickindex);
			dialog.DoModal();

			CString csName;
			csName.Format(_T("%s"), thePrefs.GetCategory(rightclickindex)->title );
			EditCatTabLabel(rightclickindex,csName);
		
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			theApp.emuledlg->transferwnd->downloadlistctrl.UpdateCurrentCategoryView();
			thePrefs.SaveCats();
			break;
		}
		case MP_CAT_REMOVE: {
			m_nLastCatTT=-1;
			theApp.downloadqueue->ResetCatParts(rightclickindex);
			thePrefs.RemoveCat(rightclickindex);
			m_dlTab.DeleteItem(rightclickindex);
			m_dlTab.SetCurSel(0);
			downloadlistctrl.ChangeCategory(0);
			thePrefs.SaveCats();
			if (thePrefs.GetCatCount()==1) 
				thePrefs.GetCategory(0)->filter=0;
			theApp.emuledlg->searchwnd->UpdateCatTabs();
			VerifyCatTabSize();
			break;
		}
// ZZ:DownloadManager -->
		case MP_PRIOLOW: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_LOW;

			thePrefs.SaveCats();
			break;
		}
		case MP_PRIONORMAL: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_NORMAL;

			thePrefs.SaveCats();
			break;
		}
		case MP_PRIOHIGH: {
            thePrefs.GetCategory(rightclickindex)->prio = PR_HIGH;

			thePrefs.SaveCats();
			break;
		}
// <-- ZZ:DownloadManager

		case MP_PAUSE: {
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_PAUSE);
			break;
		}
		case MP_STOP : {
				theApp.downloadqueue->SetCatStatus(rightclickindex,MP_STOP);
			break;
		}
		case MP_CANCEL: {
			if (AfxMessageBox(GetResString(IDS_Q_CANCELDL),MB_ICONQUESTION|MB_YESNO) == IDYES)
				theApp.downloadqueue->SetCatStatus(rightclickindex,MP_CANCEL);
			break;
		}
		case MP_RESUME: {
			theApp.downloadqueue->SetCatStatus(rightclickindex,MP_RESUME);
			break;
		}
		case MP_RESUMENEXT: {
			theApp.downloadqueue->StartNextFile(rightclickindex,false);
			break;
		}

// ZZ:DownloadManager -->
		case MP_DOWNLOAD_ALPHABETICAL: {
            BOOL newSetting = !thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder;
            thePrefs.GetCategory(rightclickindex)->downloadInAlphabeticalOrder = newSetting;
			thePrefs.SaveCats();
            if(newSetting) {
                // any auto prio files will be set to normal now.
                theApp.downloadqueue->RemoveAutoPrioInCat(rightclickindex, PR_NORMAL);
            }

            break;
		}
// <-- ZZ:DownloadManager

		case IDC_UPLOAD_ICO: {
			SwitchUploadList();
			break;
		}
		case IDC_QUEUE_REFRESH_BUTTON: {
			OnBnClickedQueueRefreshButton();
			break;
		}
									   
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetCategory(m_isetcatmenu)->incomingpath,NULL, NULL, SW_SHOW);
			break;

	}
	return TRUE;
}

void CTransferWnd::UpdateCatTabTitles(bool force) {


	CPoint pt;
	::GetCursorPos(&pt);
	if (!force && GetTabUnderMouse(&pt)!=-1)		// avoid cat tooltip jumping
		return;

	for (uint8 i=0;i<m_dlTab.GetItemCount();i++)
		EditCatTabLabel(i,/*(i==0)? GetCatTitle( thePrefs.GetCategory(0)->filter ):*/thePrefs.GetCategory(i)->title);
}

void CTransferWnd::EditCatTabLabel(int i) {
	EditCatTabLabel(i,/*(i==0)? GetCatTitle( thePrefs.GetAllcatType() ):*/thePrefs.GetCategory(i)->title);
}

void CTransferWnd::EditCatTabLabel(int index,CString newlabel) {

	TCITEM tabitem;
	tabitem.mask = TCIF_PARAM;
	m_dlTab.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;

	newlabel.Replace(_T("&"),_T("&&"));

	if (!index)
		newlabel.Empty();

	if (!index || (index && thePrefs.GetCatFilter(index)>0)) {

		if (index)
			newlabel.Append(_T(" (")) ;
		
		if (thePrefs.GetCatFilterNeg(index))
			newlabel.Append(_T("!"));			
 
		if (thePrefs.GetCatFilter(index)==18)
			newlabel.Append( _T("\"") + thePrefs.GetCategory(index)->regexp + _T("\"") );
		else
        	newlabel.Append( GetCatTitle(thePrefs.GetCatFilter(index)));

		if (index)
			newlabel.Append( _T(")") );
	}

	int count,dwl;
	if (thePrefs.ShowCatTabInfos()) {
		CPartFile* cur_file;
		count=dwl=0;
		for (int i=0;i<theApp.downloadqueue->GetFileCount();i++) {
			cur_file=theApp.downloadqueue->GetFileByIndex(i);
			if (cur_file==0) continue;
			if (cur_file->CheckShowItemInGivenCat(index)) {
				if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			}
		}
		CString title=newlabel;
		theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(index, count);
		newlabel.Format(_T("%s %i/%i"),title,dwl,count); // ZZ:DownloadManager
	}

	tabitem.pszText = newlabel.LockBuffer();
	m_dlTab.SetItem(index,&tabitem);
	newlabel.UnlockBuffer();

	VerifyCatTabSize();
}

int CTransferWnd::AddCategory(CString newtitle,CString newincoming,CString newcomment, CString newautocat, bool addTab){
	Category_Struct* newcat=new Category_Struct;

	_stprintf(newcat->title,newtitle);
	newcat->prio=PR_NORMAL; // ZZ:DownloadManager
	_stprintf(newcat->incomingpath,newincoming);
	_stprintf(newcat->comment,newcomment);
	newcat->regexp.Empty();
	newcat->ac_regexpeval=false;
	newcat->autocat=newautocat;
    newcat->downloadInAlphabeticalOrder = FALSE; // ZZ:DownloadManager
	newcat->filter=0;
	newcat->filterNeg=false;
	newcat->care4all=false;

	int index=thePrefs.AddCat(newcat);
	if (addTab) m_dlTab.InsertItem(index,newtitle);
	VerifyCatTabSize();
	
	return index;
}

int CTransferWnd::GetTabUnderMouse(CPoint* point) {

		TCHITTESTINFO hitinfo;
		CRect rect;
		m_dlTab.GetWindowRect(&rect);
		point->Offset(0-rect.left,0-rect.top);
		hitinfo.pt = *point;

		if( m_dlTab.GetItemRect( 0, &rect ) )
			if (hitinfo.pt.y< rect.top+30 && hitinfo.pt.y >rect.top-30)
				hitinfo.pt.y = rect.top;

		// Find the destination tab...
		unsigned int nTab = m_dlTab.HitTest( &hitinfo );

		if( hitinfo.flags != TCHT_NOWHERE )
			return nTab;
		else return -1;
}

void CTransferWnd::OnLvnKeydownDownloadlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int iItem = downloadlistctrl.GetSelectionMark();
	if (iItem != -1)
	{
		bool bAltKey = GetAsyncKeyState(VK_MENU) < 0;
		int iAction = EXPAND_COLLAPSE;
		if (pLVKeyDow->wVKey==VK_ADD || (bAltKey && pLVKeyDow->wVKey==VK_RIGHT))
			iAction = EXPAND_ONLY;
		else if (pLVKeyDow->wVKey==VK_SUBTRACT || (bAltKey && pLVKeyDow->wVKey==VK_LEFT))
			iAction = COLLAPSE_ONLY;
		if (iAction < EXPAND_COLLAPSE)
			downloadlistctrl.ExpandCollapseItem(iItem, iAction, true);
	}
	*pResult = 0;
}

void CTransferWnd::UpdateTabToolTips(int tab)
{
	uint8 i;

	if (tab==-1) {

		for (i=0;i<m_tooltipCats.GetToolCount();i++)
			m_tooltipCats.DelTool(&m_dlTab,i+1);

		for (i = 0; i < m_dlTab.GetItemCount(); i++)
		{
			CRect r;
			m_dlTab.GetItemRect(i, &r);
			VERIFY(m_tooltipCats.AddTool(&m_dlTab, GetTabStatistic(i), &r, i+1));
		}
	} else {
		CRect r;
		m_dlTab.GetItemRect(tab, &r);

		m_tooltipCats.DelTool(&m_dlTab,tab+1);
		VERIFY(m_tooltipCats.AddTool(&m_dlTab, GetTabStatistic(tab), &r, tab+1));
	}
}

CString CTransferWnd::GetTabStatistic(uint8 tab) {
	uint16 count,dwl,err,compl,paus;
	count=dwl=err=compl=paus=0;
	float speed=0;
	uint64 size=0;
	uint64 trsize=0;
	uint64 disksize=0;

	CPartFile* cur_file;

	for (int i=0;i<theApp.downloadqueue->GetFileCount();++i) {
		cur_file=theApp.downloadqueue->GetFileByIndex(i);
		if (cur_file==0) continue;
		if (cur_file->CheckShowItemInGivenCat(tab)) {
			count++;
			if (cur_file->GetTransferringSrcCount()>0) ++dwl;
			speed+=cur_file->GetDatarate()/1024.0f;
			size+=cur_file->GetFileSize();
			trsize+=cur_file->GetCompletedSize();
			disksize+=cur_file->GetRealFileSize();
			if (cur_file->GetStatus()==PS_ERROR) ++err;
			if (cur_file->GetStatus()==PS_PAUSED) ++paus;
		}
	}

	int total;
	compl=theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(tab,total);

// ZZ:DownloadManager -->
    CString prio;
    switch(thePrefs.GetCategory(tab)->prio) {
        case PR_LOW:
            prio = GetResString(IDS_PRIOLOW);
            break;

        case PR_HIGH:
            prio = GetResString(IDS_PRIOHIGH);
            break;

        default:
            prio = GetResString(IDS_PRIONORMAL);
            break;
    }
// ZZ:DownloadManager <--

	CString title;
	title.Format(_T("%s: %i\n\n%s: %i\n%s: %i\n%s: %i\n%s: %i\n\n%s: %s\n\n%s: %.1f %s\n%s: %s/%s\n%s%s"), // ZZ:DownloadManager
		
		GetResString(IDS_FILES), count+compl,
		GetResString(IDS_DOWNLOADING), dwl,
		GetResString(IDS_PAUSED) ,paus,
		GetResString(IDS_ERRORLIKE) ,err,
		GetResString(IDS_DL_TRANSFCOMPL) ,compl,

        GetResString(IDS_PRIORITY), prio, // ZZ:DownloadManager

		GetResString(IDS_DL_SPEED) ,speed,GetResString(IDS_KBYTESEC),


		GetResString(IDS_DL_SIZE),CastItoXBytes(trsize, false, false),CastItoXBytes(size, false, false),
		GetResString(IDS_ONDISK),CastItoXBytes(disksize, false, false));
	return title;
}


void CTransferWnd::OnDblclickDltab(){
	POINT point;
	::GetCursorPos(&point);
	CPoint pt(point);
	int tab=GetTabUnderMouse(&pt);
	if (tab<1) return;
	rightclickindex=tab;
	OnCommand(MP_CAT_EDIT,0);
}

void CTransferWnd::OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult) {
	UINT from=m_dlTab.GetLastMovementSource();
	UINT to=m_dlTab.GetLastMovementDestionation();

	if (from==0 || to==0 || from==to-1) return;

	// do the reorder
	
	// rearrange the cat-map
	if (!thePrefs.MoveCat(from,to)) return;

	// update partfile-stored assignment
	theApp.downloadqueue->MoveCat((uint8)from,(uint8)to);

	// move category of completed files
	downloadlistctrl.MoveCompletedfilesCat((uint8)from,(uint8)to);

	// of the tabcontrol itself
	m_dlTab.ReorderTab(from,to);

	UpdateCatTabTitles();
	theApp.emuledlg->searchwnd->UpdateCatTabs();

	if (to>from) --to;
	m_dlTab.SetCurSel(to);
	downloadlistctrl.ChangeCategory(to);
}

void CTransferWnd::VerifyCatTabSize() {
	CRect rect;
	int size=0;
	int right;

	for (int i=0;i<m_dlTab.GetItemCount();i++) {
		m_dlTab.GetItemRect(i,&rect);
		size+= rect.Width();
	}
	size+=20;

	WINDOWPLACEMENT wpTabWinPos;

	downloadlistctrl.GetWindowPlacement(&wpTabWinPos);
	right=wpTabWinPos.rcNormalPosition.right;

	m_dlTab.GetWindowPlacement(&wpTabWinPos);
	if (wpTabWinPos.rcNormalPosition.right<0) return;

	wpTabWinPos.rcNormalPosition.right=right;
	int left=wpTabWinPos.rcNormalPosition.right-size;
	if (left<200) left=200;
	wpTabWinPos.rcNormalPosition.left=left;

	RemoveAnchor(m_dlTab);
	m_dlTab.SetWindowPlacement(&wpTabWinPos);
	AddAnchor(m_dlTab,TOP_RIGHT);
}

CString CTransferWnd::GetCatTitle(int catid)
{
	switch (catid) {
		case 0 : return GetResString(IDS_ALL);
		case 1 : return GetResString(IDS_ALLOTHERS);
		case 2 : return GetResString(IDS_STATUS_NOTCOMPLETED);
		case 3 : return GetResString(IDS_DL_TRANSFCOMPL);
		case 4 : return GetResString(IDS_WAITING);
		case 5 : return GetResString(IDS_DOWNLOADING);
		case 6 : return GetResString(IDS_ERRORLIKE);
		case 7 : return GetResString(IDS_PAUSED);
		case 8 : return GetResString(IDS_SEENCOMPL);
		case 10 : return GetResString(IDS_VIDEO);
		case 11 : return GetResString(IDS_AUDIO);
		case 12 : return GetResString(IDS_SEARCH_ARC);
		case 13 : return GetResString(IDS_SEARCH_CDIMG);

		case 14 : return GetResString(IDS_SEARCH_DOC);
		case 15 : return GetResString(IDS_SEARCH_PICS);
		case 16 : return GetResString(IDS_SEARCH_PRG);
//		case 18 : return GetResString(IDS_REGEXPRESSION);
	}
	return _T("?");
}
