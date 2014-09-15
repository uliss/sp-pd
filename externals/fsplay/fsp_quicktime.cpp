/* 
fsplay~ - file and stream player

Copyright (c)2004-2008 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.

$LastChangedRevision: 3599 $
$LastChangedDate: 2008-04-13 15:48:35 +0400 (вс, 13 апр 2008) $
$LastChangedBy: thomas $
*/

#include "fsplay.h"

#if FLEXT_OS == FLEXT_OS_WIN
#   include <windows.h>
#   ifndef pascal
#       define pascal
#   endif
//#   define TARGET_API_MAC_CARBON 0
#   include <QTML.h>
#   include <Movies.h>
#   include <GXMath.h>
#else
#   include <stdio.h>
#   include <QuickTime/QuickTime.h>
#endif

#ifndef FloatToFixed
#define FloatToFixed(a)     ((Fixed)((double)(a) * ((Fixed) 0x00010000L)))
#endif

#ifdef _WIN32
static HANDLE qtmutex;
#endif

class QTThread
{
public:
    QTThread(Movie m = NULL):
        movie(m)
    {
#ifdef VIBREZ
#ifdef _WIN32
        WaitForSingleObject(qtmutex,INFINITE);
#else

        EnterMoviesOnThread(0); 
        if(movie)
            AttachMovieToCurrentThread(movie);
#endif
#endif
    }

    ~QTThread() 
    { 
#ifdef VIBREZ
#ifdef _WIN32
        ReleaseMutex(qtmutex);
#else
        if(movie)
            DetachMovieFromCurrentThread(movie);
        ExitMoviesOnThread(); 
#endif
#endif
    }

protected:
    Movie movie;
};

void CnvFlnm(std::string &dst,const char *src);

class fsp_quicktime
    : public fspformat
{
public:
    static bool Setup()
    {
        OSErr err;
    #ifdef _WIN32
        err = InitializeQTML(0L);
        if(err) return false;
    #ifdef VIBREZ
        qtmutex = CreateMutex(NULL,FALSE,"Local\\Quicktime");
        if(!qtmutex) return false;
    #endif
    #endif
        err = EnterMovies();
        if(err) return false;

        Add(New);
        return true;
    }

    static fspformat *New(const std::string &filename)
    {
        fsp_quicktime *ret = new fsp_quicktime(filename);
        if(ret->movie)
            return ret;
        else {
            delete ret;
            return NULL;
        }
    }

    virtual ~fsp_quicktime()
    {
        QTThread qt;
        if(extractionSessionRef) MovieAudioExtractionEnd(extractionSessionRef);
        if(movie) DisposeMovie(movie);
    }

    virtual int Channels() const 
    {
        return channels;
    }

    virtual float Samplerate() const 
    {
        return samplerate;
    }

    virtual cnt_t Frames() const 
    {
        return frames;
    }

    virtual cnt_t Read(t_sample *rbuf,cnt_t frames)
    {
        FLEXT_ASSERT(extractionSessionRef != nil);

        AudioBufferList bufferlist;
        bufferlist.mNumberBuffers = 1;
        bufferlist.mBuffers->mData = rbuf;
        bufferlist.mBuffers->mDataByteSize = int(frames)*channels*sizeof(*rbuf);
        bufferlist.mBuffers->mNumberChannels = channels;

        UInt32 numFrames = int(frames);
        UInt32 flags = 0;
        OSStatus err = MovieAudioExtractionFillBuffer(extractionSessionRef, &numFrames, &bufferlist, &flags);
        if(err) {
              post("Error %i filling audio buffer",err);
            return -1;
        }
        if((flags&kQTMovieAudioExtractionComplete) && !numFrames) {
            // extraction complete!
            return -1;
        }
        return numFrames;
    }

    virtual bool Seek(double pos)
    {
        FLEXT_ASSERT(movie);
        FLEXT_ASSERT(extractionSessionRef != nil);

        QTThread qt(movie);

        TimeRecord timeRec;
        timeRec.scale	= GetMovieTimeScale(movie);
        timeRec.base	= NULL;
        unsigned long long fpos = (long long)(pos*timeRec.scale);
        timeRec.value.hi = int(fpos>>32);
        timeRec.value.lo = int(fpos&((1LL<<32)-1));

        // Set the extraction current time.  The duration will 
        // be determined by how much is pulled.
        OSStatus err = MovieAudioExtractionSetProperty(extractionSessionRef,
                    kQTPropertyClass_MovieAudioExtraction_Movie,
                    kQTMovieAudioExtractionMoviePropertyID_CurrentTime,
                    sizeof(TimeRecord), &timeRec);

        return err == 0;
    }

#if TARGET_OS_MAC != 0 && !defined(VIBREZ)
    virtual void ThreadBegin() 
    {
        EnterMoviesOnThread(0);
        AttachMovieToCurrentThread(movie);
    }

    virtual void ThreadEnd() 
    {
        ExitMoviesOnThread();
    }
#endif

protected:
    fsp_quicktime(const std::string &fname)
        : movie(NULL)
        , extractionSessionRef(nil)
    {
        //  preprerollproc = NewMoviePrePrerollCompleteUPP(PrePrerollCompleteProc);

        short       actualResId = DoTheRightThing;
        OSErr       result = 0;

        // Using Data Reference calls now
        OSType      myDataRefType;
        Handle      myDataRef = NULL;

        if(fname.find("://") == std::string::npos) {
            std::string name;
            CnvFlnm(name,fname.c_str());

            CFStringRef inPath;

            // Convert movie path to CFString
            inPath = CFStringCreateWithCString(NULL, name.c_str(), CFStringGetSystemEncoding());
            if (!inPath) { printf("Could not get CFString \n"); goto bail; }
            
            // create the data reference
            result = QTNewDataReferenceFromFullPathCFString(inPath, (QTPathStyle)kQTNativeDefaultPathStyle, 0, &myDataRef, &myDataRefType);
            if (result) { printf("Could not get DataRef %d\n", result); goto bail; }
        }
        else {
            // allocate a new handle
            myDataRef = NewHandleClear(fname.size()+1);
            if(!myDataRef) { printf("Couldn't allocate data handle\n"); goto bail; }

            // copy the URL into the handle
            strcpy(*myDataRef,fname.c_str());

            myDataRefType = URLDataHandlerSubType;
        }

        {
            QTThread qt;

            // get the Movie
            // \TODO use asynchronous loading for streams etc.
            result = NewMovieFromDataRef(&movie, newMovieActive /*| newMovieAsyncOK*/,&actualResId, myDataRef, myDataRefType);
            if (result) { printf("Could not get Movie from DataRef %d\n", result); goto bail; }
        }

        // dispose the data reference handle - we no longer need it
        DisposeHandle(myDataRef);

        if(movie) {
            QTThread qt(movie);

            SetMovieDrawingCompleteProc(movie, movieDrawingCallWhenChanged, NULL,0);

            // disable all non-audio tracks
            long tracks = GetMovieTrackCount(movie);
            for(long t = 0; t < tracks; ++t) {
                Track track = GetMovieIndTrack(movie,t);
                Media media = GetTrackMedia(track);
                OSType type;
                GetMediaHandlerDescription(media,&type,0,NULL);
                SetTrackUsage(track,type == SoundMediaType?trackUsageInMovie:0);
                SetTrackEnabled(track,type == SoundMediaType?1:0);

                // enable high quality and disable track if non-audio
                SetMediaPlayHints(media, hintsHighQuality|hintsInactive, hintsHighQuality|(type == SoundMediaType?0:hintsInactive));
            }

            // set to off-screen output to prevent movie banners
//            SetMoviePlayHints(movie, hintsHighQuality|hintsOffscreen, hintsHighQuality|hintsOffscreen);

            OSStatus err;
            err = MovieAudioExtractionBegin(movie, 0, &extractionSessionRef);
            if(err != noErr) goto bail;

            AudioChannelLayout *layout  = NULL;
            UInt32             size     = 0;

            // First get the size of the extraction output layout
            err = MovieAudioExtractionGetPropertyInfo(extractionSessionRef,
                        kQTPropertyClass_MovieAudioExtraction_Audio,
                        kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                        NULL, &size, NULL);

            if(err == noErr) {
                // Allocate memory for the channel layout
                layout = (AudioChannelLayout *) calloc(1, size);
                if (layout == nil) 
                {
                    err = memFullErr;
                    goto bail;
                }

                // Get the layout for the current extraction configuration.
                // This will have already been expanded into channel descriptions.
                err = MovieAudioExtractionGetProperty(extractionSessionRef,
                        kQTPropertyClass_MovieAudioExtraction_Audio,
                        kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                        size, layout, nil);
            }

            // disable mixing of audio channels
            Boolean allChannelsDiscrete = true;
            err = MovieAudioExtractionSetProperty(extractionSessionRef,
                kQTPropertyClass_MovieAudioExtraction_Movie,
                kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
                    sizeof (Boolean), &allChannelsDiscrete);

            AudioStreamBasicDescription asbd;

            // Get the default audio extraction ASBD
            err = MovieAudioExtractionGetProperty(extractionSessionRef,
                    kQTPropertyClass_MovieAudioExtraction_Audio,
                    kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                    sizeof (asbd), &asbd, nil);

            // Convert the ASBD to return interleaved instead of non-interleaved Float32.
            if(!asbd.mSampleRate) 
                asbd.mSampleRate = 44100.; // \note can this be zero? 
            asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
            asbd.mBitsPerChannel = sizeof(float) * 8;
            asbd.mBytesPerFrame = sizeof(float) * asbd.mChannelsPerFrame;
            asbd.mBytesPerPacket = asbd.mBytesPerFrame;

            // Set the new audio extraction ASBD
            err = MovieAudioExtractionSetProperty(extractionSessionRef,
                        kQTPropertyClass_MovieAudioExtraction_Audio,
                        kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                        sizeof (asbd), &asbd);

            free(layout);

            channels = asbd.mChannelsPerFrame;
            samplerate = float(asbd.mSampleRate);

            if(channels) {
                printf("LOADED\n");

                // preroll movie
                TimeValue timeNow = GetMovieTime(movie, NULL);
                Fixed rate = GetMoviePreferredRate(movie);
                PrePrerollMovie(movie, timeNow, rate, NULL, NULL);

                printf("PRE-PREROLLED\n");

                PrerollMovie(movie, timeNow, rate);

                printf("PREROLLED\n");


                TimeScale scale = GetMovieTimeScale(movie);
                TimeValue dur = GetMovieDuration(movie);
                double duration = double(dur)/double(scale);

                frames = cnt_t(duration*asbd.mSampleRate);
            }
            else
                frames = 0;
        }

bail:
        ;
    }

    MovieAudioExtractionRef extractionSessionRef;
    Movie movie;
    int channels;
    cnt_t frames;
    float samplerate;

#ifdef _WIN32
    static HANDLE mutex;
#endif
};

bool loaded_quicktime = fspformat::Add(fsp_quicktime::Setup);

#ifdef _WIN32
HANDLE fsp_quicktime::mutex = NULL;
#endif
