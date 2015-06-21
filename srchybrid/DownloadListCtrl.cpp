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
#include "DownloadListCtrl.h"
#include "otherfunctions.h" 
#include "updownclient.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FileDetailDialog.h"
#include "commentdialoglst.h"
#include "MetaDataDlg.h"
#include "InputBox.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MemDC.h"
#include "ChatWnd.h"
#include "TransferWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "WebServices.h"
#include "Preview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CDownloadListCtrl

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define DLC_BARUPDATE 512


IMPLEMENT_DYNAMIC(CDownloadListCtrl, CListBox)
CDownloadListCtrl::CDownloadListCtrl() {
}

CDownloadListCtrl::~CDownloadListCtrl(){
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_A4AFMenu) VERIFY( m_A4AFMenu.DestroyMenu() );
	if (m_FileMenu) VERIFY( m_FileMenu.DestroyMenu() );
	while(m_ListItems.empty() == false){
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
}

void CDownloadListCtrl::Init()
{
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy, theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetStyle();
	ModifyStyle(LVS_SINGLESEL,0);
	
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT, 60);
	InsertColumn(2,GetResString(IDS_DL_TRANSF),LVCFMT_LEFT, 65);
	InsertColumn(3,GetResString(IDS_DL_TRANSFCOMPL),LVCFMT_LEFT, 65);
	InsertColumn(4,GetResString(IDS_DL_SPEED),LVCFMT_LEFT, 65);
	InsertColumn(5,GetResString(IDS_DL_PROGRESS),LVCFMT_LEFT, 170);
	InsertColumn(6,GetResString(IDS_DL_SOURCES),LVCFMT_LEFT, 50);
	InsertColumn(7,GetResString(IDS_PRIORITY),LVCFMT_LEFT, 55);
	InsertColumn(8,GetResString(IDS_STATUS),LVCFMT_LEFT, 70);
	InsertColumn(9,GetResString(IDS_DL_REMAINS),LVCFMT_LEFT, 110);

	CString lsctitle=GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(':');
	InsertColumn(10, lsctitle,LVCFMT_LEFT, 220);
	lsctitle=GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(':');
	InsertColumn(11, lsctitle,LVCFMT_LEFT, 220);
	InsertColumn(12, GetResString(IDS_CAT) ,LVCFMT_LEFT, 100);

	SetAllIcons();
	Localize();
	LoadSettings(CPreferences::tableDownload);
	curTab=0;

	// Barry - Use preferred sort order from preferences
	m_bRemainSort=thePrefs.TransferlistRemainSortStyle();

	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownload);
	bool sortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownload);

	uint8 adder=0;
	if (sortItem!=9 || !m_bRemainSort)
		SetSortArrow(sortItem, sortAscending);
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder);

}

void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadListCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader("SrcDownloading"));
	m_ImageList.Add(CTempIconLoader("SrcOnQueue"));
	m_ImageList.Add(CTempIconLoader("SrcConnecting"));
	m_ImageList.Add(CTempIconLoader("SrcNNPQF"));
	m_ImageList.Add(CTempIconLoader("SrcUnknown"));
	m_ImageList.Add(CTempIconLoader("ClientCompatible"));
	m_ImageList.Add(CTempIconLoader("Friend"));
	m_ImageList.Add(CTempIconLoader("ClientEDonkey"));
	m_ImageList.Add(CTempIconLoader("ClientMLDonkey"));
	m_ImageList.Add(CTempIconLoader("RatingReceived"));
	m_ImageList.Add(CTempIconLoader("BadRatingReceived"));
	m_ImageList.Add(CTempIconLoader("ClientEDonkeyHybrid"));
	m_ImageList.Add(CTempIconLoader("ClientShareaza"));
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader("ClientSecureOvl")), 1);
}

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(0, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(1, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(2, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_TRANSFCOMPL);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(3, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(4, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_PROGRESS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(5, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_SOURCES);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(6, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(7, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(8, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_DL_REMAINS);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(9, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_LASTSEENCOMPL);
	strRes.Remove(':');
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(10, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_FD_LASTCHANGE);
	strRes.Remove(':');
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(11, &hdi);
	strRes.ReleaseBuffer();

	strRes = GetResString(IDS_CAT);
	hdi.pszText = strRes.GetBuffer();
	pHeaderCtrl->SetItem(12, &hdi);
	strRes.ReleaseBuffer();

	CreateMenues();

	ShowFilesCount();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    uint16 itemnr = GetItemCount();
    newitem->owner = NULL;
    newitem->type = FILE_TYPE;
    newitem->value = toadd;
    newitem->parent = NULL;
	newitem->dwUpdated = 0; 

	// The same file shall be added only once
	ASSERT(m_ListItems.find(toadd) == m_ListItems.end());
	m_ListItems.insert(ListItemsPair(toadd, newitem));

	if (toadd->CheckShowItemInGivenCat(curTab))
		InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = owner;
    newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
    newitem->value = source;
	newitem->dwUpdated = 0; 

	// Update cross link to the owner
	ListItems::const_iterator ownerIt = m_ListItems.find(owner);
	ASSERT(ownerIt != m_ListItems.end());
	CtrlItem_Struct* ownerItem = ownerIt->second;
	ASSERT(ownerItem->value == owner);
	newitem->parent = ownerItem;

	// The same source could be added a few time but only one time per file 
	{
		// Update the other instances of this source
		bool bFound = false;
		std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(source);
		for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
			CtrlItem_Struct* cur_item = it->second;

			// Check if this source has been already added to this file => to be sure
			if(cur_item->owner == owner){
				// Update this instance with its new setting
				cur_item->type = newitem->type;
				cur_item->dwUpdated = 0;
				bFound = true;
			}
			else if(notavailable == false){
				// The state 'Available' is exclusive
				cur_item->type = UNAVAILABLE_SOURCE;
				cur_item->dwUpdated = 0;
			}
		}

		if(bFound == true){
			delete newitem; 
			return;
		}
	}
	m_ListItems.insert(ListItemsPair(source, newitem));

	if (owner->srcarevisible) {
		// find parent from the CListCtrl to add source
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)ownerItem;
		sint16 result = FindItem(&find);
		if(result != (-1))
			InsertItem(LVIF_PARAM|LVIF_TEXT,result+1,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);
	}
}

void CDownloadListCtrl::RemoveSource(CUpDownClient* source, CPartFile* owner)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::iterator, ListItems::iterator> rangeIt = m_ListItems.equal_range(source);
	for(ListItems::iterator it = rangeIt.first; it != rangeIt.second; ){
		CtrlItem_Struct* delItem  = it->second;
		if(owner == NULL || owner == delItem->owner){
			// Remove it from the m_ListItems			
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
 			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			sint16 result = FindItem(&find);
			if(result != (-1)){
				DeleteItem(result);
			}

			// finally it could be delete
			delete delItem;
		}
		else{
			it++;
		}
	}
}

bool CDownloadListCtrl::RemoveFile(CPartFile* toremove)
{
	bool bResult = false;
	if (!theApp.emuledlg->IsRunning())
		return bResult;
	// Retrieve all entries matching the File or linked to the file
	// Remark: The 'asked another files' clients must be removed from here
	ASSERT(toremove != NULL);
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* delItem = it->second;
		if(delItem->owner == toremove || delItem->value == toremove){
			// Remove it from the m_ListItems
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			sint16 result = FindItem(&find);
			if(result != (-1)){
				DeleteItem(result);
			}

			// finally it could be delete
			delete delItem;
			bResult = true;
		}
		else {
			it++;
		}
	}
	ShowFilesCount();
	return bResult;
}

void CDownloadListCtrl::UpdateItem(void* toupdate)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if (result != -1){
			updateItem->dwUpdated = 0;
			Update(result);
		}
	}
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPRECT lpRect, CtrlItem_Struct *lpCtrlItem)
{
	if(lpRect->left < lpRect->right)
	{
		CString buffer;
		CPartFile *lpPartFile = (CPartFile*)lpCtrlItem->value;
		switch(nColumn)
		{
		case 0:		// file name
			if (thePrefs.GetCatColor(lpPartFile->GetCategory()) > 0)
				dc->SetTextColor(thePrefs.GetCatColor(lpPartFile->GetCategory()));

			if ( thePrefs.ShowRatingIndicator() && ( lpPartFile->HasComment() || lpPartFile->HasRating() )){
				POINT point= {lpRect->left-4,lpRect->top+2};
				m_ImageList.Draw(dc, (lpPartFile->HasRating() && lpPartFile->HasBadRating()) ? 10 : 9, point, ILD_NORMAL);

				lpRect->left+=9;
				dc->DrawText(lpPartFile->GetFileName(), lpPartFile->GetFileName().GetLength(), lpRect, DLC_DT_TEXT);
				lpRect->left-=9;
			}
			else
				dc->DrawText(lpPartFile->GetFileName(), lpPartFile->GetFileName().GetLength(), lpRect, DLC_DT_TEXT);
			break;

		case 1:		// size
			buffer = CastItoXBytes(lpPartFile->GetFileSize());
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			break;

		case 2:		// transfered
			buffer = CastItoXBytes(lpPartFile->GetTransfered());
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			break;

		case 3:		// transfered complete
			buffer = CastItoXBytes(lpPartFile->GetCompletedSize());
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			break;
		case 4:		// speed
			if (lpPartFile->GetTransferingSrcCount())
				buffer.Format("%.1f %s", lpPartFile->GetDatarate() / 1024.0f,GetResString(IDS_KBYTESEC));
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			break;

		case 5:		// progress
			{
				lpRect->bottom --;
				lpRect->top ++;
				// added
				int  iWidth = lpRect->right - lpRect->left; 
				int iHeight = lpRect->bottom - lpRect->top;
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx; 
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !lpCtrlItem->dwUpdated) {
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(lpRect->left, lpRect->top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
				//added end

				if (thePrefs.GetUseDwlPercentage()) {
					// HoaX_69: BEGIN Display percent in progress bar
					COLORREF oldclr = dc->SetTextColor(RGB(255,255,255));
					int iOMode = dc->SetBkMode(TRANSPARENT);
					buffer.Format("%.1f%%", lpPartFile->GetPercentCompleted());
					dc->DrawText(buffer, buffer.GetLength(), lpRect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					dc->SetBkMode(iOMode);
					dc->SetTextColor(oldclr);
					// HoaX_69: END
				}

				lpRect->bottom ++;
				lpRect->top --;
			}
			break;

		case 6:		// sources
			{
				uint16 sc = lpPartFile->GetSourceCount();
				uint16 ncsc = lpPartFile->GetNotCurrentSourcesCount();				
				if(ncsc>0){
					if (!thePrefs.IsExtControlsEnabled() || lpPartFile->GetSrcA4AFCount()==0)
						buffer.Format("%i/%i (%i)", sc - ncsc, sc, lpPartFile->GetTransferingSrcCount());
					else
						buffer.Format("%i/%i+%i (%i)", sc - ncsc, sc, lpPartFile->GetSrcA4AFCount(), lpPartFile->GetTransferingSrcCount());
				}
				else {
					if (!thePrefs.IsExtControlsEnabled() || lpPartFile->GetSrcA4AFCount()==0)
						buffer.Format("%i (%i)", sc, lpPartFile->GetTransferingSrcCount());
					else
						buffer.Format("%i+%i (%i)", sc, lpPartFile->GetSrcA4AFCount(), lpPartFile->GetTransferingSrcCount());
				}
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 7:		// prio
			switch(lpPartFile->GetDownPriority()) {
			case PR_LOW:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOLOW),GetResString(IDS_PRIOAUTOLOW).GetLength(),lpRect, DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOLOW),GetResString(IDS_PRIOLOW).GetLength(),lpRect, DLC_DT_TEXT);
				break;
			case PR_NORMAL:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTONORMAL),GetResString(IDS_PRIOAUTONORMAL).GetLength(),lpRect, DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIONORMAL),GetResString(IDS_PRIONORMAL).GetLength(),lpRect, DLC_DT_TEXT);
				break;
			case PR_HIGH:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOHIGH),GetResString(IDS_PRIOAUTOHIGH).GetLength(),lpRect, DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOHIGH),GetResString(IDS_PRIOHIGH).GetLength(),lpRect, DLC_DT_TEXT);
				break;
			}
			break;

		case 8:		// <<--9/21/02
			buffer = lpPartFile->getPartfileStatus();
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			break;

		case 9:		// remaining time & size
			{
				if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE ){
					//size 
					uint32 remains;
					remains=lpPartFile->GetFileSize()-lpPartFile->GetCompletedSize();		//<<-- 09/27/2002, CML

					// time 
					sint32 restTime=lpPartFile->getTimeRemaining();
					buffer.Format("%s (%s)", CastSecondsToHM(restTime),CastItoXBytes(remains));

				}
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			}
			break;
		case 10: // last seen complete
			{
				CString tempbuffer;
				if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				{
					tempbuffer.Format("< %u", lpPartFile->m_nCompleteSourcesCountHi);
				}
				else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				{
					tempbuffer.Format("%u", lpPartFile->m_nCompleteSourcesCountLo);
				}
				else
				{
					tempbuffer.Format("%u - %u", lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);
				}
				if (lpPartFile->lastseencomplete==NULL)
					buffer.Format("%s(%s)",GetResString(IDS_UNKNOWN),tempbuffer);
				else
					buffer.Format("%s(%s)",lpPartFile->lastseencomplete.Format( thePrefs.GetDateTimeFormat()),tempbuffer);
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			}
			break;
		case 11: // last receive
			if (!IsColumnHidden(11)) {
				if(lpPartFile->GetFileDate()!=NULL)
					buffer=lpPartFile->GetCFileDate().Format( thePrefs.GetDateTimeFormat());
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			}
			break;
		case 12: // cat
			if (!IsColumnHidden(12)) {
				buffer=(lpPartFile->GetCategory()!=0)?
					thePrefs.GetCategory(lpPartFile->GetCategory())->title:GetResString(IDS_ALL);
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			}
			break;
		}
	}
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPRECT lpRect, CtrlItem_Struct *lpCtrlItem) {
	if(lpRect->left < lpRect->right) { 

		CString buffer;
		CUpDownClient *lpUpDownClient = (CUpDownClient*)lpCtrlItem->value;
		switch(nColumn) {

		case 0:		// icon, name, status
			{
				RECT cur_rec = *lpRect;
				POINT point = {cur_rec.left, cur_rec.top+1};
				if (lpCtrlItem->type == AVAILABLE_SOURCE){
					switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTING:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_CONNECTED:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_WAITCALLBACK:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_ONQUEUE:
						if(lpUpDownClient->IsRemoteQueueFull())
							m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						else
							m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
						break;
					case DS_DOWNLOADING:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_REQHASHSET:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_NONEEDEDPARTS:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_LOWTOLOWIP:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_TOOMANYCONNS:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					default:
						m_ImageList.Draw(dc, 4, point, ILD_NORMAL);
					}
				}
				else {

					m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				}
				cur_rec.left += 20;
				UINT uOvlImg = ((lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0);
				POINT point2= {cur_rec.left,cur_rec.top+1};
				if (lpUpDownClient->IsFriend())
					m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_EDONKEYHYBRID)
					m_ImageList.Draw(dc, 11, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_MLDONKEY)
					m_ImageList.Draw(dc, 8, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->GetClientSoft() == SO_SHAREAZA)
					m_ImageList.Draw(dc, 12, point2, ILD_NORMAL | uOvlImg);
				else if (lpUpDownClient->ExtProtocolAvailable())
					m_ImageList.Draw(dc, 5, point2, ILD_NORMAL | uOvlImg);
				else
					m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | uOvlImg);
				cur_rec.left += 20;

				if (!lpUpDownClient->GetUserName())
					buffer = "?";
				else
					buffer = lpUpDownClient->GetUserName();
				dc->DrawText(buffer,buffer.GetLength(),&cur_rec, DLC_DT_TEXT);
			}
			break;

		case 1:		// size
			switch(lpUpDownClient->GetSourceFrom()){
				case SF_SERVER:
					buffer = "eD2K Server";
					break;
				case SF_KADEMLIA:
					buffer = "Kad";
					break;
				case SF_SOURCE_EXCHANGE:
					buffer = GetResString(IDS_SE);
					break;
				case SF_PASSIVE:
					buffer = GetResString(IDS_PASSIVE);
					break;
				default:
					buffer = GetResString(IDS_ERROR);
			}
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			break;

		case 2:// transfered
			if ( !IsColumnHidden(3)) {
				dc->DrawText("",0,lpRect, DLC_DT_TEXT);
				break;
			}
		case 3:// completed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetTransferedDown()) {
				buffer = CastItoXBytes(lpUpDownClient->GetTransferedDown());
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 4:		// speed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetDownloadDatarate()){
				if (lpUpDownClient->GetDownloadDatarate())
					buffer.Format("%.1f %s", lpUpDownClient->GetDownloadDatarate()/1024.0f,GetResString(IDS_KBYTESEC));
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 5:		// file info
			{
				lpRect->bottom--; 
				lpRect->top++; 

				int iWidth = lpRect->right - lpRect->left; 
				int iHeight = lpRect->bottom - lpRect->top; 
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx;
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !lpCtrlItem->dwUpdated) { 
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status,(lpCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(lpRect->left, lpRect->top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);

				lpRect->bottom++; 
				lpRect->top--; 
			}
			break;

		case 6:{		// sources
			buffer = lpUpDownClient->GetClientSoftVer();
			if (buffer.IsEmpty())
				buffer = GetResString(IDS_UNKNOWN);
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			break;
		}

		case 7:		// prio
			if (lpUpDownClient->GetDownloadState()==DS_ONQUEUE){
				if( lpUpDownClient->IsRemoteQueueFull() ){
					buffer = GetResString(IDS_QUEUEFULL);
					dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
				}
				else{
					if ( lpUpDownClient->GetRemoteQueueRank()){
						buffer.Format("QR: %u",lpUpDownClient->GetRemoteQueueRank());
						dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
					}
					else{
						dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
					}
				}
			} else {
				dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			}
			break;

		case 8:	{	// status
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTING:
						buffer = GetResString(IDS_CONNECTING);
						break;
					case DS_CONNECTED:
						buffer = GetResString(IDS_ASKING);
						break;
					case DS_WAITCALLBACK:
						buffer = GetResString(IDS_CONNVIASERVER);
						break;
					case DS_ONQUEUE:
						if( lpUpDownClient->IsRemoteQueueFull() )
							buffer = GetResString(IDS_QUEUEFULL);
						else
							buffer = GetResString(IDS_ONQUEUE);
						break;
					case DS_DOWNLOADING:
						buffer = GetResString(IDS_TRANSFERRING);
						break;
					case DS_REQHASHSET:
						buffer = GetResString(IDS_RECHASHSET);
						break;
					case DS_NONEEDEDPARTS:
						buffer = GetResString(IDS_NONEEDEDPARTS);
						break;
					case DS_LOWTOLOWIP:
						buffer = GetResString(IDS_NOCONNECTLOW2LOW);
						break;
					case DS_TOOMANYCONNS:
						buffer = GetResString(IDS_TOOMANYCONNS);
						break;
					default:
						buffer = GetResString(IDS_UNKNOWN);
				}
			}
			else {
				buffer = GetResString(IDS_ASKED4ANOTHERFILE);
			}
			dc->DrawText(buffer,buffer.GetLength(),lpRect, DLC_DT_TEXT);
			break;
				}
		case 9:		// remaining time & size
			break;
		}
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct){
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	CtrlItem_Struct* content = (CtrlItem_Struct*)lpDrawItemStruct->itemData;
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

	if ((content->type == FILE_TYPE) && (lpDrawItemStruct->itemAction | ODA_SELECT) &&
	    (lpDrawItemStruct->itemState & ODS_SELECTED)) {

		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont *pOldFont = dc->SelectObject(GetFont());
	COLORREF crOldTextColor = dc->SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	BOOL notLast = lpDrawItemStruct->itemID + 1 != GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	RECT cur_rec = lpDrawItemStruct->rcItem;

	//offset was 4, now it's the standard 2 spaces
	int iOffset = dc->GetTextExtent(_T(" "), 1 ).cx*2;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left;
	cur_rec.right -= iOffset;
	cur_rec.left += iOffset;

	if (content->type == FILE_TYPE){
		for(int iCurrent = 0; iCurrent < iCount; iCurrent++) {

			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			if(iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iOffset;
				DrawFileItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawFileItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}

		}

	}
	else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE){

		for(int iCurrent = 0; iCurrent < iCount; iCurrent++) {

			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);

			if(iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iOffset;
				DrawSourceItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawSourceItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}

	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(content->type == FILE_TYPE))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(notFirst && (GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))) {
			CtrlItem_Struct* prev = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID - 1);
			if(prev->type == FILE_TYPE)
				outline_rec.top--;
		} 

		if(notLast && (GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))) {
			CtrlItem_Struct* next = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1);
			if(next->type == FILE_TYPE)
				outline_rec.bottom++;
		} 

		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	//draw focus rectangle around non-highlightable items when they have the focus
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc->FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}

	//draw tree last so it draws over selected and focus (looks better)
	if(tree_start < tree_end) {
		//set new bounds
		RECT tree_rect;
		tree_rect.top    = lpDrawItemStruct->rcItem.top;
		tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
		tree_rect.left   = tree_start;
		tree_rect.right  = tree_end;
		dc->SetBoundsRect(&tree_rect, DCB_DISABLE);

		//gather some information
		BOOL hasNext = notLast &&
			((CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1))->type != FILE_TYPE;
		BOOL isOpenRoot = hasNext && content->type == FILE_TYPE;
		BOOL isChild = content->type != FILE_TYPE;
		//BOOL isExpandable = !isChild && ((CPartFile*)content->value)->GetSourceCount() > 0;
		//might as well calculate these now
		int treeCenter = tree_start + 3;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

		//set up a new pen for drawing the tree
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, m_crWindowText);
		oldpn = dc->SelectObject(&pn);

		if(isChild) {
			//draw the line to the status bar
			dc->MoveTo(tree_end, middle);
			dc->LineTo(tree_start + 3, middle);

			//draw the line to the child node
			if(hasNext) {
				dc->MoveTo(treeCenter, middle);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} else if(isOpenRoot) {
			//draw circle
			RECT circle_rec;
			COLORREF crBk = dc->GetBkColor();
			circle_rec.top    = middle - 2;
			circle_rec.bottom = middle + 3;
			circle_rec.left   = treeCenter - 2;
			circle_rec.right  = treeCenter + 3;
			dc->FrameRect(&circle_rec, &CBrush(m_crWindowText));
			dc->SetPixelV(circle_rec.left,      circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.top,    crBk);
			dc->SetPixelV(circle_rec.left,      circle_rec.bottom - 1, crBk);
			dc->SetPixelV(circle_rec.right - 1, circle_rec.bottom - 1, crBk);
			//draw the line to the child node
			if(hasNext) {
				dc->MoveTo(treeCenter, middle + 3);
				dc->LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} /*else if(isExpandable) {
			//draw a + sign
			dc->MoveTo(treeCenter, middle - 2);
			dc->LineTo(treeCenter, middle + 3);
			dc->MoveTo(treeCenter - 2, middle);
			dc->LineTo(treeCenter + 3, middle);
		}*/

		//draw the line back up to parent node
		if(notFirst && isChild) {
			dc->MoveTo(treeCenter, middle);
			dc->LineTo(treeCenter, cur_rec.top - 1);
		}

		//put the old pen back
		dc->SelectObject(oldpn);
		pn.DeleteObject();
	}

	//put the original objects back
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc->SelectObject(pOldFont);
	dc->SetTextColor(crOldTextColor);
}

// modifier-keys -view filtering [Ese Juani+xrmb]
void CDownloadListCtrl::HideSources(CPartFile* toCollapse, bool isShift, bool isCtrl, bool isAlt)
{
	SetRedraw(false);
	int pre,post;
	pre = post = 0;
	for(int i = 0; i < GetItemCount(); i++) {
		CtrlItem_Struct* item = (CtrlItem_Struct*)this->GetItemData(i);
		if(item->owner == toCollapse) {
			pre++;
			if(isShift || isCtrl || isAlt){
				EDownloadState ds=((CUpDownClient*)item->value)->GetDownloadState();
				if((isShift && ds==DS_DOWNLOADING) ||
					(isCtrl && ((CUpDownClient*)item->value)->GetRemoteQueueRank()> 0) ||
					(isAlt && ds!=DS_NONEEDEDPARTS)) continue;
			}
			item->dwUpdated = 0;
			item->status.DeleteObject();
			DeleteItem(i--);
			post++;
		}
	}
	if (pre-post==0) toCollapse->srcarevisible = false;
	SetRedraw(true);
}

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
END_MESSAGE_MAP()

void CDownloadListCtrl::ExpandCollapseItem(int item,uint8 expand,bool collapsesource){
	if (item==-1) return;

	CtrlItem_Struct* content = (CtrlItem_Struct*)this->GetItemData(item);

	// modifier-keys -view filtering [Ese Juani+xrmb]
	bool isShift=GetAsyncKeyState(VK_SHIFT) < 0;
	bool isCtrl=GetAsyncKeyState(VK_CONTROL) < 0;
	bool isAlt=GetAsyncKeyState(VK_MENU) < 0;

	if (collapsesource && content->parent!=NULL) {// to collapse/expand files when one of its source is selected
		content=content->parent;
		
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)content;
		item = FindItem(&find);
		if (item==-1) return;
	}

	if (!content || content->type != FILE_TYPE) return;
	
	CPartFile* partfile = reinterpret_cast<CPartFile*>(content->value);
	if (!partfile) return;

	if (partfile->GetStatus()==PS_COMPLETE) {
		char* buffer = new char[MAX_PATH];
		sprintf(buffer,"%s",partfile->GetFullName());
		ShellOpenFile(buffer, NULL);
		delete[] buffer;
		return;
	}

	// Check if the source branch is disable
	if(partfile->srcarevisible == false ) {
		if (expand>COLLAPSE_ONLY){
			SetRedraw(false);
			
			// Go throught the whole list to find out the sources for this file
			// Remark: don't use GetSourceCount() => UNAVAILABLE_SOURCE
			for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
				const CtrlItem_Struct* cur_item = it->second;
				if(cur_item->owner == partfile){
					if(isShift || isCtrl || isAlt) {
						ASSERT(cur_item->type != FILE_TYPE);
						EDownloadState ds=((CUpDownClient*)cur_item->value)->GetDownloadState();
						if(!(isShift && ds==DS_DOWNLOADING ||
							isCtrl && ((CUpDownClient*)cur_item->value)->GetRemoteQueueRank()>0 ||
							isAlt && ds!=DS_NONEEDEDPARTS))
							continue; // skip this source
					}
					partfile->srcarevisible = true;
					InsertItem(LVIF_PARAM|LVIF_TEXT,item+1,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)cur_item);
				}
			}

			SetRedraw(true);
		}
	}
	else {
		if (expand==EXPAND_COLLAPSE || expand==COLLAPSE_ONLY) HideSources(partfile,isShift,isCtrl,isAlt);
	}
}

// CDownloadListCtrl message handlers
void CDownloadListCtrl::OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult){
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (thePrefs.IsDoubleClickEnabled() || pNMIA->iSubItem > 0)
		ExpandCollapseItem(pNMIA->iItem,2);
	*pResult = 0;
}

void CDownloadListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			// get merged settings
			bool bFirstItem = true;
			int iSelectedItems = 0;
			int iFilesNotDone = 0;
			int iFilesToPause = 0;
			int iFilesToStop = 0;
			int iFilesToResume = 0;
			int iFilesToOpen = 0;
			int iFilesToPreview = 0;
			int iFilesA4AFAuto = 0;
			UINT uPrioMenuItem = 0;
			const CPartFile* file1 = NULL;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
				if (bFirstItem)
					file1 = pFile;
				iSelectedItems++;

				bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
				iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
				iFilesToPreview += pFile->CanPreviewFile() ? 1 : 0;
				iFilesA4AFAuto += (!bFileDone && pFile->IsA4AFAuto()) ? 1 : 0;

				UINT uCurPrioMenuItem = 0;
				if (pFile->IsAutoDownPriority())
					uCurPrioMenuItem = MP_PRIOAUTO;
				else if (pFile->GetDownPriority() == PR_HIGH)
					uCurPrioMenuItem = MP_PRIOHIGH;
				else if (pFile->GetDownPriority() == PR_NORMAL)
					uCurPrioMenuItem = MP_PRIONORMAL;
				else if (pFile->GetDownPriority() == PR_LOW)
					uCurPrioMenuItem = MP_PRIOLOW;
				else
					ASSERT(0);

				if (bFirstItem)
					uPrioMenuItem = uCurPrioMenuItem;
				else if (uPrioMenuItem != uCurPrioMenuItem)
					uPrioMenuItem = 0;

				bFirstItem = false;
			}

			m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

			// enable commands if there is at least one item which can be used for the action
			m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);

			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
			CMenu PreviewMenu;
			PreviewMenu.CreateMenu();
			int iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewMenu, (iSelectedItems == 1) ? file1 : NULL);
			if (iPreviewMenuEntries)
				m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewMenu.m_hMenu, GetResString(IDS_DL_PREVIEW));

			bool bDetailsEnabled = (iSelectedItems > 0);
			m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
			if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
				m_FileMenu.SetDefaultItem(MP_OPEN);
			else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
				m_FileMenu.SetDefaultItem(MP_METINFO);
			else
				m_FileMenu.SetDefaultItem((UINT)-1);
			m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);

			m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem((UINT_PTR)m_A4AFMenu.m_hMenu, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
			m_A4AFMenu.CheckMenuItem(MP_ALL_A4AF_AUTO, (iSelectedItems == 1 && iFilesNotDone == 1 && iFilesA4AFAuto == 1) ? MF_CHECKED : MF_UNCHECKED);

			m_FileMenu.EnableMenuItem(MP_GETED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_GETHTMLED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);

			CMenu WebMenu;
			WebMenu.CreateMenu();
			int iWebMenuEntries = theWebServices.GetFileMenuEntries(WebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES));

			// create cat-submenue
			CMenu CatsMenu;
			CatsMenu.CreateMenu();
			flag = (thePrefs.GetCatCount() == 1) ? MF_GRAYED : MF_ENABLED;
			if (thePrefs.GetCatCount()>1) {
				for (int i = 0; i < thePrefs.GetCatCount(); i++)
					CatsMenu.AppendMenu(MF_STRING,MP_ASSIGNCAT+i, (i==0)?GetResString(IDS_CAT_UNASSIGN):thePrefs.GetCategory(i)->title);
			}
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT));

			GetPopupMenuPos(*this, point);
			m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			if (iPreviewMenuEntries)
				VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewMenu.m_hMenu, MF_BYCOMMAND) );
			VERIFY( WebMenu.DestroyMenu() );
			VERIFY( CatsMenu.DestroyMenu() );
			VERIFY( PreviewMenu.DestroyMenu() );
		}
		else{
			const CUpDownClient* client = (CUpDownClient*)content->value;
			CTitleMenu ClientMenu;
			ClientMenu.CreatePopupMenu();
			ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS));
			ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
			ClientMenu.SetDefaultItem(MP_DETAIL);
			ClientMenu.AppendMenu(MF_STRING | ((client && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND));
			ClientMenu.AppendMenu(MF_STRING, MP_MESSAGE, GetResString(IDS_SEND_MSG));
			ClientMenu.AppendMenu(MF_STRING | ((!client || !client->GetViewSharedFilesSupport()) ? MF_GRAYED : MF_ENABLED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
			if (Kademlia::CKademlia::isRunning() && !Kademlia::CKademlia::getPrefs()->getLastContact())
				ClientMenu.AppendMenu(MF_STRING | ((!client || client->GetKadPort()==0) ? MF_GRAYED : MF_ENABLED), MP_BOOT, GetResString(IDS_BOOTSTRAP));

			CMenu A4AFMenu;
			A4AFMenu.CreateMenu();
			if (thePrefs.IsExtControlsEnabled()) {
				if (content->type == UNAVAILABLE_SOURCE)
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_THIS,GetResString(IDS_SWAP_A4AF_TO_THIS)); // Added by sivka [Ambdribant]
				if (content->type == AVAILABLE_SOURCE)
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_OTHER,GetResString(IDS_SWAP_A4AF_TO_OTHER)); // Added by sivka
				if (A4AFMenu.GetMenuItemCount()>0)
					ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
			}

			GetPopupMenuPos(*this, point);
			ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			
			VERIFY( A4AFMenu.DestroyMenu() );
			VERIFY( ClientMenu.DestroyMenu() );
		}
	}
	else{
		m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CANCEL,MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE,MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP,MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME,MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PREVIEW,MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem((UINT_PTR)m_A4AFMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_GETED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_GETHTMLED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		CMenu WebMenu;
		WebMenu.CreateMenu();
		int iWebMenuEntries = theWebServices.GetFileMenuEntries(WebMenu);
		m_FileMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES));

		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
		VERIFY( WebMenu.DestroyMenu() );
	}
}

BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case MP_PASTE:
			theApp.PasteClipboard();
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			//for multiple selections 
			UINT selectedCount = 0;
			CTypedPtrList<CPtrList, CPartFile*> selectedList; 
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = GetNextSelectedItem(pos);
				if(index > -1) 
				{
					if (((const CtrlItem_Struct*)GetItemData(index))->type == FILE_TYPE)
					{
						selectedCount++;
						selectedList.AddTail((CPartFile*)((const CtrlItem_Struct*)GetItemData(index))->value);
					}
				} 
			} 

			CPartFile* file = (CPartFile*)content->value;
			switch (wParam)
			{
				case MPG_DELETE:
				case MP_CANCEL:
				{
					//for multiple selections
					if(selectedCount > 0)
					{
						SetRedraw(false);
						CString fileList;
						bool validdelete = false;
						for (pos = selectedList.GetHeadPosition(); pos != 0; )
						{
							CPartFile* cur_file = selectedList.GetNext(pos);
							if (cur_file->GetStatus() != PS_COMPLETING && cur_file->GetStatus() != PS_COMPLETE){
								validdelete = true;
								if (selectedCount<50)
									fileList.Append(_T("\n") + CString(cur_file->GetFileName()));
							}
						}
						CString quest;
						if (selectedCount==1)
							quest=GetResString(IDS_Q_CANCELDL2);
						else
							quest=GetResString(IDS_Q_CANCELDL);
						if (validdelete && AfxMessageBox(quest + fileList,MB_ICONQUESTION|MB_YESNO) == IDYES)
						{
							while(!selectedList.IsEmpty())
							{
								HideSources(selectedList.GetHead());
								switch(selectedList.GetHead()->GetStatus()) {
									case PS_WAITINGFORHASH:
									case PS_HASHING:
									case PS_COMPLETING:
									case PS_COMPLETE:
										selectedList.RemoveHead();
										break;
									case PS_PAUSED:
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										break;
									default:
										if (thePrefs.StartNextFile())
											theApp.downloadqueue->StartNextFile();
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
								}
							}
						}
						SetRedraw(true);
					}
					break;
				}
				case MP_PRIOHIGH:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOLOW:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_LOW);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIONORMAL:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_NORMAL);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOAUTO:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(true);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PAUSE:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanPauseFile())
							partfile->PauseFile();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_RESUME:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanResumeFile()){
							if (partfile->GetStatus() == PS_INSUFFICIENT)
								partfile->ResumeFileInsufficient();
							else
								partfile->ResumeFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_STOP:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.GetHead();
						if (partfile->CanStopFile()){
							HideSources(partfile);
							partfile->StopFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					ClearCompleted();
					SetRedraw(true);
					break;
				case MP_ALL_A4AF_TO_THIS:
					SetRedraw(false);
					if (selectedCount == 1 && (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
					{
						theApp.downloadqueue->DisableAllA4AFAuto();

						POSITION pos1, pos2;
						for (pos1 = file->A4AFsrclist.GetHeadPosition();(pos2=pos1)!=NULL;)
						{
							file->A4AFsrclist.GetNext(pos1);
							CUpDownClient *cur_source = file->A4AFsrclist.GetAt(pos2);
							if( cur_source->GetDownloadState() != DS_DOWNLOADING
								&& cur_source->reqfile
								&& ( (!cur_source->reqfile->IsA4AFAuto()) || cur_source->GetDownloadState() == DS_NONEEDEDPARTS)
								&& !cur_source->IsSwapSuspended(file) )
							{
								CPartFile* oldfile = cur_source->reqfile;
								if (cur_source->SwapToAnotherFile(true, false, false, file)){
									cur_source->DontSwapTo(oldfile);
								}
							}
						}
					}
					SetRedraw(true);
					UpdateItem(file);
					break;
				case MP_ALL_A4AF_AUTO:
					file->SetA4AFAuto(!file->IsA4AFAuto());
					break;
				case MP_ALL_A4AF_TO_OTHER:
					SetRedraw(false);
					if (selectedCount == 1 && (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
					{
						theApp.downloadqueue->DisableAllA4AFAuto();

						POSITION pos1, pos2;
						for (pos1 = file->srclist.GetHeadPosition(); (pos2 = pos1) != NULL;)
						{
							file->srclist.GetNext(pos1);
							file->srclist.GetAt(pos2)->SwapToAnotherFile(false, false, false, NULL);
						}
					}
					SetRedraw(true);
					break;
				case MPG_F2: {
					InputBox inputbox;
					CString title=GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title ,GetResString(IDS_DL_FILENAME),file->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString nn=inputbox.GetInput();
					if (!inputbox.WasCancelled() && nn.GetLength()>0){
						file->SetFileName(nn,true);
						file->UpdateDisplayedInfo();
						file->SavePartFile();
					}
					break;
				}
				case MPG_ALTENTER:
				case MP_METINFO:
					ShowFileDialog();
					break;
				case MP_GETED2KLINK:{
					CString str;
					while (!selectedList.IsEmpty()){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += CreateED2kLink(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_GETHTMLED2KLINK:{
					CString str;
					while (!selectedList.IsEmpty()){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += CreateHTMLED2kLink(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_OPEN:
					if(selectedCount > 1)
						break;
					file->OpenFile();
					break;
				case MP_PREVIEW:{
					if(selectedCount > 1)
						break;
					file->PreviewFile();
					break;
				}
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(NULL, true);
					break;

				default:
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99){
						SetRedraw(FALSE);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetCategory(wParam - MP_ASSIGNCAT);
							partfile->UpdateDisplayedInfo(true);
							selectedList.RemoveHead();
						}
						SetRedraw(TRUE);
						ChangeCategory(curTab);
					}
					else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
						thePreviewApps.RunApp(file, wParam);
					}
					break;
			}
		}
		else{
			CUpDownClient* client = (CUpDownClient*)content->value;
			CPartFile* file = (CPartFile*)content->owner; // added by sivka

			switch (wParam){
				case MP_SHOWLIST:
					client->RequestSharedFileList();
					break;
				case MP_MESSAGE:
					theApp.emuledlg->chatwnd->StartSession(client);
					break;
				case MP_ADDFRIEND:
					if (theApp.friendlist->AddFriend(client))
						UpdateItem(client);
					break;
				case MPG_ALTENTER:
				case MP_DETAIL:{
					CClientDetailDialog dialog(client);
					dialog.DoModal();
					break;
				}
				case MP_BOOT:
					if (client->GetKadPort())
						Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					break;
				case MP_SWAP_A4AF_TO_THIS: // added by sivka [enkeyDEV(Ottavio84) -A4AF-]
					if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if (client->GetDownloadState() != DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(true, true, false, file);
							UpdateItem(file);
						}
					}
					break;
				case MP_SWAP_A4AF_TO_OTHER:
					if (client != NULL && client->GetDownloadState() != DS_DOWNLOADING)
						client->SwapToAnotherFile(true, true, false, NULL);
					break;
			}
		}
	}
	else /*nothing selected*/
	{
		switch (wParam){
			case MP_CLEARCOMPLETED:
				ClearCompleted();
				break;
		}
	}

	return TRUE;
}

void CDownloadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = thePrefs.GetColumnSortItem(CPreferences::tableDownload);
	bool m_oldSortAscending = thePrefs.GetColumnSortAscending(CPreferences::tableDownload);

	if (sortItem==9) {
		m_bRemainSort=(sortItem != pNMListView->iSubItem) ? false : (m_oldSortAscending?m_bRemainSort:!m_bRemainSort);
	}

	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;
	
	// Save new preferences
	thePrefs.SetColumnSortItem(CPreferences::tableDownload, sortItem);
	thePrefs.SetColumnSortAscending(CPreferences::tableDownload, sortAscending);
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);
	
	// Sort table
	uint8 adder=0;
	if (sortItem!=9 || !m_bRemainSort)
		SetSortArrow(sortItem, sortAscending);
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	

	SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder );

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CtrlItem_Struct* item1 = (CtrlItem_Struct*)lParam1;
	CtrlItem_Struct* item2 = (CtrlItem_Struct*)lParam2;

	int sortMod = 1;
	if(lParamSort >= 100) {
		sortMod = -1;
		lParamSort -= 100;
	}

	int comp;

	if(item1->type == FILE_TYPE && item2->type != FILE_TYPE) {
		if(item1->value == item2->parent->value)
			return -1;

		comp = Compare((CPartFile*)item1->value, (CPartFile*)(item2->parent->value), lParamSort);

	} else if(item2->type == FILE_TYPE && item1->type != FILE_TYPE) {
		if(item1->parent->value == item2->value)
			return 1;

		comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)item2->value, lParamSort);

	} else if (item1->type == FILE_TYPE) {
		CPartFile* file1 = (CPartFile*)item1->value;
		CPartFile* file2 = (CPartFile*)item2->value;

		comp = Compare(file1, file2, lParamSort);

	} else {
		comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)(item2->parent->value), lParamSort);
		if(comp != 0)
			return sortMod * comp;
		if (item1->type != item2->type)
			return item1->type - item2->type;

		CUpDownClient* client1 = (CUpDownClient*)item1->value;
		CUpDownClient* client2 = (CUpDownClient*)item2->value;
		comp = Compare(client1, client2, lParamSort,sortMod);
	}

	return sortMod * comp;
}


void CDownloadListCtrl::ClearCompleted(bool ignorecats){
	// Search for completed file(s)
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if(file->IsPartFile() == false && (file->CheckShowItemInGivenCat(curTab) || ignorecats) ){
				if (RemoveFile(file))
					it = m_ListItems.begin();
			}
		}
	}
}

void CDownloadListCtrl::SetStyle() {
	if (thePrefs.IsDoubleClickEnabled())
		SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	else
		SetExtendedStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT *pResult) {
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);
}

int CDownloadListCtrl::Compare(const CPartFile* file1, const CPartFile* file2, LPARAM lParamSort)
{
	switch(lParamSort){
	case 0: //filename asc
		return _tcsicmp(file1->GetFileName(),file2->GetFileName());
	case 1: //size asc
		return CompareUnsigned(file1->GetFileSize(), file2->GetFileSize());
	case 2: //transfered asc
		return CompareUnsigned(file1->GetTransfered(), file2->GetTransfered());
	case 3: //completed asc
		return CompareUnsigned(file1->GetCompletedSize(), file2->GetCompletedSize());
	case 4: //speed asc
		return CompareUnsigned(file1->GetDatarate(), file2->GetDatarate());
	case 5: //progress asc
		{
			float comp = file1->GetPercentCompleted() - file2->GetPercentCompleted();
			if(comp > 0)
				return 1;
			else if(comp < 0)
				return -1;
			else
				return 0;
		}
	case 6: //sources asc
		return file1->GetSourceCount() - file2->GetSourceCount();
	case 7: //priority asc
		return file1->GetDownPriority() - file2->GetDownPriority();
	case 8: //Status asc 
		return file1->getPartfileStatusRang()-file2->getPartfileStatusRang(); 
	case 9: //Remaining Time asc 
		if (file1->GetDatarate())
			if (file2->GetDatarate())
                return file1->getTimeRemaining()-file2->getTimeRemaining() ;
			else
				return -1;
		else
			if (file2->GetDatarate())
				return 1;
			else
                return 0;
	case 90: //Remaining SIZE asc 
		return ((file1->GetFileSize()-file1->GetCompletedSize()) -
			(file2->GetFileSize()-file2->GetCompletedSize()) );

	case 10: //last seen complete asc 
		if (file1->lastseencomplete > file2->lastseencomplete)
			return 1;
		else if(file1->lastseencomplete < file2->lastseencomplete)
			return -1;
		else
			return 0;
	case 11: //last received Time asc 
		if (file1->GetFileDate() > file2->GetFileDate())
			return 1;
		else if(file1->GetFileDate() < file2->GetFileDate())
			return -1;
		else
			return 0;

	case 12:
		//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
		return _tcsicmp(
			(const_cast<CPartFile*>(file1)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->title:GetResString(IDS_ALL),
			(const_cast<CPartFile*>(file2)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->title:GetResString(IDS_ALL)
		);
		return 0;

	default:
		return 0;
	}
}

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort, int sortMod)
{
	switch(lParamSort){
	case 0: //name asc
		if(client1->GetUserName() == client2->GetUserName())
			return 0;
		else if(!client1->GetUserName())
			return 1;
		else if(!client2->GetUserName())
			return -1;
		return _tcsicmp(client1->GetUserName(),client2->GetUserName());
	case 1: //size but we use status asc
		return client1->GetSourceFrom() - client2->GetSourceFrom();
	case 2://transfered asc
	case 3://completed asc
		return CompareUnsigned(client1->GetTransferedDown(), client2->GetTransferedDown());
	case 4: //speed asc
		return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());
	case 5: //progress asc
		return client1->GetAvailablePartCount() - client2->GetAvailablePartCount();
	case 6:
		if( client1->GetClientSoft() == client2->GetClientSoft() )
			return client2->GetVersion() - client1->GetVersion();
		return client1->GetClientSoft() - client2->GetClientSoft();
	case 7: //qr asc
		if(client1->GetDownloadState() == DS_DOWNLOADING)
			if(client2->GetDownloadState() == DS_DOWNLOADING)
				return CompareUnsigned(client2->GetDownloadDatarate(), client1->GetDownloadDatarate());
			else
				return -1;
		else if (client2->GetDownloadState() == DS_DOWNLOADING)
			return 1;
		if(client1->GetRemoteQueueRank() == 0 && client1->GetDownloadState() == DS_ONQUEUE && client1->IsRemoteQueueFull() == true) return 1;
		if(client2->GetRemoteQueueRank() == 0 && client2->GetDownloadState() == DS_ONQUEUE && client2->IsRemoteQueueFull() == true) return -1;
		if(client1->GetRemoteQueueRank() == 0) return 1;
		if(client2->GetRemoteQueueRank() == 0) return -1;
		return client1->GetRemoteQueueRank() - client2->GetRemoteQueueRank();
	case 8:
		if( client1->GetDownloadState() == client2->GetDownloadState() ){
			if( client1->IsRemoteQueueFull() && client2->IsRemoteQueueFull() )
				return 0;
			else if( client1->IsRemoteQueueFull() )
				return 1;
			else if( client2->IsRemoteQueueFull() )
				return -1;
			else
				return 0;
		}
		return client1->GetDownloadState() - client2->GetDownloadState();
	default:
		return 0;
	}
}

void CDownloadListCtrl::OnNMDblclkDownloadlist(NMHDR *pNMHDR, LRESULT *pResult) {
	int iSel = GetSelectionMark();
	if (iSel != -1){
		CtrlItem_Struct* content = (CtrlItem_Struct*)this->GetItemData(iSel);
		if (content && content->value){
			if (content->type == FILE_TYPE){
				if (!thePrefs.IsDoubleClickEnabled()){
					CPoint pt;
					::GetCursorPos(&pt);
					ScreenToClient(&pt);
					LVHITTESTINFO hit;
					hit.pt = pt;
					if (HitTest(&hit) >= 0 && (hit.flags & LVHT_ONITEM)){
						LVHITTESTINFO subhit;
						subhit.pt = pt;
						if (SubItemHitTest(&subhit) >= 0 && subhit.iSubItem == 0){
							ShowFileDialog((CPartFile*)content->value);
						}
					}
				}
			}
			else{
				CClientDetailDialog dialog((CUpDownClient*)content->value);
				dialog.DoModal();
			}
		}
	}
	
	*pResult = 0;
}

void CDownloadListCtrl::CreateMenues() {
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_A4AFMenu) VERIFY( m_A4AFMenu.DestroyMenu() );
	if (m_FileMenu) VERIFY( m_FileMenu.DestroyMenu() );

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));

	m_A4AFMenu.CreateMenu();
	m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_THIS, GetResString(IDS_ALL_A4AF_TO_THIS)); // sivka [Tarod]
	m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_OTHER, GetResString(IDS_ALL_A4AF_TO_OTHER)); // sivka
	m_A4AFMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_AUTO, GetResString(IDS_ALL_A4AF_AUTO)); // sivka [Tarod]


	m_FileMenu.CreatePopupMenu();
	m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + " ("+ GetResString(IDS_DOWNLOAD) +")"  );

	m_FileMenu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_DL_PAUSE));
	m_FileMenu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_DL_STOP));
	m_FileMenu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_DL_RESUME));
	m_FileMenu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL) );
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_DL_OPEN) );//<--9/21/02
	m_FileMenu.AppendMenu(MF_STRING,MP_PREVIEW, GetResString(IDS_DL_PREVIEW) );
	m_FileMenu.AppendMenu(MF_STRING,MP_METINFO, GetResString(IDS_DL_INFO) );//<--9/21/02
	m_FileMenu.AppendMenu(MF_STRING,MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL) );

	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR));
	if (thePrefs.IsExtControlsEnabled())
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_A4AFMenu.m_hMenu, GetResString(IDS_A4AF));

	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1) );
	m_FileMenu.AppendMenu(MF_STRING,MP_GETHTMLED2KLINK, GetResString(IDS_DL_LINK2));
	m_FileMenu.AppendMenu(MF_STRING,MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
}

CString CDownloadListCtrl::getTextList() {
	CString out="";

	// Search for file(s)
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){ // const is better
		CtrlItem_Struct* cur_item = it->second;
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);

			char buffer[50+1];
			ASSERT( !file->GetFileName().IsEmpty() );
			strncpy(buffer, file->GetFileName(), 50);
			buffer[50] = '\0';

			CString temp;
			temp.Format("\n%s\t [%.1f%%] %i/%i - %s",
						buffer,
						file->GetPercentCompleted(),
						file->GetTransferingSrcCount(),
						file->GetSourceCount(), 
						file->getPartfileStatus());
			
			out += temp;
		}
	}

	return out;
}

void CDownloadListCtrl::ShowFilesCount() {
	uint16	count=0;
	CString counter;
	CtrlItem_Struct* cur_item;

	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file=(CPartFile*)cur_item->value;
			if (file->CheckShowItemInGivenCat(curTab))
				++count;
		}
	}

	counter.Format("%s (%u)", GetResString(IDS_TW_DOWNLOADS),count);
	theApp.emuledlg->transferwnd->GetDlgItem(IDC_DOWNLOAD_TEXT)->SetWindowText(counter);
}

void CDownloadListCtrl::ShowSelectedFileDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p);
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly! 

	CtrlItem_Struct* content = (CtrlItem_Struct*)this->GetItemData(GetSelectionMark());
	if (content->type == FILE_TYPE){
		CPartFile* file = (CPartFile*)content->value;

		if ((file->HasComment() || file->HasRating()) && p.x < 13){
			ShowFileDialog(NULL, true);
		}
		else{
			ShowFileDialog();
		}
	}
	else{
		CUpDownClient* client = (CUpDownClient*)content->value;
		CClientDetailDialog dialog(client);
		dialog.DoModal();
	}
}

void CDownloadListCtrl::ChangeCategory(int newsel){

	SetRedraw(FALSE);

	// remove all displayed files with a different cat and show the correct ones
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if (!file->CheckShowItemInGivenCat(newsel))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

	SetRedraw(TRUE);
	curTab=newsel;
	ShowFilesCount();
}

void CDownloadListCtrl::HideFile(CPartFile* tohide)
{
	HideSources(tohide);

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(tohide);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if(result != (-1)) {
			DeleteItem(result);
			return;
		}
	}
}

void CDownloadListCtrl::ShowFile(CPartFile* toshow){
	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toshow);
	ListItems::const_iterator it = rangeIt.first;
	if(it != rangeIt.second){
		CtrlItem_Struct* updateItem  = it->second;

		// Check if entry is already in the List
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		sint16 result = FindItem(&find);
		if(result == (-1))
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)updateItem);
	}
}

void CDownloadListCtrl::GetDisplayedFiles(CArray<CPartFile*,CPartFile*> *list){
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			list->Add(file);
		}
	}	
}

void CDownloadListCtrl::MoveCompletedfilesCat(uint8 from, uint8 to){
	uint8 mycat;

	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator.
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (!file->IsPartFile()){
				mycat=file->GetCategory();
				if ( mycat>=min(from,to) && mycat<=max(from,to)) {
					if (mycat==from) 
						file->SetCategory(to); 
					else
						if (from<to) file->SetCategory(mycat-1);
							else file->SetCategory(mycat+1);
				}
			}
		}
	}
}

void CDownloadListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	/*TRACE("CDownloadListCtrl::OnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
	if (pDispInfo->item.mask & LVIF_TEXT)
		TRACE(" LVIF_TEXT");
	if (pDispInfo->item.mask & LVIF_IMAGE)
		TRACE(" LVIF_IMAGE");
	if (pDispInfo->item.mask & LVIF_STATE)
		TRACE(" LVIF_STATE");
	TRACE("\n");*/

	// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
	// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
	// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
	// it needs to know the contents of the primary item.
	//
	// But, the listview control sends this notification all the time, even if we do not search for an item. At least
	// this notification is only sent for the visible items and not for all items in the list. Though, because this
	// function is invoked *very* often, no *NOT* put any time consuming code here in.

    if (pDispInfo->item.mask & LVIF_TEXT){
        const CtrlItem_Struct* pItem = reinterpret_cast<CtrlItem_Struct*>(pDispInfo->item.lParam);
        if (pItem != NULL && pItem->value != NULL){
			if (pItem->type == FILE_TYPE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CPartFile*)pItem->value)->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else if (pItem->type == UNAVAILABLE_SOURCE || pItem->type == AVAILABLE_SOURCE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (((CUpDownClient*)pItem->value)->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CUpDownClient*)pItem->value)->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else
				ASSERT(0);
        }
    }
    *pResult = 0;
}

void CDownloadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != '\0')
				pGetInfoTip->pszText[0] = '\0';
			return;
		}

		CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(pGetInfoTip->iItem);
		if (content && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString info;

			// build info text and display it
			if (content->type == 1) // for downloading files
			{
				CPartFile* partfile = (CPartFile*)content->value;
				info=partfile->GetInfoSummary(partfile);
			}
			else if (content->type == 3 || content->type == 2) // for sources
			{
				CUpDownClient* client = (CUpDownClient*)content->value;
				in_addr server;
				server.S_un.S_addr = client->GetServerIP();

				info.Format(GetResString(IDS_NICKNAME) + _T(" %s\n")
							+ GetResString(IDS_SERVER) + _T(": %s:%d\n")
							+ GetResString(IDS_SOURCEINFO),
							client->GetUserName(),
							inet_ntoa(server), client->GetServerPort(),
							client->GetAskedCountDown(), client->GetAvailablePartCount());

				if (content->type == 2)
				{	// normal client
					info += GetResString(IDS_CLIENTSOURCENAME) + CString(client->GetClientFilename());
				}
				else
				{	// client asked twice
					info += GetResString(IDS_ASKEDFAF);
				}
				//-For File Comment-// 
				try { 
					if (content->type==2){
						if (client->GetFileComment() != "") { 
							info += "\n" + GetResString(IDS_CMT_READ)  + " " + CString(client->GetFileComment()); 
						} 
						else { 
							//No comment entered 
							info += "\n" + GetResString(IDS_CMT_NONE); 
						} 
						info += "\n" + GetRateString(client->GetFileRate());
					}
				}
				catch(...) {
					ASSERT(0);
					//Information not received = not connected or connecting 
					info += "\n" + GetResString(IDS_CMT_NOTCONNECTED); 
				} 
				//-End file comment-//
			}

			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

void CDownloadListCtrl::ShowFileDialog(CPartFile* pFile, bool bOpenCommentsPage)
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
		{
			const CtrlItem_Struct* pCtrlItem = (CtrlItem_Struct*)GetItemData(iItem);
			if (pCtrlItem->type == FILE_TYPE)
				aFiles.Add((CPartFile*)pCtrlItem->value);
		}
	}

	if (aFiles.GetSize() > 0)
	{
		CFileDetailDialog dialog(&aFiles, bOpenCommentsPage);
		dialog.DoModal();
	}
}
