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

#if FLEXT_OS == FLEXT_OS_WIN
// only Windows code is situated in this file

#include <windows.h>

#include <map>

typedef std::map<flext::thrid_t,VSTPlugin *> WndMap;
static WndMap wndmap;
static flext::ThrMutex mapmutex;

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
            plug->SetEditWindow(hwnd);
            break; 
        case WM_CLOSE:
	        plug->StopEditing();
            DestroyWindow(hwnd);
            break; 
        case WM_DESTROY: 
            PostQuitMessage(0); 
            break; 
        case WM_TIMER:
//        	plug->Dispatch(effEditIdle, 0, 0, NULL, 0.0f);
  //          break; 
        case WM_ENTERIDLE:
            plug->EditorIdle();		
            break; 
        case WM_MOVE:
            plug->setPos(LOWORD(lp),HIWORD(lp));
            break; 
/*
        case WM_PAINT: 
            // Paint the window's client area. 
            break;  
        case WM_SIZE: 
            // Set the size and position of the window. 
            break; 
*/
        default: 
            res = DefWindowProc(hwnd,msg,wp,lp); 
    }
    return res;
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
        WS_BORDER|WS_CAPTION|/*WS_THICKFRAME|*/WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX,
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

	    SetTimer(wnd,0,25,NULL);

	    RECT r = plug->GetEditorRect();
	    SetWindowPos(wnd,HWND_TOPMOST,plug->getX(),plug->getY(),(r.right - r.left) + 6 , r.bottom - r.top + 26 , SWP_SHOWWINDOW);
    //	ShowWindow( SW_SHOW );		
    //  BringWindowToTop(wnd);
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

#endif // FLEXT_OS_WIN
