// PopupWindow.cpp : implementation file
//

#include "main.h"
#include "vst.h"
#include "PopupWindow.h"
#include "EditorThread.h"
#include "VstHost.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CVstApp *theApp;

/////////////////////////////////////////////////////////////////////////////
// CPopupWindow

IMPLEMENT_DYNCREATE(CPopupWindow, CFrameWnd)

CPopupWindow::CPopupWindow():
    plug(NULL)
{}

CPopupWindow::~CPopupWindow()
{
	plug->OnEditorClose();
}


BEGIN_MESSAGE_MAP(CPopupWindow, CFrameWnd)
	//{{AFX_MSG_MAP(CPopupWindow)
	ON_WM_ENTERIDLE()
	ON_WM_TIMER()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupWindow message handlers

void CPopupWindow::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	CFrameWnd::OnEnterIdle(nWhy, pWho);
	
	// TODO: Add your message handler code here
	if(plug) plug->EditorIdle();		
}

void CPopupWindow::SetPlugin(VSTPlugin *p)
{
    plug = p;

    char tmp[256];
    sprintf(tmp,"vst~ - %s",plug->GetName());

	CreateEx( WS_EX_DLGMODALFRAME,AfxRegisterWndClass(CS_DBLCLKS),tmp,WS_CAPTION|WS_THICKFRAME|WS_POPUP|WS_SYSMENU,0,0,0,0,NULL,NULL,NULL);
    
	plug->Dispatch(effEditOpen , 0 , 0 , m_hWnd , 0.0f  );
/*
	CString str = theApp->GetProfileString( "VSTPos" , plug->GetName() , "10,10");
	int idx = str.Find(",");
	CString x = str.Left( idx );
	CString y = str.Right( idx );
	printf(" index is %d left is %s and right is %s" , idx , x , y);	
*/

	DoInit();

	RECT r = plug->GetEditorRect();
	SetWindowPos(&wndTop,plug->getX(),plug->getY(),(r.right - r.left) + 10 , r.bottom - r.top + 30 , SWP_SHOWWINDOW);
//	ShowWindow( SW_SHOW );		
//	BringWindowToTop();
//	SetFocus();
}


//DEL BOOL CPopupWindow::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
//DEL {
//DEL 	// TODO: Add your specialized code here and/or call the base class
//DEL 	
//DEL 	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
//DEL }

void CPopupWindow::DoInit()
{
//	printf("DoInit\n");
	plug->Dispatch(effEditTop,0,0, 0,0.0f);			
//	printf("Dispatched to the top\n");
	SetTimer(0,25,NULL);
}

void CPopupWindow::OnTimer(UINT nIDEvent) 
{
	plug->Dispatch(effEditIdle, 0, 0, NULL, 0.0f);
	CFrameWnd::OnTimer(nIDEvent);
}

void CPopupWindow::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if(plug) plug->setPos(x,y);
/*
	{
		char buf[100];
		sprintf( buf , "%d,%d" , x , y );
		theApp->WriteProfileString( "VSTPos" , plug->GetName() , buf );
	}
*/
}


//DEL void CPopupWindow::OnFinalRelease() 
//DEL {
//DEL 	//
//DEL 	CFrameWnd::OnFinalRelease();
//DEL }

void CPopupWindow::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	plug->StopEditing();
	CFrameWnd::OnClose();
}
