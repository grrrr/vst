/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Sepp�nen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __VSTHOST_H
#define __VSTHOST_H

#include <flext.h>
#include "Vst\AEffectx.h"
#include "vst\aeffeditor.h"

#if FLEXT_OS == FLEXT_OS_WIN
#include <windows.h>
typedef HWND WHandle;
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


class VSTPlugin:
    public flext
{
public:

	VSTPlugin();
	~VSTPlugin();

	int Instance(const char *dllname);
//	void Create(VSTPlugin *plug);
	void Free();
	void DspInit(float samplerate,int blocksize);

    bool Is() const { return _pEffect != NULL; }

    ULONG GetVersion() const { return _pEffect?_pEffect->version:0; }

    bool IsSynth() const { return HasFlags(effFlagsIsSynth); }
    bool IsReplacing() const { return HasFlags(effFlagsCanReplacing); }
    bool HasEditor() const { return HasFlags(effFlagsHasEditor); }

	const char *GetName() const { return _sProductName; }
	const char *GetVendorName() const { return _sVendorName; }
	const char *GetDllName() const { return _sDllName.c_str(); }

    int GetNumInputs() const { return _pEffect?_pEffect->numInputs:0; }
    int GetNumOutputs() const { return _pEffect?_pEffect->numOutputs:0; }

    int GetNumParams() const { return _pEffect?_pEffect->numParams:0; }
	void GetParamName(int numparam,char *name) const;
	void GetParamValue(int numparam,char *parval) const;
	float GetParamValue(int numparam) const;

    bool SetParamFloat(int parameter, float value);
    bool SetParamInt(int parameter, int value) { return SetParamFloat(parameter,value/65535.0f); }

    void SetCurrentProgram(int prg) { Dispatch(effSetProgram,0,prg); }
    int GetCurrentProgram() const { return Dispatch(effGetProgram); }
	int GetNumPrograms() const { return _pEffect->numPrograms; }

    int GetNumCategories() const { return Dispatch(effGetNumProgramCategories); }
	bool GetProgramName( int cat, int p , char* buf) const;


	bool AddMIDI(unsigned char data0,unsigned char data1=0,unsigned char data2=0);

	bool AddNoteOn( unsigned char note,unsigned char speed,unsigned char midichannel=0);
	bool AddNoteOff( unsigned char note,unsigned char midichannel=0);
	
	void AddControlChange( int control , int value );
	void AddProgramChange( int value );
	void AddPitchBend( int value );
	void AddAftertouch( int value );

	void Edit(bool open);

	void StartEditing(WHandle h );
    void StopEditing();
    bool IsEdited() const { return hwnd != NULL; }
    WHandle EditorHandle() const { return hwnd; }

    void GetEditorRect(ERect &er) const { ERect *r; Dispatch(effEditGetRect,0,0,&r); er = *r; }
    void EditorIdle() { Dispatch(effEditIdle); }

    void Visible(bool vis);
    bool IsVisible() const;

//    void Paint(ERect &r) const { Dispatch(effEditDraw,0,0,&r); }

	void processReplacing( float **inputs, float **outputs, long sampleframes );
	void process( float **inputs, float **outputs, long sampleframes );

	long Dispatch(long opCode, long index = 0, long value = 0, void *ptr = NULL, float opt = 0) const
	{
        return Is()?_pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt):0;
	}

	static long VSTCALLBACK Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

    void SetPos(int x,int y,bool upd = true);
    void SetX(int x,bool upd = true) { SetPos(x,posy,upd); }
    void SetY(int y,bool upd = true) { SetPos(posx,y,upd); }
    int GetX() const { return posx; }
    int GetY() const { return posy; }

protected:

#if FLEXT_OS == FLEXT_OS_WIN
	HMODULE h_dll;
#else
#error
#endif

    WHandle hwnd;

	AEffect *_pEffect;

    inline long GetFlags() const { return _pEffect?_pEffect->flags:0; } 
    inline bool HasFlags(long msk) const { return _pEffect && (_pEffect->flags&msk); } 

	char _sProductName[64];
	char _sVendorName[64];
    std::string _sDllName;	// Contains dll name

/*
	float *inputs[MAX_INOUTS];
	float *outputs[MAX_INOUTS];
*/

//	static VstTimeInfo _timeInfo;
	VstMidiEvent midievent[MAX_EVENTS];
	VstEvents events;
	int	queue_size;

//    float sample_rate;

	void SendMidi();
	char _midichannel;

    int posx,posy;
};

#endif
