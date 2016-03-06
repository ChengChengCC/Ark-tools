#pragma once


// CMyList

class CMyList : public CListCtrl
{
	DECLARE_DYNAMIC(CMyList)

public:
	CMyList();
	virtual ~CMyList();

	//插入数据，可设置字体颜色
	int InsertItem(int nItem,LPCTSTR lpText,COLORREF fontcolor=RGB(0,0,0));

protected:
	DECLARE_MESSAGE_MAP()
};


