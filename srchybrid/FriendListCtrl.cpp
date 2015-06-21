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
#include "FriendListCtrl.h"
#include "friend.h"
#include "ClientDetailDialog.h"
#include "Addfriend.h"
#include "FriendList.h"
#include "emuledlg.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "ListenSocket.h"
#include "MenuCmds.h"
#include "ChatWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CFriendListCtrl

IMPLEMENT_DYNAMIC(CFriendListCtrl, CMuleListCtrl)
CFriendListCtrl::CFriendListCtrl()
{
}

CFriendListCtrl::~CFriendListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFriendListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
END_MESSAGE_MAP()



// CFriendListCtrl message handlers

void CFriendListCtrl::Init()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	RECT rcWindow;
	GetWindowRect(&rcWindow);
	InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, rcWindow.right - rcWindow.left - 4, 0);
	SetAllIcons();
	theApp.friendlist->SetWindow(this);
	SetSortArrow(0, true);
}

void CFriendListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CFriendListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader("FriendNoClient"));
	iml.Add(CTempIconLoader("FriendWithClient"));
	iml.Add(CTempIconLoader("FriendConnected"));
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himlOld = ApplyImageList(iml.Detach());
	if (himlOld)
		ImageList_Destroy(himlOld);
}

void CFriendListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();
}

void CFriendListCtrl::AddFriend(const CFriend* toadd)
{
	int itemnr = GetItemCount();
	InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,itemnr,(LPCTSTR)toadd->m_strName,0,0,1,(LPARAM)toadd);
	RefreshFriend(toadd);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RemoveFriend(const CFriend* toremove)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toremove;
	int result = FindItem(&find);
	if (result != -1)
		DeleteItem(result);
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
}

void CFriendListCtrl::RefreshFriend(const CFriend* toupdate)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)toupdate;
	int itemnr = FindItem(&find);
	if (itemnr != -1)
	{
		SetItemText(itemnr,0,(LPCTSTR)toupdate->m_strName);
		int image;
		if (!toupdate->m_LinkedClient)
			image = 0;
		else if (toupdate->m_LinkedClient->socket && toupdate->m_LinkedClient->socket->IsConnected())
			image = 2;
		else
			image = 1;
		SetItem(itemnr,0,LVIF_IMAGE,0,image,0,0,0,0);
	}
	else
		ASSERT(0);
}

void CFriendListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_FRIENDLIST));

	const CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		cur_friend = (CFriend*)GetItemData(iSel);
		ClientMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS));
		ClientMenu.SetDefaultItem(MP_DETAIL);
	}

	ClientMenu.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_ADDAFRIEND));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND));
	ClientMenu.AppendMenu(MF_STRING | (cur_friend ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG));
	ClientMenu.AppendMenu(MF_STRING | ((cur_friend==NULL || (cur_friend && cur_friend->m_LinkedClient && !cur_friend->m_LinkedClient->GetViewSharedFilesSupport())) ? MF_GRAYED : MF_ENABLED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
	ClientMenu.AppendMenu(MF_STRING, MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT));
	if (cur_friend && cur_friend->m_LinkedClient && !cur_friend->m_LinkedClient->HasLowID()){
		ClientMenu.EnableMenuItem(MP_FRIENDSLOT, MF_ENABLED);
		ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (cur_friend->m_LinkedClient->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	}
	else
		ClientMenu.EnableMenuItem(MP_FRIENDSLOT, MF_GRAYED);

	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CFriendListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CFriend* cur_friend = NULL;
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		cur_friend = (CFriend*)GetItemData(iSel);
	
	switch (wParam){
		case MP_MESSAGE:{
			if (cur_friend){
				if (cur_friend->m_LinkedClient)
					theApp.emuledlg->chatwnd->StartSession(cur_friend->m_LinkedClient);
				else{
					CUpDownClient* chatclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					chatclient->SetUserName(cur_friend->m_strName.GetBuffer());
					theApp.clientlist->AddClient(chatclient);
					theApp.emuledlg->chatwnd->StartSession(chatclient);
				}
			}
			break;
		}
		case MP_REMOVEFRIEND:{
			if (cur_friend){
				theApp.friendlist->RemoveFriend(cur_friend);
				// auto select next item after deleted one.
				if (iSel < GetItemCount()){
					SetSelectionMark(iSel);
					SetItemState(iSel, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			break;
		}
		case MP_ADDFRIEND:{
			CAddFriend dialog2; 
			dialog2.DoModal();
			break;
		}
		case MPG_ALTENTER:
		case MP_DETAIL:
			if (cur_friend)
				ShowFriendDetails(cur_friend);
			break;
		case MP_SHOWLIST:
		{
			if (cur_friend){
				if (cur_friend->m_LinkedClient)
					cur_friend->m_LinkedClient->RequestSharedFileList();
				else{
					CUpDownClient* newclient = new CUpDownClient(0,cur_friend->m_nLastUsedPort,cur_friend->m_dwLastUsedIP,0,0,true);
					newclient->SetUserName(cur_friend->m_strName.GetBuffer());
					theApp.clientlist->AddClient(newclient);
					newclient->RequestSharedFileList();
				}
			}
			break;
		}
		case MP_FRIENDSLOT:
		{
			if (cur_friend && cur_friend->m_LinkedClient){
				bool IsAlready;
				IsAlready = cur_friend->m_LinkedClient->GetFriendSlot();
				theApp.friendlist->RemoveAllFriendSlots();
				if( !IsAlready )
					cur_friend->m_LinkedClient->SetFriendSlot(true);
			}
			break;
		}
	}
	return true;
}

void CFriendListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) 
		ShowFriendDetails((CFriend*)GetItemData(iSel));
	*pResult = 0;
}

void CFriendListCtrl::ShowFriendDetails(const CFriend* pFriend)
{
	if (pFriend){
		if (pFriend->m_LinkedClient){
			CClientDetailDialog dialog(pFriend->m_LinkedClient);
			dialog.DoModal();
		}
		else{
			CAddFriend dlg;
			dlg.m_pShowFriend = const_cast<CFriend*>(pFriend);
			dlg.DoModal();
		}
	}
}

BOOL CFriendListCtrl::PreTranslateMessage(MSG* pMsg) 
{
   	if ( pMsg->message == 260 && pMsg->wParam == 13 && GetAsyncKeyState(VK_MENU)<0 ) {
		PostMessage(WM_COMMAND, MPG_ALTENTER, 0);
		return TRUE;
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE)
		PostMessage(WM_COMMAND, MP_REMOVEFRIEND, 0);
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_INSERT)
		PostMessage(WM_COMMAND, MP_ADDFRIEND, 0);

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}

void CFriendListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CFriendListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CFriend* item1 = (CFriend*)lParam1;
	CFriend* item2 = (CFriend*)lParam2; 
	if (item1 == NULL || item2 == NULL)
		return 0;

	int iResult;
	switch (LOWORD(lParamSort))
	{
		case 0:
			iResult = _tcsicmp(item1->m_strName, item2->m_strName);
			break;
		default:
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}

void CFriendListCtrl::UpdateList()
{
	theApp.emuledlg->chatwnd->UpdateFriendlistCount(theApp.friendlist->GetCount());
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));
}