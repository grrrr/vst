// EditorThread.cpp : implementation file
//

#include "stdafx.h"
#include "vst.h"
#include "EditorThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorThread

IMPLEMENT_DYNCREATE(CEditorThread, CWinThread)

CEditorThread::CEditorThread(): pop(NULL) {}

CEditorThread::~CEditorThread() {}


BOOL CEditorThread::InitInstance()
{
    SetThreadPriority(THREAD_PRIORITY_LOWEST);

	m_pMainWnd = pop = new CPopupWindow;
	pop->SetPlugin( plug);	// window class, size etc. is set here!
	return TRUE;
}

int CEditorThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CEditorThread, CWinThread)
	//{{AFX_MSG_MAP(CEditorThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorThread message handlers

void CEditorThread::SetPlugin(VSTPlugin *p)
{
	plug = p;	
}

void CEditorThread::Close()
{
    if(pop) pop->SendMessage(WM_CLOSE);
}

void CEditorThread::Show(bool show)
{
    if(pop) pop->ShowWindow(show);
}