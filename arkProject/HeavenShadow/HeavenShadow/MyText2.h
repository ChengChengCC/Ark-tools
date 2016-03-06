#pragma once


// CMyText2

class CMyText2 : public CStatic
{
	DECLARE_DYNAMIC(CMyText2)

public:
	CMyText2();
	virtual ~CMyText2();
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
public:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


