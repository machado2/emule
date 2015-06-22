#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "IrcNickListCtrl.h"
#include "IrcChannelListCtrl.h"
#include "IrcChannelTabCtrl.h"

class CIrcMain;

class CIrcWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CIrcWnd)

public:
	CIrcWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIrcWnd();
	void Localize();
	bool GetLoggedIn()				
	{
		return m_bLoggedIn;
	}
	void SetLoggedIn( bool flag )
	{
		m_bLoggedIn = flag;
	}
	void SetSendFileString( CString in_file )
	{
		m_sSendString = in_file;
	}
	CString GetSendFileString()	
	{
		return m_sSendString;
	}
	bool IsConnected()
	{
		return m_bConnected;
	}
	void UpdateFonts(CFont* pFont);
	void ParseChangeMode( CString channel, CString changer, CString commands, CString params );
	void AddStatus( CString received, ... );
	void AddInfoMessage( CString channelName, CString received, ... );
	void AddMessage( CString channelName, CString targetname, CString line,...);
	void SetConnectStatus( bool connected );
	void NoticeMessage( CString source, CString message );
	CString StripMessageOfFontCodes( CString temp );
	CString StripMessageOfColorCodes( CString temp );
	void SetTitle( CString channel, CString title );
	void SendString( CString send );
	CIrcChannelTabCtrl m_channelselect;
	CIrcNickListCtrl m_nicklist;
	CIrcChannelListCtrl m_serverChannelList;
	CEdit titleWindow;
	CEdit inputWindow;
	CIrcMain* m_pIrcMain;
	enum { IDD = IDD_IRC };
	afx_msg void OnBnClickedClosechat(int nItem=-1);
protected:
	virtual BOOL OnInitDialog();
	virtual void OnSize(UINT nType, int cx, int cy);
	virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnBnClickedBnIrcconnect();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedChatsend();
private:
	void OnChatTextChange();
	void AutoComplete();
	CString m_sSendString;
	bool m_bLoggedIn;
	bool m_bConnected;
};
