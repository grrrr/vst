/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Sepp�nen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "main.h"

#include "Editor.h"
#include "VstHost.h"

#include <stdlib.h>
#include <direct.h>
#include <io.h>

#include <string>
using namespace std;


#define VST_VERSION "0.1.0pre9"


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
    V mg_plug(AtomList &sym) const { sym(1); SetString(sym[0],plugname.c_str()); }

    V mg_editor(BL &ed) { ed = plug && plug->HasEditor(); }

    V ms_edit(BL on) { if(plug) plug->edit(on); }
    V mg_edit(BL &ed) { ed = plug && plug->IsEdited(); }
    V ms_vis(BL vis) { if(plug) plug->Visible(vis); }
    V mg_vis(BL &vis) { vis = plug && plug->IsVisible(); }

    V mg_winx(I &x) const { x = plug?plug->GetX():0; }
    V mg_winy(I &y) const { y = plug?plug->GetY():0; }
    V ms_winx(I x) { if(plug) plug->SetX(x); }
    V ms_winy(I y) { if(plug) plug->SetY(y); }

    V mg_chnsin(I &c) const { c = plug?plug->GetNumInputs():0; }
    V mg_chnsout(I &c) const { c = plug?plug->GetNumOutputs():0; }
    V mg_params(I &p) const { p = plug?plug->GetNumParams():0; }
    V mg_programs(I &p) const { p = plug?plug->GetNumPrograms():0; }
    V mg_progcats(I &p) const { p = plug?plug->GetNumCategories():0; }
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
    V m_aftertouch(I ctrl_value);
    V m_ctrlchange(I control,I ctrl_value);
    V m_note(I note,I vel);
    inline V m_noteoff(I note) { m_note(note,0); }

    V ms_param(I pnum,F val);
    V mg_param(I pnum);
    V m_pname(I pnum);
    V m_ptext(I pnum);

private:
    V display_parameter(I param,BL showparams);

    VSTPlugin *plug;
    string plugname;
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
    FLEXT_CALLGET_B(mg_editor)
    FLEXT_CALLVAR_B(mg_vis,ms_vis)

//    FLEXT_CALLBACK_2(m_control,t_symptr,int)
    FLEXT_CALLBACK_I(m_pitchbend)
    FLEXT_CALLBACK_I(m_aftertouch)
    FLEXT_CALLBACK_I(m_programchange)
    FLEXT_CALLBACK_II(m_ctrlchange)

    FLEXT_CALLVAR_I(mg_program,ms_program)
    FLEXT_CALLBACK_2(ms_param,int,float)
    FLEXT_CALLBACK_I(mg_param)
    FLEXT_CALLBACK_I(m_pname)
    FLEXT_CALLBACK_I(m_ptext)

    FLEXT_CALLBACK_II(m_note)
    FLEXT_CALLBACK_I(m_noteoff)

    FLEXT_ATTRVAR_B(echoparam)
    FLEXT_CALLVAR_I(mg_winx,ms_winx)
    FLEXT_CALLVAR_I(mg_winy,ms_winy)

    FLEXT_CALLGET_I(mg_chnsin)
    FLEXT_CALLGET_I(mg_chnsout)
    FLEXT_CALLGET_I(mg_params)
    FLEXT_CALLGET_I(mg_programs)
    FLEXT_CALLGET_I(mg_progcats)
    FLEXT_CALLGET_S(mg_plugname)
    FLEXT_CALLGET_S(mg_plugvendor)
    FLEXT_CALLGET_S(mg_plugdll)
    FLEXT_CALLGET_I(mg_plugversion)
    FLEXT_CALLGET_B(mg_issynth)
};

FLEXT_NEW_DSP_V("vst~",vst);


V vst::Setup(t_classid c)
{
    post("");
	post("vst~ %s - VST plugin object, (C)2003-04 Thomas Grill",VST_VERSION);
	post("based on the work of Jarno Sepp�nen and Mark Williamson");
	post("");

	FLEXT_CADDATTR_VAR(c,"plug",mg_plug,ms_plug);
	FLEXT_CADDATTR_VAR(c,"edit",mg_edit,ms_edit);
	FLEXT_CADDATTR_GET(c,"editor",mg_editor);
	FLEXT_CADDATTR_VAR(c,"vis",mg_vis,ms_vis);
	FLEXT_CADDMETHOD_(c,0,"print",m_print);

	FLEXT_CADDMETHOD_II(c,0,"note",m_note);
	FLEXT_CADDMETHOD_I(c,0,"noteoff",m_noteoff);
//	FLEXT_CADDMETHOD_2(c,0,"control",m_control,t_symptr,int);
	FLEXT_CADDMETHOD_(c,0,"pbend",m_pitchbend);
	FLEXT_CADDMETHOD_(c,0,"atouch",m_aftertouch);
	FLEXT_CADDMETHOD_II(c,0,"ctlchg",m_ctrlchange);

	FLEXT_CADDMETHOD_(c,0,"progchg",m_programchange);
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
	FLEXT_CADDATTR_GET(c,"progcats",mg_progcats);
	FLEXT_CADDATTR_GET(c,"name",mg_plugname);
	FLEXT_CADDATTR_GET(c,"vendor",mg_plugvendor);
	FLEXT_CADDATTR_GET(c,"dll",mg_plugdll);
	FLEXT_CADDATTR_GET(c,"version",mg_plugversion);
	FLEXT_CADDATTR_GET(c,"synth",mg_issynth);

    SetupEditor();
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

        if(argc >= 3 && !ms_plug(argc-2,argv+2)) InitProblem();
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
        ms_edit(false);
        ClearBuf();
        delete plug; plug = NULL;
    }
}

V vst::InitPlug()
{
    FLEXT_ASSERT(plug);

	vstfun = plug->IsReplacing()?VSTPlugin::processReplacing:VSTPlugin::process;
    sigmatch = plug->GetNumInputs() == CntInSig() && plug->GetNumOutputs() == CntOutSig();

    InitBuf();
}

V vst::ClearBuf()
{
    if(!plug) return;

    if(vstin) {
        for(I i = 0; i < plug->GetNumInputs(); ++i) delete[] vstin[i];
        delete[] vstin; vstin = NULL;
        delete[] tmpin; tmpin = NULL;
    }
    if(vstout) {
        for(I i = 0; i < plug->GetNumOutputs(); ++i) delete[] vstout[i];
        delete[] vstout; vstout = NULL;
        delete[] tmpout; tmpout = NULL;
    }
}

V vst::InitBuf()
{
    FLEXT_ASSERT(!vstin && !tmpin && !vstout && !tmpout);
    const int inputs = plug->GetNumInputs(),outputs = plug->GetNumOutputs();

    I i;

    vstin = new R *[inputs];
    tmpin = new R *[inputs];
    for(i = 0; i < inputs; ++i) vstin[i] = new R[Blocksize()];
    
    vstout = new R *[outputs];
    tmpout = new R *[outputs];
    for(i = 0; i < outputs; ++i) vstout[i] = new R[Blocksize()];
}

static string findFilePath(const string &path,const string &dllname)
{
	_chdir( path.c_str() );
#if FLEXT_OS == FLEXT_OS_WIN
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile(dllname.c_str(),&data);
    if(fh != INVALID_HANDLE_VALUE) {
        FindClose(fh);
        return path;
    }
#endif
/*
	CFileFind finder;
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
*/
   
    return string();
}


BL vst::ms_plug(I argc,const A *argv)
{
    ClearPlug();

    plugname.clear();
	C buf[255];	
	for(I i = 0; i < argc; i++) {
		if(i > 0) plugname += ' ';
		GetAString(argv[i],buf,sizeof buf);
        strlwr(buf);
		plugname += buf;
	}

    if(!plugname.length()) return false;

    plug = new VSTPlugin;
    
    // now try to load plugin

    // to help deal with spaces we assume ALL of the args make 
	// up the filename 
	bool lf = false;
    int loaderr = VSTINSTANCE_NO_ERROR;

	// try loading the dll from the raw filename 
	if ((loaderr = plug->Instance(plugname.c_str())) == VSTINSTANCE_NO_ERROR) {
		FLEXT_LOG("raw filename loaded fine");
		lf = true;
	}

    if(!lf) { // try finding it on the PD path
	    C *name,dir[1024];
	    I fd = open_via_path("",plugname.c_str(),".dll",dir,&name,sizeof(dir)-1,0);
	    if(fd > 0) close(fd);
	    else name = NULL;

        if(name) {
    		FLEXT_LOG("found VST dll on the PD path");
	        // if dir is current working directory... name points to dir
	        if(dir == name) strcpy(dir,".");
        
            string dllname(dir);
            dllname += "\\";
            dllname += name;

	        lf = (loaderr = plug->Instance(dllname.c_str())) == VSTINSTANCE_NO_ERROR;
        }
    }

    if(!lf) { // try finding it on the VST path
		C *vst_path = getenv ("VST_PATH");

		string dllname(plugname);
		if(dllname.find(".dll") == -1) dllname += ".dll";			

		if(vst_path) {
    		FLEXT_LOG("found VST_PATH env variable");
            char* tok_path = new C[strlen( vst_path)+1];
            strcpy( tok_path , vst_path);
			char *tok = strtok( tok_path , ";" );
			while( tok != NULL ) {
				string abpath( tok );
				if( abpath[abpath.length()-1] != '\\' ) abpath += "\\";

        		FLEXT_LOG1("trying VST_PATH %s",(const C *)abpath.c_str());

				string realpath = findFilePath( abpath , dllname );				
				//post( "findFilePath( %s , %s ) = %s\n" , abpath , dllname , realpath );
				if ( realpath.length() ) {
					realpath += plugname;
            		FLEXT_LOG1("trying %s",(const C *)realpath.c_str());
					if((loaderr = plug->Instance( realpath.c_str() )) == VSTINSTANCE_NO_ERROR ) {
                		FLEXT_LOG("plugin loaded via VST_PATH");
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
		post("%s - unable to load plugin '%s', load error %i",thisName(),plugname,loaderr);
		ClearPlug();
	}

    // re-init dsp stuff 
    if(plug) InitPlug();

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
        const int inputs = plug->GetNumInputs(),outputs = plug->GetNumOutputs();

        if(sigmatch)
            (plug->*vstfun)(const_cast<R **>(insigs),const_cast<R **>(outsigs),n);
        else {
            R **inv,**outv;

            if(inputs <= CntInSig()) 
                inv = const_cast<R **>(insigs);
            else { // more plug inputs than inlets
                I i;
                for(i = 0; i < CntInSig(); ++i) tmpin[i] = const_cast<R *>(insigs[i]);

                // set dangling inputs to zero
                // according to mode... (e.g. set zero)
                for(; i < inputs; ++i) ZeroSamples(tmpin[i] = vstin[i],n);

                inv = tmpin;
            }

            const BL more = outputs <= CntOutSig();
            if(more) // more outlets than plug outputs 
                outv = const_cast<R **>(outsigs);
            else {
                I i;
                for(i = 0; i < CntOutSig(); ++i) tmpout[i] = outsigs[i];
                for(; i < outputs; ++i) tmpout[i] = vstout[i];

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

V vst::m_aftertouch(I ctrl_value)     
{
	if(plug) plug->AddAftertouch(ctrl_value );    
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
		post("parameters %d\naudio: %d in(s)/%d out(s) \nLoaded from library \"%s\".\n",
			plug->GetNumParams(),
			CntInSig(),
			CntOutSig(),
			plug->GetDllName());

		post("Flags");
		if ( plug->HasEditor() ) post("Has editor");
		if ( plug->IsReplacing() ) post("Can do replacing");
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
//			plug->DescribeValue( j , display );		
            plug->GetParamValue(j,display);
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
		plug->SetParamFloat( pnum, val );
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
//	plug->DescribeValue(pnum,display);
	plug->GetParamValue(pnum,display);

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
