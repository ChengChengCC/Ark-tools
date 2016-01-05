
// RegisterManagerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <WinIoCtl.h>
#include <Sddl.h>
#include <list>
#include "RegFindDlg.h"
using namespace std;
typedef struct _KEY_BASIC_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   NameLength;
	WCHAR   Name[1];           
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;


typedef enum _KEY_INFORMATION_CLASS {
	KeyBasicInformation,
	KeyNodeInformation,
	KeyFullInformation,
	KeyNameInformation,
	KeyCachedInformation,
	KeyFlagsInformation,
	KeyVirtualizationInformation,
	KeyHandleTagsInformation,
	MaxKeyInfoClass 
} KEY_INFORMATION_CLASS;

typedef struct _KEY_VALUE_FULL_INFORMATION {
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   DataOffset;
	ULONG   DataLength;
	ULONG   NameLength;
	WCHAR   Name[1];          
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
	KeyValueBasicInformation,
	KeyValueFullInformation,
	KeyValuePartialInformation,
	KeyValueFullInformationAlign64,
	KeyValuePartialInformationAlign64,
	MaxKeyValueInfoClass  
} KEY_VALUE_INFORMATION_CLASS;


#define	CLASSES_ROOT		L"\\Registry\\Machine\\SOFTWARE\\Classes"
#define	LOCAL_MACHINE		L"\\Registry\\Machine"
#define USERS				L"\\Registry\\User"
#define CURRENT_CONFIGL     L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current"



#define CTL_CREATE_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x833,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_OPEN_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_ENUM_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_ENUM_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x832,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_SET_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x834,METHOD_NEITHER,FILE_ANY_ACCESS)


#define CTL_DELETE_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x835,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_RENAME_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x836,METHOD_NEITHER,FILE_ANY_ACCESS)


#define CTL_DELETE_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x837,METHOD_NEITHER,FILE_ANY_ACCESS)

#define OBJ_CASE_INSENSITIVE    0x00000040L
#define InitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof(OBJECT_ATTRIBUTES);          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
}UNICODE_STRING, *PUNICODE_STRING;


typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;       
	PVOID SecurityQualityOfService; 
}OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;


// CRegisterManagerDlg 对话框
class CRegisterManagerDlg : public CDialogEx
{
// 构造
public:
	CRegisterManagerDlg(CWnd* pParent = NULL);	// 标准构造函数
	CRegisterManagerDlg::~CRegisterManagerDlg();
	BOOL InitControlTree();
	VOID InitRegistry();
	VOID EnumSubKeys(CString strKey, HTREEITEM hItem, BOOL bSubSubKey = FALSE);
	VOID DeleteSubTree(HTREEITEM hTreeItem);
	VOID CRegisterManagerDlg::FreeUnicodeString(UNICODE_STRING *uniString);
	BOOL InitUnicodeString(PUNICODE_STRING uniString, WCHAR *wzString);
	BOOL CRegisterManagerDlg::OpenKey(OUT PHANDLE  KeyHandle,
		IN ACCESS_MASK  DesiredAccess,
		IN POBJECT_ATTRIBUTES  ObjectAttributes
		);
	BOOL CRegisterManagerDlg::EnumerateKey(IN HANDLE  KeyHandle,
		IN ULONG  ulIndex,
		IN ULONG  ulKeyInformationClass,
		OUT PVOID  KeyInformation,
		IN ULONG   ulLength,
		OUT PULONG ResultLength
		);
	void CRegisterManagerDlg::EnumCurrentUserSubKeys(HTREEITEM hChild);
	BOOL CRegisterManagerDlg::GetCurrentUserKeyPath(OUT WCHAR *wzCurrentUserPath);
	void CRegisterManagerDlg::EnumSubSubKeys(CString strKey, HTREEITEM hItem);
    void CRegisterManagerDlg::EnumValues(CString strKey);

	BOOL CRegisterManagerDlg::EnumerateValueKey(IN HANDLE  KeyHandle,
		IN ULONG  ulIndex,
		IN ULONG  ulKeyValueInformationClass,
		OUT PVOID  KeyValueInformation,
		IN ULONG  ulLength,
		OUT PULONG  ulResultLength
		);
	CString CRegisterManagerDlg::GetKeyType(ULONG ulType);
	CString CRegisterManagerDlg::GetKeyData(ULONG ulType, WCHAR* wzData, ULONG ulDataLength);


	VOID CRegisterManagerDlg::InitializeComboBox();
	void CRegisterManagerDlg::GetAllRegPath(std::list <CString> &RegPathList);
	void CRegisterManagerDlg::InitRegPathList();
	CString CRegisterManagerDlg::ComboString2KeyPath();
	HTREEITEM CRegisterManagerDlg::GetTreeSubItemByName(HTREEITEM hPatentItem, CString strName);
	CString CRegisterManagerDlg::String2KeyPath();

	BOOL CRegisterManagerDlg::CreateKey(
		OUT PHANDLE  KeyHandle,
		IN ACCESS_MASK  DesiredAccess,
		IN POBJECT_ATTRIBUTES  ObjectAttributes,
		IN ULONG  ulTitleIndex,
		IN PUNICODE_STRING  uniClass,
		IN ULONG  ulCreateOptions,
		OUT PULONG  Disposition
		);

	void CRegisterManagerDlg::SetValueKey(ULONG ulType, PVOID Data, ULONG ulDataSize);

	BOOL CRegisterManagerDlg::SetValueKey(
		IN HANDLE  KeyHandle,
		IN PUNICODE_STRING  uniValueName,
		IN ULONG  ulTitleIndex,
		IN ULONG  ulType,
		IN PVOID  Data,
		IN ULONG  ulDataSize
		);


	void CRegisterManagerDlg::DeleteKeyAndSubKeys(CString strKey);
	BOOL CRegisterManagerDlg::DeleteKey(IN HANDLE KeyHandle);
	void CRegisterManagerDlg::UpdateKey(CString strKey, HTREEITEM hItem);


	BOOL CRegisterManagerDlg::RenameKey(
		IN HANDLE  KeyHandle,
		IN PUNICODE_STRING  uniNewName
		);
	BOOL SetStringToClipboard(CString strImageName);
	VOID CRegisterManagerDlg::ShellImportRegister(
		LPCTSTR ImportFile) ;

	VOID CRegisterManagerDlg::ShellExportRegister(
		CString strItem,		
		CString strFileName);


	void CRegisterManagerDlg::JmpToReg(CString strKey, CString strData);
	void CRegisterManagerDlg::JmpToRegCommon(CString strKey, CString strData);

	BOOL CRegisterManagerDlg::DeleteValueKey(
		IN HANDLE  KeyHandle,
		IN PUNICODE_STRING  uniValueName
		);
	

	PVOID CRegisterManagerDlg::GetValueInfo(CString strKey, CString strValue);
	void CRegisterManagerDlg::CreateValueKey(CString strValue, ULONG ulType, PVOID Data, ULONG ulDataSize);
	ULONG CRegisterManagerDlg::GetValueType(CString strType);
	void CRegisterManagerDlg::ModifyValue(CString strValue, ULONG ulType, PVOID Data, ULONG ulDataSize);
	ULONG CRegisterManagerDlg::HexStringToLong(CString strHex);
	// 对话框数据
	enum { IDD = IDD_REGISTERMANAGER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_Tree;
	CImageList m_TreeImageList;
	CListCtrl m_List;
	CImageList m_ImageList;
	CComboBox m_ComboBox;
	CString m_strComboText;
	WCHAR* m_wzhKeyCurrentUser;   //这个是存储Current_User 键值    注意在析构函数要进行释放内存
	std::list <CString> m_RegPathList;		// 注册表的快速定位项
	ULONG m_nComboBoxCnt;                   // 快速定位的总数
	HTREEITEM m_RightClickTreeItem;         // 在树控件上的右键选择项
	CBitmap m_bmRefresh;
	CBitmap m_bmDelete;
	CBitmap m_bmCopy;
	CBitmap m_bmExport;
	CBitmap m_bmLookfor;
	CRegFindDlg *m_FindRegDlg;            //查找Dlg
	afx_msg void OnTvnItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedGoto();
	afx_msg void OnCbnDropdownCombo();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnRegRefresh();
	afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRegNewKey();
	afx_msg void OnStringValue();
	afx_msg void OnBinaryValue();
	afx_msg void OnDwordValue();
	afx_msg void OnMuiStringValue();
	afx_msg void OnExpandStringValue();
	afx_msg void OnRegDelete();
	afx_msg void OnRegRename();
	afx_msg void OnQwordValue();
	afx_msg void OnRegCopyKeyName();
	afx_msg void OnRegCopyFullKeyName();
	afx_msg void OnRegAddToQuickAddress();
	afx_msg void OnRegInport();
	afx_msg void OnRegExport();
	afx_msg void OnRegLookFor();
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRegListRefresh();
	afx_msg void OnRegListModify();
	afx_msg void OnRegListDelete();
	afx_msg void OnRegListRename();
	afx_msg void OnRegListCopyValue();
	afx_msg void OnRegListCopyValueData();
};
