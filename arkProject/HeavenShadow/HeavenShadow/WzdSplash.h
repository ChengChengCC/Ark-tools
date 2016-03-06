#pragma once
// CWzdSplash
class CWzdSplash : public CWnd
{
	DECLARE_DYNAMIC(CWzdSplash)
public:
	CWzdSplash();
	virtual ~CWzdSplash();
protected:
	DECLARE_MESSAGE_MAP()
public:
	CBitmap m_bitmap;
//	void Create(void);
	void Create(UINT nBitmapID);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

