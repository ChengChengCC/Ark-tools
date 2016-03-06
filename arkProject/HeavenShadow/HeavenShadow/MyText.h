#pragma once


// CMyText

class CMyText : public CStatic
{
	DECLARE_DYNAMIC(CMyText)

public:
	CMyText();
	virtual ~CMyText();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
public:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


