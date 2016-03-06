#pragma once


// CMyPicButton

class CMyPicButton : public CStatic
{
	DECLARE_DYNAMIC(CMyPicButton)

public:
	CMyPicButton();
	virtual ~CMyPicButton();

	BOOL m_bIsHand;

protected:
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnMouseMove(UINT nFlags, CPoint point);

public:
	CString m_szToolTipText;
	CToolTipCtrl m_toolTip;
	void SetToolTipText(const CString &szTip);
	void UpdateToolTipText();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PreSubclassWindow();
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


