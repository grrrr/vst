/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "vsthost.h"

static const int VST_VERSION = 100;
static const char *vendor = "grrrr.org";
static const char *product = "vst~";

// Host callback dispatcher
long VSTPlugin::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
#ifdef FLEXT_LOGGING
  	post("VST -> host: Eff = 0x%.8X, Opcode = %d, Index = %d, Value = %d, PTR = %.8X, OPT = %.3f\n",(int)effect, opcode,index,value,(int)ptr,opt);
#endif

    VSTPlugin *th = effect?(VSTPlugin *)effect->user:NULL;

	switch (opcode) {
    case audioMasterAutomate: // 0
#ifdef FLEXT_LOGGING
        post("Automate index=%li value=%li opt=%f",index,value,opt);
#endif
		// index, value given
		//! \todo set effect parameter
        return 0;

    case audioMasterVersion: // 1
        // support VST 2.3
        return 2300;

    case audioMasterCurrentId: // 2
        // set to subplugin id (default 0)
        return uniqueid;

	case audioMasterIdle: // 3
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;

	case audioMasterPinConnected: // 4
		//! \todo set connection state correctly (if possible..)
		// index=pin, value=0..input, else..output
#ifdef FLEXT_LOGGING
        post("Pin connected pin=%li conn=%li",index,value);
#endif
		return 0; // 0 means connected

	case audioMasterWantMidi: // 6
#ifdef FLEXT_LOGGING
        post("Want MIDI = %li",value);
#endif
		return 0; // VST header says: "currently ignored"

    case audioMasterGetTime: { // 7
        if(!th) return 0;

        static VstTimeInfo time;
		memset(&time,0,sizeof(time));
		time.samplePos = th->samplepos;
        time.sampleRate = th->samplerate;
        // flags
        time.flags = kVstTempoValid|kVstBarsValid|kVstCyclePosValid;

        time.tempo = th->tempo;
        time.barStartPos = th->barstartpos;
        time.cycleStartPos = th->cyclestartpos;
        time.cycleEndPos = th->cycleendpos;

        if(th->timesigden) {
            time.timeSigNumerator = th->timesignom;
            time.timeSigDenominator = th->timesigden;
            time.flags |= kVstTimeSigValid;
        }

        if(value&kVstNanosValid) {
            time.nanoSeconds = flext::GetOSTime()*1.e9;
            time.flags |= kVstNanosValid;
        }

//        time.ppqPos = 0;

/*
        // SMPTE data
        time.smpteOffset = 0;
        time.smpteFrameRate = 0;
        //
        time.samplesToNextClock = 0;
*/
		return (long)&time;
    }
    case audioMasterProcessEvents: { // 8
        // VST event data from plugin
        VstEvent *ev = static_cast<VstEvent *>(ptr);
        if(ev->type == kVstMidiType) {
            VstMidiEvent *mev = static_cast<VstMidiEvent *>(ptr);
#ifdef FLEXT_DEBUG
            if(mev->byteSize == 24)
                post("MIDI event delta=%li len=%li offs=%li detune=%i offvel=%i",mev->deltaFrames,mev->noteLength,mev->noteOffset,(int)mev->detune,(int)mev->noteOffVelocity);
            else
                // has incorrect size
                post("MIDI event");
#endif
        }
        else {
#ifdef FLEXT_DEBUG
            post("VST event type=%li",ev->type);
#endif
        }
		return 1;
    }

    case audioMasterSetTime: { // 9
        VstTimeInfo *tminfo = static_cast<VstTimeInfo *>(ptr);
#ifdef FLEXT_DEBUG
        post("TimeInfo pos=%lf rate=%lf filter=%li",tminfo->samplePos,tminfo->sampleRate,value);
#endif
        return 0; // not supported
    }

	case audioMasterTempoAt: // 10
		return 0; // not supported

	case audioMasterGetNumAutomatableParameters: // 11
		return 0; // not supported

    case audioMasterSizeWindow: // 15
        return 0;

    case audioMasterGetSampleRate: // 16
        return 0; // not supported
    case audioMasterGetBlockSize: // 17
        return 0; // not supported

    case audioMasterGetCurrentProcessLevel: // 23
        // return thread state
        return flext::GetThreadId() == flext::GetSysThreadId()?2:1;

	case audioMasterGetVendorString: // 32
		strcpy((char*)ptr,vendor);
        return 0;

	case audioMasterGetProductString: // 33
		strcpy((char *)ptr,product);
		return 0;

	case audioMasterGetVendorVersion: // 34
		return VST_VERSION;

	case audioMasterCanDo: // 37
#ifdef FLEXT_LOGGING
    	post("\taudioMasterCanDo PTR = %s",ptr);
#endif
        if(!strcmp((char *)ptr,"sendVstEvents"))
            return 1;
        else if(!strcmp((char *)ptr,"sendVstMidiEvent"))
            return 1;
        else if(!strcmp((char *)ptr,"sendVstTimeInfo"))
            return 1; // NOT YET
        else if(!strcmp((char *)ptr,"receiveVstEvents")) 
            return 1;
        else if(!strcmp((char *)ptr,"receiveVstMidiEvent"))
            return 1;
        else if(!strcmp((char *)ptr,"receiveVstTimeInfo"))
            return 1; // NOT YET
        else if(!strcmp((char *)ptr,"reportConnectionChanges"))
            return 0; // \TODO PD has hard times supporting that...
        else if(!strcmp((char *)ptr,"acceptIOChanges"))
            return 0; // \TODO what does this means exactly?
        else if(!strcmp((char *)ptr,"supplyIdle"))
            return 1;
        else if(!strcmp((char *)ptr,"sizeWindow"))
            return 1;
        else if(!strcmp((char *)ptr,"supportShell"))
            return 0; // deprecated - new one is shellCategory
        else if(!strcmp((char *)ptr,"offline"))
            return 0; // not supported
        else if(!strcmp((char *)ptr,"asyncProcessing"))
            return 0; // not supported
        else if(!strcmp((char *)ptr,"shellCategory"))
            return 1; // supported!
        else if(!strcmp((char *)ptr,"editFile"))
            return 0; // not supported
        else if(!strcmp((char *)ptr,"openFileSelector"))
            return 0; // not supported
        else if(!strcmp((char *)ptr,"closeFileSelector"))
            return 0; // not supported
        else if(!strcmp((char *)ptr,"startStopProcess"))
            return 0; // not supported
#ifdef FLEXT_DEBUG
        else
    	    post("Unknown audioMasterCanDo PTR = %s",ptr);
#endif

		return 0; // not supported

	case audioMasterGetLanguage: // 38
		return kVstLangEnglish;

	case audioMasterGetDirectory: // 41
        // return full path of plugin
		return 0; // not supported

    case audioMasterUpdateDisplay: // 42
#ifdef FLEXT_LOGGING
        post("UPDATE DISPLAY");
#endif
        return 0;

    default:
#ifdef FLEXT_DEBUG
        post("Unknown opcode %li",opcode);
#endif
        return 0;
    }
}
