/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __VSTHOST_H
#define __VSTHOST_H

#include <flext.h>
#include <string>
#include <map>
#include <math.h>

#include "AEffectx.h"
#include "AEffEditor.hpp"


#if FLEXT_OS == FLEXT_OS_WIN
#include <windows.h>
typedef HWND WHandle;
typedef HMODULE MHandle;
#elif FLEXT_OS == FLEXT_OS_MAC
#include <CoreServices/CoreServices.h>
typedef Handle WHandle;
typedef void *MHandle;
#else
#error Platform not supported!
#endif


#define MIDI_MAX_EVENTS	64

class Responder
{
public:
    virtual void Respond(const t_symbol *sym,int argc = 0,const t_atom *argv = NULL) = 0;
};

class VSTPlugin:
    public flext
{
public:

    static void Setup();

	VSTPlugin(Responder *resp);
	~VSTPlugin();

	bool Instance(const char *plug,const char *subplug = NULL);
	void Free();
	void DspInit(float samplerate,int blocksize);

    //////////////////////////////////////////////////////////////////////////////

public:
    bool Is() const { return effect != NULL; }

    long GetVersion() const { return effect?effect->version:0; }

    bool IsSynth() const { return HasFlags(effFlagsIsSynth); }
    bool IsReplacing() const { return HasFlags(effFlagsCanReplacing); }
    bool HasEditor() const { return HasFlags(effFlagsHasEditor); }

	const char *GetName() const { return productname; }
	const char *GetVendorName() const { return vendorname; }
	const char *GetDllName() const { return dllname.c_str(); }

    int GetNumInputs() const { return effect?effect->numInputs:0; }
    int GetNumOutputs() const { return effect?effect->numOutputs:0; }

    void ListPlugs(const t_symbol *sym) const;

private:
	char productname[300];
	char vendorname[300];
    std::string dllname;	// Contains dll name

    //////////////////////////////////////////////////////////////////////////////

public:
    int GetNumParams() const { return effect?effect->numParams:0; }
	void GetParamName(int numparam,char *name) const;
	void GetParamValue(int numparam,char *parval) const;
	float GetParamValue(int numparam) const;

    // scan plugin names (can take a _long_ time!!)
    void ScanParams(int i = -1);
    // get number of scanned parameters
    int ScannedParams() const { return paramnamecnt; }
    // get index of named (scanned) parameter... -1 if not found
    int GetParamIx(const char *p) const;

    bool SetParamFloat(int parameter, float value);
    bool SetParamInt(int parameter, int value) { return SetParamFloat(parameter,value/65535.0f); }

    void SetCurrentProgram(int prg) { Dispatch(effSetProgram,0,prg); }
    int GetCurrentProgram() const { return Dispatch(effGetProgram); }
	int GetNumPrograms() const { return effect->numPrograms; }

    int GetNumCategories() const { return Dispatch(effGetNumProgramCategories); }
	bool GetProgramName(int cat,int p,char* buf) const;

private:
    struct NameCmp:
        std::less<std::string>
    {
        bool operator()(const std::string &a,const std::string &b) const { return a.compare(b) < 0; }
    };

    typedef std::map<std::string,int,NameCmp> NameMap;
    int paramnamecnt;
    NameMap paramnames;

    //////////////////////////////////////////////////////////////////////////////

public:
    void SetPos(int x,int y,bool upd = true);
    void SetX(int x,bool upd = true) { SetPos(x,posy,upd); }
    void SetY(int y,bool upd = true) { SetPos(posx,y,upd); }
    int GetX() const { return posx; }
    int GetY() const { return posy; }
    void SetCaption(bool b);
    bool GetCaption() const { return caption; }
    void SetTitle(const char *t);
    const char *GetTitle() const { return title.c_str(); }


	void Edit(bool open);

	void StartEditing(WHandle h );
    void StopEditing();
    bool IsEdited() const { return hwnd != NULL; }
    WHandle EditorHandle() const { return hwnd; }

    void GetEditorRect(ERect &er) const { ERect *r; Dispatch(effEditGetRect,0,0,&r); er = *r; }
    void EditorIdle() { Dispatch(effEditIdle); }

    void Visible(bool vis);
    bool IsVisible() const;

    void Paint(ERect &r) const { Dispatch(effEditDraw,0,0,&r); }

private:
    int posx,posy; // Window position
    bool caption; // Window border
    std::string title; // Window title

    //////////////////////////////////////////////////////////////////////////////

public:
    enum {
        MIDI_NOTEON = 144,
        MIDI_NOTEOFF = 128,
        MIDI_POLYAFTERTOUCH = 160,
        MIDI_CONTROLCHANGE = 176,
        MIDI_PROGRAMCHANGE = 192,
        MIDI_AFTERTOUCH = 208,
        MIDI_PITCHBEND = 224
    };
    
    bool AddMIDI(unsigned char data0,unsigned char data1 = 0,unsigned char data2 = 0);

    static int range(int value,int mn = 0,int mx = 127) { return value < mn?mn:(value > mx?mx:value); }

	bool AddNoteOn(unsigned char note,unsigned char speed,unsigned char midichannel = 0)
    {
        return AddMIDI((char)MIDI_NOTEON|midichannel,note,speed);
    }

    bool AddNoteOff(unsigned char note,unsigned char midichannel = 0)
    {
        return AddMIDI((char)MIDI_NOTEOFF|midichannel,note,0);
    }
	
	void AddControlChange(int control,int value)
    {
        AddMIDI(MIDI_CONTROLCHANGE+(midichannel&0xf),range(control),range(value));
    }

	void AddProgramChange(int value)
    {
        AddMIDI(MIDI_PROGRAMCHANGE+(midichannel&0xf),range(value),0);
    }

	void AddPitchBend(int value)
    {
	    AddMIDI(MIDI_PITCHBEND+(midichannel&0xf),((value>>7)&127),(value&127));
    }

	void AddAftertouch(int value)
    {
 	    AddMIDI((char)MIDI_AFTERTOUCH|midichannel,range(value));
    }

private:
	void SendMidi();

    //	static VstTimeInfo _timeInfo;
	VstMidiEvent midievent[MIDI_MAX_EVENTS];
	VstEvents events;
	int	eventqusz;

	char midichannel;

    //////////////////////////////////////////////////////////////////////////////

public:

    void SetPlaying(bool p) { if(playing != p) transchg = true,playing = p; }
    bool GetPlaying() const { return playing; }
    void SetLooping(bool p) { if(looping != p) transchg = true,looping = p; }
    bool GetLooping() const { return looping; }

    void SetSamplePos(double p) { if(samplepos != p) transchg = true,samplepos = p; }
    double GetSamplePos() const { return samplepos; }
    void SetTempo(double p) { if(tempo != p) transchg = true,tempo = p; }
    double GetTempo() const { return tempo; }
    void SetPPQPos(double p) { if(ppqpos != p) transchg = true,ppqpos = p; }
    double GetPPQPos() const { return ppqpos; }

    void SetTimesigNom(int p) { if(timesignom != p) transchg = true,timesignom = p; }
    int GetTimesigNom() const { return timesignom; }
    void SetTimesigDen(int p) { if(timesigden != p) transchg = true,timesigden = p; }
    int GetTimesigDen() const { return timesigden; }
    void SetBarStart(double p) { if(barstartpos != p) transchg = true,barstartpos = p; }
    double GetBarStart() const { return barstartpos; }
    void SetCycleStart(double p) { if(cyclestartpos != p) transchg = true,cyclestartpos = p; }
    double GetCycleStart() const { return cyclestartpos; }
    void SetCycleEnd(double p) { if(cycleendpos != p) transchg = true,cycleendpos = p; }
    double GetCycleEnd() const { return cycleendpos; }

    void SetSmpteOffset(int p) { if(smpteoffset != p) transchg = true,smpteoffset = p; }
    int GetSmpteOffset() const { return smpteoffset; }
    void SetSmpteRate(int p) { if(smpterate != p) transchg = true,smpterate = p; }
    int GetSmpteRate() const { return smpterate; }

private:

    bool playing,looping;
    float samplerate;
    bool transchg;

    double samplepos,tempo;
    double ppqpos;

    int timesignom,timesigden;
    double barstartpos;
    double cyclestartpos,cycleendpos;
    int smpteoffset,smpterate;

    //////////////////////////////////////////////////////////////////////////////

public:
	void processReplacing(float **inputs,float **outputs,long sampleframes )
    {
        FLEXT_ASSERT(effect);
    	effect->processReplacing(effect,inputs,outputs,sampleframes);
        if(playing) updatepos(sampleframes);
    }

	void process(float **inputs,float **outputs,long sampleframes )
    {
        FLEXT_ASSERT(effect);
    	effect->process(effect,inputs,outputs,sampleframes);
        if(playing) updatepos(sampleframes);
    }

private:
    void updatepos(long frames);

    //////////////////////////////////////////////////////////////////////////////

private:
    Responder *responder;

    bool NewPlugin(const char *plugname);
    void FreePlugin();
    bool InstPlugin(long plugid = 0);

    static long uniqueid;
    static std::string dllloading;

    inline long GetFlags() const { return effect?effect->flags:0; } 
    inline bool HasFlags(long msk) const { return effect && (effect->flags&msk); } 


    // the handle to the shared library
	MHandle hdll;
    // the handle to the plugin editor window
    WHandle hwnd;
    // the VST plugin instance
	AEffect *effect;

    typedef AEffect *(VSTCALLBACK *PVSTMAIN)(audioMasterCallback audioMaster);
    PVSTMAIN pluginmain;
    audioMasterCallback audiomaster;
    
    long Dispatch(long opCode,long index = 0,long value = 0,void *ptr = NULL,float opt = 0) const
	{
        FLEXT_ASSERT(effect);
        return effect->dispatcher(effect,opCode,index,value,ptr,opt);
	}

	static long VSTCALLBACK Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

    static const t_symbol *sym_event,*sym_evmidi,*sym_evaudio,*sym_evvideo,*sym_evparam,*sym_evtrigger,*sym_evsysex,*sym_ev_;
    static const t_symbol *sym_midi[8];

    void ProcessEvent(const VstEvent &ev);
};

#endif
