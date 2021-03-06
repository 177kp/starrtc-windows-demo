// CMinClassListDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "starrtcdemo.h"
#include "CMinClassListDlg.h"
#include "XHChatroomManager.h"
#include "afxdialogex.h"
#include "CInterfaceUrls.h"
#include "CMinClassDlg.h"
#include "CCreateLiveDialog.h"
// CMinClassListDlg 对话框

IMPLEMENT_DYNAMIC(CMinClassListDlg, CDialogEx)

CMinClassListDlg::CMinClassListDlg(CUserManager* pUserManager, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MIN_CLASS_LIST, pParent)
{
	m_pUserManager = pUserManager;
	m_pConfig = NULL;
	XHLiveManager::addChatroomGetListListener(this);
}

CMinClassListDlg::~CMinClassListDlg()
{
	mVLivePrograms.clear();
	m_pConfig = NULL;
}

void CMinClassListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MIN_CLASS, m_MinClassList);
}


BEGIN_MESSAGE_MAP(CMinClassListDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_MIN_CLASS_BRUSH_LIST, &CMinClassListDlg::OnBnClickedButtonMinClassBrushList)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_NEW_MIN_CLASS, &CMinClassListDlg::OnBnClickedButtonCreateNewMinClass)
	ON_NOTIFY(NM_CLICK, IDC_LIST_MIN_CLASS, &CMinClassListDlg::OnNMClickListMinClass)
END_MESSAGE_MAP()
BOOL CMinClassListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LONG lStyle;
	lStyle = GetWindowLong(m_MinClassList.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_MinClassList.m_hWnd, GWL_STYLE, lStyle);

	DWORD dwStyleLiveList = m_MinClassList.GetExtendedStyle();
	dwStyleLiveList |= LVS_EX_FULLROWSELECT;                                        //选中某行使整行高亮(LVS_REPORT)
	dwStyleLiveList |= LVS_EX_GRIDLINES;                                            //网格线(LVS_REPORT)
	//dwStyle |= LVS_EX_CHECKBOXES;                                            //CheckBox
	m_MinClassList.SetExtendedStyle(dwStyleLiveList);

	m_MinClassList.InsertColumn(LIVE_VIDEO_ID, _T("ID"), LVCFMT_LEFT, 110);
	m_MinClassList.InsertColumn(LIVE_VIDEO_NAME, _T("Name"), LVCFMT_LEFT, 120);
	m_MinClassList.InsertColumn(LIVE_VIDEO_CREATER, _T("Creator"), LVCFMT_LEFT, 80);
	m_MinClassList.InsertColumn(LIVE_VIDEO_STATUS, _T("liveState"), LVCFMT_LEFT, 100);

	getMinClassList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


// CMinClassListDlg 消息处理程序
// CInteracteLiveDlg 消息处理程序
void CMinClassListDlg::getMinClassList()
{
	char strListType[10] = { 0 };
	sprintf_s(strListType, "%d,%d", CHATROOM_LIST_TYPE_CLASS, CHATROOM_LIST_TYPE_CLASS_PUSH);
	if (m_pConfig != NULL && m_pConfig->m_bAEventCenterEnable)
	{
		list<ChatroomInfo> listData;
		CInterfaceUrls::demoQueryList(strListType, listData);
		chatroomQueryAllListOK(listData);
	}
	else
	{
		XHLiveManager::getLiveList("", strListType);
	}	
}

void CMinClassListDlg::setConfig(CConfigManager* pConfig)
{
	m_pConfig = pConfig;
}

/**
* 查询聊天室列表回调
*/
int CMinClassListDlg::chatroomQueryAllListOK(list<ChatroomInfo>& listData)
{
	mVLivePrograms.clear();
	m_MinClassList.DeleteAllItems();

	list<ChatroomInfo>::iterator iter = listData.begin();
	int i = 0;
	for (; iter != listData.end(); iter++)
	{
		CLiveProgram liveProgram;
		liveProgram.m_strName = (char*)(*iter).m_strName.c_str();
		liveProgram.m_strId = (char*)(*iter).m_strRoomId.c_str();
		liveProgram.m_strCreator = (char*)(*iter).m_strCreaterId.c_str();
		mVLivePrograms.push_back(liveProgram);
	}
	int nRowIndex = 0;
	CString strStatus = "";
	for (int i = 0; i < (int)mVLivePrograms.size(); i++)
	{
		m_MinClassList.InsertItem(i, mVLivePrograms[i].m_strName);
		//m_liveList.AddString(mVLivePrograms[i].m_strName);
		m_MinClassList.SetItemText(i, LIVE_VIDEO_ID, mVLivePrograms[i].m_strId);

		m_MinClassList.SetItemText(i, LIVE_VIDEO_CREATER, mVLivePrograms[i].m_strCreator);
		if (mVLivePrograms[i].m_liveState)
		{
			strStatus = "正在直播";
		}
		else
		{
			strStatus = "直播未开始";
		}
		m_MinClassList.SetItemText(i, LIVE_VIDEO_STATUS, strStatus);
	}
	return 0;
}


void CMinClassListDlg::OnBnClickedButtonMinClassBrushList()
{
	getMinClassList();
}


void CMinClassListDlg::OnBnClickedButtonCreateNewMinClass()
{
	CString strName = "";
	bool bPublic = false;
	CCreateLiveDialog dlg(m_pUserManager);
	if (dlg.DoModal() == IDOK)
	{
		strName = dlg.m_strLiveName;
		bPublic = dlg.m_bPublic;
		CMinClassDlg dlg(m_pUserManager);
		dlg.setConfig(m_pConfig);
		bool bRet = dlg.createMinClass(strName.GetBuffer(0));
		if (bRet)
		{
			dlg.m_bWatch = false;
			dlg.DoModal();
		}
		return;
	}
	
}


void CMinClassListDlg::OnNMClickListMinClass(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	int nItem = -1;
	if (pNMItemActivate != NULL)
	{
		nItem = pNMItemActivate->iItem;
		if (nItem >= 0)
		{
			CString strId = m_MinClassList.GetItemText(nItem, LIVE_VIDEO_ID);
			CString strName = m_MinClassList.GetItemText(nItem, LIVE_VIDEO_NAME);
			CString strCreater = m_MinClassList.GetItemText(nItem, LIVE_VIDEO_CREATER);

			CString strUserId = m_pUserManager->m_ServiceParam.m_strUserId.c_str();

			if (strId.GetLength() == 32)
			{
				CMinClassDlg dlg(m_pUserManager);
				if (dlg.m_pCurrentLive == NULL)
				{
					dlg.m_pCurrentLive = new CLiveProgram();
				}
				dlg.m_pCurrentLive->m_strId = strId;
				dlg.m_pCurrentLive->m_strName = strName;
				dlg.m_pCurrentLive->m_strCreator = strCreater;
				if (strCreater == strUserId)
				{
					dlg.m_bWatch = false;
				}
				else
				{
					dlg.m_bWatch = true;
				}
				dlg.DoModal();
			}
			else
			{
				CString strMessage = "";
				strMessage.Format("err id %s", strId);
				AfxMessageBox(strMessage);
			}	
		}
	}
} 
