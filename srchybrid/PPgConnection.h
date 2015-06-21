#pragma once

class CPPgConnection : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgConnection)

public:
	CPPgConnection();
	virtual ~CPPgConnection();

// Dialog Data
	enum { IDD = IDD_PPG_CONNECTION };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void LoadSettings(void);
	
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnEnChangeUDPDisable();
	afx_msg void OnLimiterChange();
	afx_msg void OnBnClickedWizard();
	void Localize(void);

private:
	void ShowLimitValues();
	bool guardian;
public:
	afx_msg void OnBnClickedNetworkKademlia();
};