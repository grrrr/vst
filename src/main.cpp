/* 

vst - VST plugin object for PD 
based on the work of mark@junklight.com

Copyright (c)2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "vst.h"

#include "EditorThread.h"
#include "VstHost.h"

#include <stdlib.h>
#include <direct.h>
#include <io.h>

#define VST_VERSION "0.1.0pre"

#if 0
/* ----- MFC stuff ------------- */

BEGIN_MESSAGE_MAP(CVstApp, CWinApp)
	//{{AFX_MSG_MAP(CVstApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CVstApp::CVstApp() {}

CVstApp theApp;

/* ----- MFC stuff ------------- */
#endif


class vst:
	public flext_dsp
{
	FLEXT_HEADER_S(vst,flext_dsp,Setup)

public:
	vst(I argc,const A *argv);
	~vst();

protected:
    virtual V m_dsp(I n,t_signalvec const *insigs,t_signalvec const *outsigs);
    virtual V m_signal(I n,R *const *insigs,R *const *outsigs);

    BL ms_plug(I argc,const A *argv);
    BL ms_plug(const AtomList &args) { return ms_plug(args.Count(),args.Atoms()); }
    V mg_plug(AtomList &sym) const { sym(1); SetString(sym[0],plugname); }

    V ms_edit(BL on);
    V mg_edit(BL &ed) { ed = plug && plug->Edited(); }
    V ms_vis(BL vis);

    V mg_winx(I &x) const { x = plug?plug->getX():0; }
    V mg_winy(I &y) const { y = plug?plug->getY():0; }
    V ms_winx(I x) { if(plug) plug->setX(x); }
    V ms_winy(I y) { if(plug) plug->setY(y); }

    V mg_chnsin(I &c) const { c = plug?plug->getNumInputs():0; }
    V mg_chnsout(I &c) const { c = plug?plug->getNumOutputs():0; }
    V mg_params(I &p) const { p = plug?plug->GetNumParams():0; }
    V mg_programs(I &p) const { p = plug?plug->NumPrograms():0; }
    V mg_plugname(const S *&s) const { s = MakeSymbol(plug?plug->GetName():""); }
    V mg_plugvendor(const S *&s) const { s = MakeSymbol(plug?plug->GetVendorName():""); }
    V mg_plugdll(const S *&s) const { s = MakeSymbol(plug?plug->GetDllName():""); }
    V mg_plugversion(I &v) const { v = plug?plug->GetVersion():0; }
    V mg_issynth(BL &s) const { s = plug && plug->IsSynth(); }

    V m_print(I ac,const A *av);

    V ms_program(I p);
    V mg_program(I &p) const { p = plug?plug->GetCurrentProgram():0; }
    
//    V m_control(const S *ctrl_name,I ctrl_value);
    V m_pitchbend(I ctrl_value);
    V m_programchange(I ctrl_value);
    V m_ctrlchange(I control,I ctrl_value);
    V m_note(I note,I vel);

    V ms_param(I pnum,F val);
    V mg_param(I pnum);
    V m_pname(I pnum);
    V m_ptext(I pnum);

private:
    V display_parameter(I param,BL showparams);

    VSTPlugin *plug;
    CString plugname;
    BL echoparam,visible;

    I blsz;
    V (VSTPlugin::*vstfun)(R **insigs,R **outsigs,L n);
    BL sigmatch;
    R **vstin,**vstout,**tmpin,**tmpout;

    V InitPlug();
    V ClearPlug();
    V InitBuf();
    V ClearBuf();
	static V Setup(t_classid);
	

    FLEXT_CALLBACK_V(m_print)

    FLEXT_CALLVAR_V(mg_plug,ms_plug)

    FLEXT_CALLVAR_B(mg_edit,ms_edit)
    FLEXT_CALLSET_B(ms_vis)
    FLEXT_ATTRGET_B(visible)

//    FLEXT_CALLBACK_2(m_control,t_symptr,int)
    FLEXT_CALLBACK_I(m_pitchbend)
    FLEXT_CALLBACK_I(m_programchange)
    FLEXT_CALLBACK_II(m_ctrlchange)

    FLEXT_CALLVAR_I(mg_program,ms_program)
    FLEXT_CALLBACK_2(ms_param,int,float)
    FLEXT_CALLBACK_I(mg_param)
    FLEXT_CALLBACK_I(m_pname)
    FLEXT_CALLBACK_I(m_ptext)

    FLEXT_CALLBACK_II(m_note)

    FLEXT_ATTRVAR_B(echoparam)
    FLEXT_CALLVAR_I(mg_winx,ms_winx)
    FLEXT_CALLVAR_I(mg_winy,ms_winy)

    FLEXT_CALLGET_I(mg_chnsin)
    FLEXT_CALLGET_I(mg_chnsout)
    FLEXT_CALLGET_I(mg_params)
    FLEXT_CALLGET_I(mg_programs)
    FLEXT_CALLGET_S(mg_plugname)
    FLEXT_CALLGET_S(mg_plugvendor)
    FLEXT_CALLGET_S(mg_plugdll)
    FLEXT_CALLGET_I(mg_plugversion)
    FLEXT_CALLGET_B(mg_issynth)
};

FLEXT_NEW_DSP_V("vst~",vst);


V vst::Setup(t_classid c)
{
#if FLEXT_OS == FLEXT_OS_WIN
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    AfxOleInit( );
#endif

    post("");
	post("vst~ %s - VST plugin object, (C)2003 Thomas Grill",VST_VERSION);
	post("based on the work of mark@junklight.com");
	post("");

	FLEXT_CADDATTR_VAR(c,"plug",mg_plug,ms_plug);
	FLEXT_CADDATTR_VAR(c,"edit",mg_edit,ms_edit);
	FLEXT_CADDATTR_VAR(c,"vis",visible,ms_vis);
	FLEXT_CADDMETHOD_(c,0,"print",m_print);

	FLEXT_CADDMETHOD_II(c,0,"note",m_note);
//	FLEXT_CADDMETHOD_2(c,0,"control",m_control,t_symptr,int);
	FLEXT_CADDMETHOD_(c,0,"pitchbend",m_pitchbend);
	FLEXT_CADDMETHOD_II(c,0,"ctrlchange",m_ctrlchange);

	FLEXT_CADDMETHOD_(c,0,"programchange",m_programchange);
	FLEXT_CADDATTR_VAR(c,"program",mg_program,ms_program);

	FLEXT_CADDMETHOD_2(c,0,"param",ms_param,int,float);
	FLEXT_CADDMETHOD_(c,0,"getparam",mg_param);
	FLEXT_CADDMETHOD_I(c,0,"getpname",m_pname);
	FLEXT_CADDMETHOD_I(c,0,"getptext",m_ptext);

	FLEXT_CADDATTR_VAR1(c,"echo",echoparam);
	FLEXT_CADDATTR_VAR(c,"x",mg_winx,ms_winx);
	FLEXT_CADDATTR_VAR(c,"y",mg_winy,ms_winy);

	FLEXT_CADDATTR_GET(c,"ins",mg_chnsin);
	FLEXT_CADDATTR_GET(c,"outs",mg_chnsout);
	FLEXT_CADDATTR_GET(c,"params",mg_params);
	FLEXT_CADDATTR_GET(c,"programs",mg_programs);
	FLEXT_CADDATTR_GET(c,"name",mg_plugname);
	FLEXT_CADDATTR_GET(c,"vendor",mg_plugvendor);
	FLEXT_CADDATTR_GET(c,"dll",mg_plugdll);
	FLEXT_CADDATTR_GET(c,"version",mg_plugversion);
	FLEXT_CADDATTR_GET(c,"synth",mg_issynth);
}


vst::vst(I argc,const A *argv):
    plug(NULL),visible(false),
    blsz(0),
    vstfun(NULL),vstin(NULL),vstout(NULL),tmpin(NULL),tmpout(NULL),
    echoparam(false)
{
    if(argc >= 2 && CanbeInt(argv[0]) && CanbeInt(argv[1])) {
	    AddInSignal(GetAInt(argv[0]));
        AddOutSignal(GetAInt(argv[1]));     

        if(!ms_plug(argc-2,argv+2)) InitProblem();
    }
    else {
        post("%s - syntax: vst~ inputs outputs [plug]",thisName());
        InitProblem();
    }
}

vst::~vst()
{
    ClearPlug();
}

V vst::ClearPlug()
{
    if(plug) {
        ClearBuf();
        delete plug; plug = NULL;
    }
}

V vst::InitPlug()
{
    FLEXT_ASSERT(plug);

    vstfun = plug->replace()?plug->processReplacing:plug->process;
    sigmatch = plug->getNumInputs() == CntInSig() && plug->getNumOutputs() == CntOutSig();

    InitBuf();
}

V vst::ClearBuf()
{
    if(!plug) return;

    if(vstin) {
        for(I i = 0; i < plug->getNumInputs(); ++i) delete[] vstin[i];
        delete[] vstin; vstin = NULL;
        delete[] tmpin; tmpin = NULL;
    }
    if(vstout) {
        for(I i = 0; i < plug->getNumOutputs(); ++i) delete[] vstout[i];
        delete[] vstout; vstout = NULL;
        delete[] tmpout; tmpout = NULL;
    }
}

V vst::InitBuf()
{
    FLEXT_ASSERT(!vstin && !tmpin && !vstout && !tmpout);

    I i;

    vstin = new R *[plug->getNumInputs()];
    tmpin = new R *[plug->getNumInputs()];
    for(i = 0; i < plug->getNumInputs(); ++i) vstin[i] = new R[Blocksize()];
    
    vstout = new R *[plug->getNumOutputs()];
    tmpout = new R *[plug->getNumOutputs()];
    for(i = 0; i < plug->getNumOutputs(); ++i) vstout[i] = new R[Blocksize()];
}

static const C *findFilePath(const C *path,const C *dllname)
{
	CFileFind finder;
	_chdir( path );

	if(finder.FindFile( dllname ))
		return path;
	else {
		finder.FindFile();
		while(finder.FindNextFile()) {
			if(finder.IsDirectory()) {
				if(!finder.IsDots()) {
					CString *npath = new CString( finder.GetFilePath()); 
					const C *ret = findFilePath( *npath , dllname );
					if(ret) {
						CString *retstr = new CString(ret);
						return *retstr;
					}
				}
			}
		}
	}
	return NULL;
}


BL vst::ms_plug(I argc,const A *argv)
{
    ClearPlug();

    plugname.Empty();
	C buf[255];	
	for(I i = 0; i < argc; i++) {
		if(i > 0) plugname += ' ';
		GetAString(argv[i],buf,sizeof buf);
		plugname += buf;
	}
    plugname.MakeLower();
    if(!plugname.GetLength()) return false;

    plug = new VSTPlugin;
    
    // now try to load plugin

    // to help deal with spaces we assume ALL of the args make 
	// up the filename 
	bool lf = false;

	// try loading the dll from the raw filename 
	if (plug->Instance(plugname) == VSTINSTANCE_NO_ERROR) {
		//post( "it loaded fine ");
		lf = true;
	}

    if(!lf) { // try finding it on the PD path
	    C *name,dir[1024];
	    I fd = open_via_path("",plugname,".dll",dir,&name,sizeof(dir)-1,0);
	    if(fd > 0) close(fd);
	    else name = NULL;

	    // if dir is current working directory... name points to dir
	    if(dir == name) strcpy(dir,".");
        
        CString dllname(dir);
        dllname += "\\";
        dllname += name;

	    lf = plug->Instance(dllname) == VSTINSTANCE_NO_ERROR;
    }

    if(!lf) { // try finding it on the VST path
		C *vst_path = getenv ("VST_PATH");

		CString dllname(plugname);
		if(dllname.Find(".dll") == -1) dllname += ".dll";			

		if(vst_path) {
            char* tok_path = new C[strlen( vst_path)+1];
            strcpy( tok_path , vst_path);
			char *tok = strtok( tok_path , ";" );
			while( tok != NULL ) {
				CString abpath( tok );
				if( abpath.Right( 1 ) != _T("\\") ) abpath += "\\";

				const char * realpath = findFilePath( abpath , dllname );				
				//post( "findFilePath( %s , %s ) = %s\n" , abpath , dllname , realpath );
				if ( realpath != NULL ) {
				    CString rpath( realpath );
					rpath += _T("\\") + plugname;
					post( "trying %s " , rpath );
					if(plug->Instance( rpath ) == VSTINSTANCE_NO_ERROR ) {
//						post("%s - plugin '%s' loaded ",thisName(),plug->GetName());
						lf = true;
						break;
					}
				}

				tok = strtok( NULL , ";" );
                if(!tok) post("%s - couldn't find plugin",thisName());
			}

            delete[] tok_path;
		}
	}

    if(!lf) { // failed - don't make any ins or outs
		post("%s - unable to load plugin '%s'",thisName(),plugname);
		ClearPlug();
	}

    // re-init dsp stuff 
    InitPlug();

    return lf;
}

V vst::m_dsp(I n,t_signalvec const *,t_signalvec const *)
{
    if(plug) {
        plug->Init(Samplerate(),(F)Blocksize());
        FLEXT_ASSERT(vstfun);

        if(blsz != Blocksize()) {
            blsz = Blocksize();
            ClearBuf();
            InitBuf();
        }
    }
}

V vst::m_signal(I n,R *const *insigs,R *const *outsigs)
{
    if(plug) {
        if(sigmatch)
            (plug->*vstfun)(const_cast<R **>(insigs),const_cast<R **>(outsigs),n);
        else {
            R **inv,**outv;

            if(plug->getNumInputs() <= CntInSig()) 
                inv = const_cast<R **>(insigs);
            else { // more plug inputs than inlets
                I i;
                for(i = 0; i < CntInSig(); ++i) tmpin[i] = const_cast<R *>(insigs[i]);

                // set dangling inputs to zero
                // according to mode... (e.g. set zero)
                for(; i < plug->getNumInputs(); ++i) ZeroSamples(tmpin[i] = vstin[i],n);

                inv = tmpin;
            }

            const BL more = plug->getNumOutputs() <= CntOutSig();
            if(more) // more outlets than plug outputs 
                outv = const_cast<R **>(outsigs);
            else {
                I i;
                for(i = 0; i < CntOutSig(); ++i) tmpout[i] = outsigs[i];
                for(; i < plug->getNumOutputs(); ++i) tmpout[i] = vstout[i];

                outv = tmpout;
            }

            (plug->*vstfun)(inv,outv,n);

            if(more) {
                // according to mode set dangling output vectors
            }
        }
    }
    else  
        flext_dsp::m_signal(n,insigs,outsigs);
}


#if 0

V vst::m_control(const S *ctrl_name,I ctrl_value)     
{
    if(!plug) return;

    I parm_num = 0;
    
    if (!*GetString(ctrl_name) || !strlen(GetString(ctrl_name))) {
		error ("plugin~: control messages must have a name and a value");
		return;
    }
    //parm_num = vst_tilde_get_parm_number (x, ctrl_name->s_name);
    //if (parm_num) 
	//{
		//vst_tilde_set_control_input_by_index (x, parm_num - 1, ctrl_value);
    //}
    //else 
	//{
		//vst_tilde_set_control_input_by_name (x, ctrl_name->s_name, ctrl_value);
    //}
}

#endif

V vst::m_pitchbend(I ctrl_value)     
{
	if(plug) plug->AddPitchBend(ctrl_value );    
}

V vst::m_programchange(I ctrl_value)     
{
	if(plug) plug->AddProgramChange(ctrl_value );    
}

V vst::ms_program(I p)
{
	if(plug && p >= 0) plug->SetCurrentProgram(p);
}

V vst::m_ctrlchange(I control,I ctrl_value)     
{
	if(plug) plug->AddControlChange(control,ctrl_value );    
}


 /**
 *	display the parameters names and values and some other bits and pieces that 
 *	may be of use
 */

V vst::m_print(I ac,const A *av) 
{
    if(!plug) return;

    int i;
	bool params = false;
	bool header = true;
	bool programs = false;
	bool parameters = true;
	int specific = -1;
    if( ac > 0 ) {
		for( i = 0 ; i < ac ; i++) {
			if(IsString(av[i])) {
				const C *buf = GetString(av[i]);	
				if ( strcmp( buf , "-params" ) == 0 ) {
					params = true;
				}
				else if ( strcmp( buf , "-noheader" ) == 0 ) {
					header = false;
				}
				else if ( strcmp( buf , "-programs" ) == 0 ) {
					programs = true;
					parameters = false;
				}
				else if ( strcmp( buf , "-parameters" ) == 0 ) {				
					parameters = false;
				}
				else if ( strcmp( buf , "-help" ) == 0 ) {
					post("print options:");
					post("-help \t\tprint this");
					post("-programs \tshow the programs");
					post("-parameters \tshow the parameters");
					post("-params \tshow the parameter display values");
					post("-noheader \tdo not display the header");
					return;
				}
			}
			else if(CanbeInt(av[i])) {
				int p = GetAInt(av[i]);
				if (( p > 0 ) && ( p <=  plug->GetNumParams())) {
					specific = p - 1;
				}
			}
		}
	 }

	 if ( header ) {
		post("VST~ plugin: %s " , plug->GetName() );
		post("made by: %s " , plug->GetVendorName() );
		post("parameterss %d\naudio: %d in(s)/%d out(s) \nLoaded from library \"%s\".\n",
			plug->GetNumParams(),
			CntInSig(),
			CntOutSig(),
			plug->GetDllName());

		post("Flags");
		if ( plug->_pEffect->flags & effFlagsHasEditor ) {
			post("Has editor");
		}
		if ( plug->_pEffect->flags & effFlagsCanReplacing ) {
			post("Can do replacing");
		}
	 }

	 if ( parameters ) {
		if ( specific == -1) {
			for (i = 0; i < plug->GetNumParams(); i++) 
				display_parameter( i , params );	
		}
		else
			display_parameter( specific , params);	
	 }

	 if( programs ) {
		for( int j = 0; j < plug->GetNumCategories() ; j++ ) {
			for( i = 0 ; i < plug->GetNumParams() ; i++ ) {
				char buf[64];
				plug->GetProgramName( j , i , buf );
				post("Program %d: %s ", i , buf );
			}
		}
	 }
}


//!	display an editor 
V vst::ms_edit(BL on)
{
#if FLEXT_OS == FLEXT_OS_WIN
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

    if(plug) plug->edit(on);
}

V vst::ms_vis(BL vis)
{
#if FLEXT_OS == FLEXT_OS_WIN
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

   if(plug) plug->visible(vis);
}


V vst::display_parameter(I param,BL showparams)
{
	int j = param;
	/* the Steinberg(tm) way... */
	char name[109];
	char display[164];
	float val;

//	if(j == 0) post ("Control input/output(s):");

    memset (name, 0, sizeof(name));
	memset( display, 0 ,sizeof(display));
	plug->GetParamName( j , name );

	if(*name) {
		if (showparams) {
			plug->DescribeValue( j , display );		
			val = plug->GetParamValue( j );
			post ("parameter[#%d], \"%s\" value=%f (%s) ", j, name,  val,display);			
		}
		else {
			val = plug->GetParamValue( j );
			post ("parameter[#%d], \"%s\" value=%f ", j, name,  val);			
		}
	}
}


// set the value of a parameter
V vst::ms_param(I pnum,F val)     
{
    if(!plug || pnum < 0 || pnum >= plug->GetNumParams()) return;

	F xval = plug->GetParamValue( pnum );
//    if(xval <= 1.0f) // What's that????
    if(true)
    { 
		plug->SetParameter( pnum, val );
		if(echoparam) display_parameter(pnum , true );
	}	
    else
        FLEXT_ASSERT(false);
}

V vst::mg_param(I pnum)
{
    if(!plug || pnum < 0 || pnum >= plug->GetNumParams()) return;

    A at[2];
    SetInt(at[0],pnum);
    SetFloat(at[1],plug->GetParamValue(pnum));
	ToOutAnything(GetOutAttr(),MakeSymbol("param"),2,at);
}

V vst::m_pname(I pnum)
{
    if(!plug || pnum < 0 || pnum >= plug->GetNumParams()) return;

    C name[109]; /* the Steinberg(tm) way... */

    memset(name,0,sizeof(name));
	plug->GetParamName(pnum,name);

    A at[2];
    SetInt(at[0],pnum);
    SetString(at[1],name);
	ToOutAnything(GetOutAttr(),MakeSymbol("pname"),2,at);
}

V vst::m_ptext(I pnum)
{
    if(!plug || pnum < 0 || pnum >= plug->GetNumParams()) return;

	C display[164];  /* the Steinberg(tm) way... */

	memset(display,0,sizeof(display));
	plug->DescribeValue(pnum,display);

    A at[2];
    SetInt(at[0],pnum);
    SetString(at[1],display);
	ToOutAnything(GetOutAttr(),MakeSymbol("ptext"),2,at);
}

V vst::m_note(I note,I velocity)
{
    if(!plug) return;

	if(velocity > 0)
		plug->AddNoteOn(note,velocity);
	else
		plug->AddNoteOff(note);
}
