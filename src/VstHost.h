/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef _VSTPLUGIN_HOST
#define _VSTPLUGIN_HOST

#include "Vst\AEffectx.h"
#include <flext.h>

#if FLEXT_OS == FLEXT_OS_WIN
#include <windows.h>
#else
#error Platform not supported!
#endif

#define MAX_EVENTS		64
#define MAX_INOUTS		8

#define VSTINSTANCE_ERR_NO_VALID_FILE -1
#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
#define VSTINSTANCE_ERR_REJECTED -3
#define VSTINSTANCE_NO_ERROR 0

#define MIDI_NOTEON 144
#define MIDI_NOTEOFF 128
#define MIDI_POLYAFTERTOUCH 160
#define MIDI_CONTROLCHANGE 176
#define MIDI_PROGRAMCHANGE 192
#define MIDI_AFTERTOUCH 208
#define MIDI_PITCHBEND 224

typedef AEffect* (*PVSTMAIN)(audioMasterCallback audioMaster);
typedef HWND (*POPWIN)(void);
typedef HWND (*GETWIN)(void);

class VSTPlugin:
    public flext
{
public:
	VSTPlugin();
	~VSTPlugin();

    bool Is() const { return _pEffect != NULL; }

    ULONG GetVersion() const { return _pEffect?_pEffect->version:0; }

    bool IsSynth() const { return HasFlags(effFlagsIsSynth); }
    bool IsReplacing() const { return HasFlags(effFlagsCanReplacing); }
    bool HasEditor() const { return HasFlags(effFlagsHasEditor); }

	int GetNumCategories();
	bool GetProgramName( int cat, int p , char* buf);
	void AddControlChange( int control , int value );
	void AddProgramChange( int value );
	void AddPitchBend( int value );
	void AddAftertouch( int value );
	bool ShowParams();
	void SetShowParameters( bool s);

	void SetEditWindow( HWND h );
    void StopEditing() { hwnd = NULL; }
    bool IsEdited() const { return hwnd != NULL; }

	void OnEditorClose();
    HWND EditorHandle() const { return hwnd; }

	RECT GetEditorRect();
	void EditorIdle();

	void edit(bool open);
    void Visible(bool vis);
    bool IsVisible() const;

	void Free();
	int Instance( const char *dllname);
//	void Create(VSTPlugin *plug);
	void Init( float samplerate , float blocksize );

    int GetNumInputs() const { return _pEffect?_pEffect->numInputs:0; }
    int GetNumOutputs() const { return _pEffect?_pEffect->numOutputs:0; }

	virtual char* GetName(void) { return _sProductName; }
	char* GetVendorName(void) { return _sVendorName; }
	char* GetDllName(void) { return _sDllName; }

	int GetNumParams() { return _pEffect->numParams; }

	void GetParamName(int numparam,char* name);
//    float GetParameter(long parameter) { return _pEffect->getParameter(_pEffect, parameter); }
	void GetParamValue(int numparam,char* parval);
	float GetParamValue(int numparam);

    bool SetParamFloat(int parameter, float value);
    bool SetParamInt(int parameter, int value) { return SetParamFloat(parameter,value/65535.0f); }

    void SetCurrentProgram(int prg);
	int GetCurrentProgram();
	int GetNumPrograms() const { return _pEffect->numPrograms; }

	bool AddMIDI(unsigned char data0,unsigned char data1=0,unsigned char data2=0);
	void SendMidi();

	void processReplacing( float **inputs, float **outputs, long sampleframes );
	void process( float **inputs, float **outputs, long sampleframes );

	long Dispatch(long opCode, long index, long value, void *ptr, float opt)
	{
		return _pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt);
	}

	static long Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

	bool AddNoteOn( unsigned char note,unsigned char speed,unsigned char midichannel=0);
	bool AddNoteOff( unsigned char note,unsigned char midichannel=0);
	

	char _midichannel;
//	bool instantiated;
//	int _instance;		// Remove when Changing the FileFormat.

    void SetPos(int x,int y,bool upd = true);
    void SetX(int x,bool upd = true) { SetPos(x,posy,upd); }
    void SetY(int y,bool upd = true) { SetPos(posx,y,upd); }
    int GetX() const { return posx; }
    int GetY() const { return posy; }

protected:

	HMODULE h_dll;
//	HMODULE h_winddll;
	HWND hwnd;

	AEffect *_pEffect;

    inline long GetFlags() const { return _pEffect?_pEffect->flags:0; } 
    inline bool HasFlags(long msk) const { return _pEffect && (_pEffect->flags&msk); } 

//	bool DescribeValue(int parameter,char* psTxt);

	char _sProductName[64];
	char _sVendorName[64];
	char *_sDllName;	// Contains dll name
//	bool _isSynth,_editor;

	float * inputs[MAX_INOUTS];
	float * outputs[MAX_INOUTS];
//	float junk[256];

	static VstTimeInfo _timeInfo;
	VstMidiEvent midievent[MAX_EVENTS];
	VstEvents events;
	int	queue_size;
//	bool overwrite;

    float sample_rate;

private:
    int posx,posy;
//	bool show_params;
//	static float sample_rate;
};

#endif // _VSTPLUGIN_HOST
