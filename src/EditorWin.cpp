/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "Editor.h"
#include "VstHost.h"
#include <flext.h>

#include <map>

#if FLEXT_OS == FLEXT_OS_WIN
// only Windows code is situated in this file

#include <windows.h>

typedef std::map<flext::thrid_t,VSTPlugin *> WndMap;
static WndMap wndmap;
static flext::ThrMutex mapmutex;

#define TIMER_INTERVAL 25
#define WCLNAME "vst~-class"


static LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    mapmutex.Lock();
    VSTPlugin *plug = wndmap[flext::GetThreadId()];
    mapmutex.Unlock();
    FLEXT_ASSERT(plug != NULL);

    LRESULT res = 0;

//    post("Message %x",msg);
    switch(msg) {
//        case WM_NCREATE: res = TRUE; break;
        case WM_CREATE: 
            // Initialize the window. 
            plug->StartEditing(hwnd);
            break; 
        case WM_CLOSE:
#ifdef FLEXT_DEBUG
            flext::post("WM_CLOSE");
#endif
	        plug->StopEditing(); // this sets plug->hwnd = NULL
            DestroyWindow(hwnd);
            break; 
        case WM_DESTROY: 
#ifdef FLEXT_DEBUG
            flext::post("WM_DESTROY");
#endif
            // stop editor thread
            PostQuitMessage(0); 
            break; 
        case WM_TIMER:
//        	plug->Dispatch(effEditIdle, 0, 0, NULL, 0.0f);
  //          break; 
        case WM_ENTERIDLE:
            plug->EditorIdle();		
            break; 
        case WM_MOVE: {
            // ignore after WM_CLOSE so that x,y positions are preserved
            if(!plug->IsEdited()) break;

            WORD wx = LOWORD(lp),wy = HIWORD(lp);
            short x = reinterpret_cast<short &>(wx),y = reinterpret_cast<short &>(wy);
            // x and y are the coordinates of the client rect (= actual VST interface)
            plug->SetPos(x,y,false);
#ifdef FLEXT_DEBUG
            flext::post("WM_MOVE x/y=%i/%i",x,y);
#endif
            break; 
        }
/*
        case WM_PAINT: {
            // Paint the window's client area. 
            RECT rect;
            GetUpdateRect(hwnd,rect,FALSE);
            plug->Paint(rect);
            break;  
        }
*/
        case WM_NCCALCSIZE:
#ifdef FLEXT_DEBUG
//            flext::post("WM_NCCALCSIZE %i/%x",wp?1:0,lp);
            if(wp) {
                NCCALCSIZE_PARAMS *p = (NCCALCSIZE_PARAMS *)lp;
                flext::post("WM_NCCALCSIZE %i/%i %i/%i",p->lppos->x,p->lppos->y,p->lppos->cx,p->lppos->cy);
                flext::post("\t1: %i/%i %i/%i",p->rgrc[0].left,p->rgrc[0].top,p->rgrc[0].right,p->rgrc[0].bottom);
                flext::post("\t2: %i/%i %i/%i",p->rgrc[1].left,p->rgrc[1].top,p->rgrc[1].right,p->rgrc[1].bottom);
                flext::post("\t3: %i/%i %i/%i",p->rgrc[2].left,p->rgrc[2].top,p->rgrc[2].right,p->rgrc[2].bottom);
            }
#endif
//            if(wp) { res = WVR_ALIGNTOP|WVR_ALIGNLEFT|WVR_REDRAW; }
            res = DefWindowProc(hwnd,msg,wp,lp); 
            break;
#ifdef FLEXT_DEBUG
        case WM_SIZE: {
            WORD wx = LOWORD(lp),wy = HIWORD(lp);
            short x = reinterpret_cast<short &>(wx),y = reinterpret_cast<short &>(wy);
            // x and y are the coordinates of the client rect (= actual VST interface)
            flext::post("WM_SIZE x/y=%i/%i",x,y);
            break; 
        }
#endif

        default: 
            res = DefWindowProc(hwnd,msg,wp,lp); 
    }
    return res;
}

static void windowsize(HWND wnd,int x,int y,int w,int h,bool caption)
{
	WINDOWINFO winfo;
	winfo.cbSize = sizeof(winfo);
	GetWindowInfo(wnd,&winfo);
	TITLEBARINFO tinfo;
	tinfo.cbSize = sizeof(tinfo);
    if(!caption || !GetTitleBarInfo(wnd,&tinfo)) 
        // clear structure
        memset(&tinfo,0,sizeof(tinfo));

	SetWindowPos(wnd,HWND_TOP,
    	x-(winfo.rcClient.left-winfo.rcWindow.left),
        y-(winfo.rcClient.top-winfo.rcWindow.top),
		w+winfo.cxWindowBorders*2,
		h+(tinfo.rcTitleBar.bottom-tinfo.rcTitleBar.top)+winfo.cyWindowBorders*2, 
		SWP_SHOWWINDOW
	);

}

static void threadfun(flext::thr_params *p)
{
    flext::RelPriority(-2);

    VSTPlugin *plug = (VSTPlugin *)p;
    HINSTANCE hinstance = (HINSTANCE)GetModuleHandle(NULL);
    flext::thrid_t thrid = flext::GetThreadId();

    mapmutex.Lock();
    wndmap[thrid] = plug;
    mapmutex.Unlock();    


    char tmp[256];
    sprintf(tmp,"vst~ - %s",plug->GetName());

    HWND wnd = CreateWindow( 
        WCLNAME,tmp,
        (plug->GetCaption()?WS_BORDER|WS_CAPTION:0)|WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
        NULL,NULL,
        hinstance,NULL
    );

    if(!wnd) 
        FLEXT_LOG1("wnd == NULL: %i",GetLastError());
    else {
//        plug->Dispatch(effEditOpen , 0 , 0 , wnd, 0.0f  );  // Done in WNDPROC!!
    /*
	    CString str = theApp->GetProfileString( "VSTPos" , plug->GetName() , "10,10");
	    int idx = str.Find(",");
	    CString x = str.Left( idx );
	    CString y = str.Right( idx );
	    printf(" index is %d left is %s and right is %s" , idx , x , y);	
    */

//	    plug->Dispatch(effEditTop,0,0, 0,0.0f);			
    //	printf("Dispatched to the top\n");

	    SetTimer(wnd,0,TIMER_INTERVAL,NULL);

	    ERect r;
        plug->GetEditorRect(r);
        windowsize(wnd,plug->GetX(),plug->GetY(),r.right-r.left,r.bottom-r.top,plug->GetCaption());
#ifdef FLEXT_DEBUG
        flext::post("Editor rect left/top=%i/%i, right/bottom=%i/%i",r.left,r.top,r.right,r.bottom);
#endif

	//	SetFocus();

        // Message pump
        MSG msg;
        BOOL bRet;
        while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0) { 
            if (bRet == -1) {
                // handle the error and possibly exit
                FLEXT_LOG1("GetMessage error: %i",GetLastError());
            }
            else {
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        }
    }

//    UnregisterClass(wcx.lpszClassName,hinstance);

    mapmutex.Lock();
    wndmap.erase(thrid);
    mapmutex.Unlock();
}

void SetupEditor()
{
    HINSTANCE hinstance = (HINSTANCE)GetModuleHandle(NULL);

    // Fill in the window class structure with parameters that describe the main window. 
    WNDCLASS wcx; 
    wcx.style = CS_DBLCLKS; // | CS_HREDRAW | CS_VREDRAW;   // redraw if size changes 
    wcx.lpfnWndProc = wndproc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = hinstance;         // handle to instance 
    wcx.hIcon = NULL; //LoadIcon(NULL, IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = NULL; //GetStockObject(WHITE_BRUSH);                  // white background brush 
    wcx.lpszMenuName = NULL;    // name of menu resource 
    wcx.lpszClassName = WCLNAME;  // name of window class 

    ATOM at = RegisterClass(&wcx); 
    FLEXT_ASSERT(at);
}

void StartEditor(VSTPlugin *p)
{
    flext::LaunchThread(threadfun,reinterpret_cast<flext::thr_params *>(p));
}

void StopEditor(VSTPlugin *p) 
{
    PostMessage(p->EditorHandle(),WM_CLOSE,0,0); 
    flext::StopThread(threadfun,reinterpret_cast<flext::thr_params *>(p));
}

void ShowEditor(VSTPlugin *p,bool show) 
{
    ShowWindow(p->EditorHandle(),show); 
}

void MoveEditor(VSTPlugin *p,int x,int y) 
{
    HWND wnd = p->EditorHandle();

	WINDOWINFO winfo;
	winfo.cbSize = sizeof(winfo);
	GetWindowInfo(wnd,&winfo);

    SetWindowPos(wnd,NULL,
		x-(winfo.rcClient.left-winfo.rcWindow.left),y-(winfo.rcClient.top-winfo.rcWindow.top),
        0,0,
		SWP_NOSIZE|SWP_NOZORDER
	);
}

void SizeEditor(VSTPlugin *p,int x,int y) 
{
    SetWindowPos(p->EditorHandle(),NULL,0,0,x,y,SWP_NOMOVE|SWP_NOZORDER);
}

void CaptionEditor(VSTPlugin *plug,bool c)
{
    HWND wnd = plug->EditorHandle();
    LONG ns,style = GetWindowLong(wnd,GWL_STYLE);
    if(c) ns = style|WS_BORDER|WS_CAPTION;
    else ns = style&~(WS_BORDER|WS_CAPTION);
    if(ns != style) {
        SetWindowLong(wnd,GWL_STYLE,ns);

        // maybe resize below is just enough ... SWP_FRAMECHANGED not needed
//        SetWindowPos(wnd,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);

        ERect r;
        plug->GetEditorRect(r);
        windowsize(wnd,plug->GetX(),plug->GetY(),r.right-r.left,r.bottom-r.top,c);
    }
}

void TitleEditor(VSTPlugin *p,const char *t) 
{
    SetWindowText(p->EditorHandle(),t);
}

bool IsEditorShown(const VSTPlugin *p) 
{
    return IsWindowVisible(p->EditorHandle()) != FALSE;
}

#endif // FLEXT_OS_WIN
