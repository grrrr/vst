/* 

vst - VST plugin object for PD 

Copyright (c) 2003 Thomas Grill (xovo@gmx.net)
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

#define VST_VERSION "0.0.0"


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



class vst:
	public flext_dsp
{
	FLEXT_HEADER_S(vst,flext_dsp,setup)

public:
	vst(I argc,const A *argv);
	~vst();

	virtual BL Init();

protected:
    virtual V m_dsp(I n,t_signalvec const *insigs,t_signalvec const *outsigs);
    virtual V m_signal(I n,R *const *insigs,R *const *outsigs);

    V (VSTPlugin::*dspfun)(R **insigs,R **outsigs,L n);

    V m_control(const S *ctrl_name,I ctrl_value);
    V m_pitchbend(I ctrl_value);
    V m_programchange(I ctrl_value);
    V m_program (I ctrl_value);
    V m_ctrlchange(I control,I ctrl_value);
    V m_print(I ac,const A *av);
    V m_edit();
    V m_showparams();
    V m_noshowparams();
    V m_param(I pnum,F val);
    V m_reset();
    inline V m_midinote(I note,I vel) { m_velocity(vel); m_note(note); }
    V m_note(I note);
    inline V m_velocity(I vel) { velocity = vel; }

private:
    V display_parameter(I param,BL showparams);

    VSTPlugin *plug;
    CString cmdln;
    I velocity;


	static V setup(t_classid);
	
    FLEXT_CALLBACK_2(m_control,t_symptr,int)
    FLEXT_CALLBACK_I(m_pitchbend)
    FLEXT_CALLBACK_I(m_programchange)
    FLEXT_CALLBACK_I(m_program)
    FLEXT_CALLBACK_II(m_ctrlchange)
    FLEXT_CALLBACK_V(m_print)
    FLEXT_CALLBACK(m_edit)
    FLEXT_CALLBACK(m_showparams)
    FLEXT_CALLBACK(m_noshowparams)
    FLEXT_CALLBACK_2(m_param,int,float)
    FLEXT_CALLBACK(m_reset)
    FLEXT_CALLBACK_II(m_midinote)
    FLEXT_CALLBACK_I(m_note)
    FLEXT_CALLBACK_I(m_velocity)
};

FLEXT_NEW_DSP_V("vst~",vst);


V vst::setup(t_classid c)
{
#if FLEXT_OS == FLEXT_OS_WIN
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    AfxOleInit( );
#endif

    post("");
	post("vst %s - VST plugin object,(C)2003 Thomas Grill",VST_VERSION);
	post("");

	FLEXT_CADDMETHOD_2(c,0,"control",m_control,t_symptr,int);
	FLEXT_CADDMETHOD_(c,0,"pitchbend",m_pitchbend);
	FLEXT_CADDMETHOD_(c,0,"programchange",m_programchange);
	FLEXT_CADDMETHOD_(c,0,"program",m_program);
	FLEXT_CADDMETHOD_II(c,0,"ctrlchange",m_ctrlchange);
	FLEXT_CADDMETHOD_(c,0,"print",m_print);
	FLEXT_CADDMETHOD_(c,0,"edit",m_edit);
	FLEXT_CADDMETHOD_(c,0,"showparams",m_showparams);
	FLEXT_CADDMETHOD_(c,0,"noshowparams",m_noshowparams);
	FLEXT_CADDMETHOD_2(c,0,"param",m_param,int,float);
	FLEXT_CADDMETHOD_(c,0,"reset",m_reset);
	FLEXT_CADDMETHOD_II(c,0,"midinote",m_midinote);
}


vst::vst(I argc,const A *argv):
    dspfun(NULL)
{
    plug = new VSTPlugin;

	C buf[255];	
	for(I i = 0; i < argc; i++) {
		if(i > 0) cmdln += ' ';
		GetAString(argv[i],buf,sizeof buf);
		cmdln += buf;
	}
    cmdln.MakeLower();

    if(!cmdln.GetLength()) InitProblem();
}

vst::~vst()
{
    if(plug) delete plug;
}


static const char * findFilePath( const char * path , const char * dllname )
{
	CFileFind finder;
	_chdir( path );
	if ( finder.FindFile( dllname ) == TRUE )
	{
		return path;
	}
	else
	{
		finder.FindFile();
		while( finder.FindNextFile() )
		{
			if ( finder.IsDirectory() )
			{
				if ( !finder.IsDots() )
				{
					CString *npath = new CString( finder.GetFilePath()); 
					const char * ret = findFilePath( *npath , dllname );
					if ( ret != NULL)
					{
						CString *retstr = new CString( ret );
						return *retstr;
					}
				}
			}
		}
	}
	return NULL;
}

BL vst::Init()
{
    // to help deal with spaces we assume ALL of the args make 
	// up the filename 
	bool lf = false;

	// try loading the dll from the raw filename 
	if (plug->Instance(cmdln) == VSTINSTANCE_NO_ERROR) {
		//post( "it loaded fine ");
		lf = true;
	}

    if(!lf) { // try finding it on the PD path
	    C *name,dir[1024];
	    I fd = open_via_path("",cmdln,".dll",dir,&name,sizeof(dir)-1,0);
	    if(fd > 0) close(fd);
	    else name = NULL;

	    // if dir is current working directory... name points to dir
	    if(dir == name) strcpy(dir,".");
        
        CString dllname(dir);
        dllname += "\\";
        dllname += name;

	    if(plug->Instance(dllname) == VSTINSTANCE_NO_ERROR) {
		    //post( "it loaded fine ");
		    lf = true;
	    }
    }

    if(!lf) { // try finding it on the VST PAth
		char* vst_path = getenv ("VST_PATH");

		CString dllname(cmdln);
		if(cmdln.Find(".dll") == -1) dllname += ".dll";			

		if ( vst_path ) {
            char* tok_path = new C[strlen( vst_path)+1];
            strcpy( tok_path , vst_path);
			char *tok = strtok( tok_path , ";" );
			 while( tok != NULL )
			 {
				 CString abpath( tok );
				 if( abpath.Right( 1 ) != _T("\\") )
				 {
					abpath += "\\";
				 }				 
				 const char * realpath = findFilePath( abpath , dllname );				
				 //post( "findFilePath( %s , %s ) = %s\n" , abpath , dllname , realpath );
				 if ( realpath != NULL )
				 {
					 CString rpath( realpath );
					rpath += _T("\\") + cmdln;
					 post( "trying %s " , rpath );
					if ( plug->Instance( rpath ) == VSTINSTANCE_NO_ERROR )
					{
						post( "  %s loaded " , plug->GetName());
						lf = TRUE;
						break;
					}
				 }
				tok = strtok( NULL , ";" );
				if ( tok == NULL )
				{
					post( "couldn't find dll");
				}
			 }

             delete[] tok_path;
		}
	}

	if ( !lf ) // failed - don't make any ins or outs
	{
		post("Unable to load %s" , cmdln);
		delete plug;
		plug = NULL;
	}

    if(lf) {
    	I inputs = plug->getNumInputs(),outputs = plug->getNumOutputs();
        
	    AddInSignal(inputs);

        AddInInt("midi note");
        AddInInt("midi velocity");

    	FLEXT_ADDMETHOD(inputs,m_note);
	    FLEXT_ADDMETHOD(inputs+1,m_velocity);

        AddOutAnything("control out");
        AddOutSignal(outputs);
    }

    return lf && flext_dsp::Init();
}


V vst::m_dsp(I n,t_signalvec const *insigs,t_signalvec const *outsigs)
{
    if(plug) {
		plug->Init(Samplerate(),Blocksize());	
        dspfun = plug->replace()?plug->processReplacing:plug->process;
    }
    else
        dspfun = NULL;
}

V vst::m_signal(I n,R *const *insigs,R *const *outsigs)
{
    if(dspfun) 
            (plug->*dspfun)(const_cast<R **>(insigs),const_cast<R **>(outsigs),n);
    else 
            flext_dsp::m_signal(n,insigs,outsigs);
}


V vst::m_control(const S *ctrl_name,I ctrl_value)     
{
    if(!plug) return;

    I parm_num = 0;
    
    if (!*GetString(ctrl_name) || !strlen(GetString(ctrl_name))) 
	{
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

V vst::m_pitchbend(I ctrl_value)     
{
	if(plug) plug->AddPitchBend(ctrl_value );    
}

V vst::m_programchange(I ctrl_value)     
{
	if(plug) plug->AddProgramChange(ctrl_value );    
}

V vst::m_program (I ctrl_value)     
{
	if(plug) plug->SetCurrentProgram(ctrl_value );    
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
    if ( ac > 0 ) {
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
					post("-params \tshow the parameter display values ");
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


/**
*	display an editor - currently not implemented 
*/

V vst::m_edit()     
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	if(plug) 
        plug->edit();    
	else
		post("No plugin to edit");
}

V vst::m_showparams()     
{
	if(plug) plug->SetShowParameters( true);    
}

V vst::m_noshowparams()
{
	if(plug) plug->SetShowParameters(false);
}



V vst::display_parameter(I param,BL showparams)
{
	int j = param;
	/* the Steinberg(tm) way... */
	char name[109];
	char display[164];
	float val;
	if (j == 0) 
	{
		post ("Control input/output(s):");
	}
	memset (name, 0, 108);
	memset( display, 0 ,163);
	plug->GetParamName( j , name );
	if ( name[0] != NULL )
	{
		if ( showparams )
		{
			plug->DescribeValue( j , display );		
			val = plug->GetParamValue( j );
			post ("parameter[#%d], \"%s\" value=%f (%s) ", j + 1, name,  val,display);			
		}
		else
		{
			val = plug->GetParamValue( j );
			post ("parameter[#%d], \"%s\" value=%f ", j + 1, name,  val);			
		}
	}
}


/**
*	set the value of a parameter
*/

V vst::m_param(I pnum,F val)     
{
    if(!plug) return;

	if ( ( pnum > 0 ) && ( pnum <= plug->GetNumParams() ))
	{		
		int i = (int) pnum - 1;
		char name[9];
		char display[64];
		float xval;
		memset (name, 0, 9);
		memset( display, 0 ,64);
		plug->GetParamName( i , name );
		if ( name[0] != NULL )
		{		
			xval = plug->GetParamValue( i );
			if ( xval <= 1.0f)
			{
				plug->SetParameter( i , val );
				if ( plug->ShowParams() )
				{
					display_parameter(i , true );
				}
			}		
		}
	}
}

V vst::m_reset()
{
}

V vst::m_note(I note)     
{
    if(!plug) return;

	if(velocity > 0)
		plug->AddNoteOn(note,velocity);
	else
		plug->AddNoteOff(note);
}
