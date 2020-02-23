/* Audio.hh - main header file dealing with audio utilities 
 *
 * See the files in `audio/` for more information on some of the implmentations
 * 
 */

#pragma once

#ifndef BLOK_AUDIO_HH__
#define BLOK_AUDIO_HH__

// general Blok library
#include <Blok/Blok.hh>


#include <mutex>

namespace Blok::Audio {


    // the speed of sound (in m/s)
    const float SOUND_SPEED = 343.0f;

    // the default sampling rate
    const int DEFAULT_HZ = 44100;

    // y= 10^(db/20)
    inline float db_to_coef(float db) {
        return powf(10.0f, db / 20.0f);
    }

    // y=10^(db/20) -> log_10(y) = db/20, db = 20 * log_10(y)
    inline float coef_to_db(float coef) {
        return 20.0f * logf(coef) / logf(10.0f);
    }

    // a single sample is a stereo (LR) sample
    using sample = vec2;

    // Buffer - a class describing a buffer of audio
    struct Buffer {

        // cache of loaded constant buffers from files
        static Map<String, Buffer*> cache;

        // read a wav file into a buffer, as a const (i.e. do not modify or free it)
        // it will be stored in 'cache'
        static Buffer* loadConst(const String& fname);


        // the actual list of samples in the buffer
        List<sample> data;

        // by default, 44100hz
        int hz;

        // construct a buffer object from samples, and a given rate
        Buffer(const List<sample>& data={}, int hz=DEFAULT_HZ) {
            this->data = data;
            this->hz = hz;
        }

        void set(int idx, sample smp) {
            data[idx] = smp;
        }

        sample get(int idx) {
            return data[idx];
        }

        int size() {
            return data.size();
        }

        // subscript
        vec2& operator[](int idx) {
            return data[idx];
        }

    };


    // BufferPlay - a datastructure representing a single instance of playing a sound buffer in an
    //   engine
    struct BufferPlay {
        public:

        // the buffer that is being played
        Buffer* buf;

        // current position in the buffer that it is being played at
        int pos;

        // the current volume of the piece
        float volume;

        // whether or not to loop the playing
        bool loop;

        BufferPlay(Buffer* buf) {
            this->buf = buf;
            pos = 0;
            volume = 1.0;
            loop = true;
        }

    };


    // Engine - an entire audio rendering engine framework
    class Engine {

        public:

        // sample rate & buffer sizes
        int hz, bufsize;

        // a lock to control access to all stream-related variables
        std::mutex L_stream;


        // the PortAudio stream
        // NOTE: Do not modify without first locking 'L_stream'!
        PaStream* paStream;

        // the output parameters
        // NOTE: Do not modify without first locking 'L_stream'!
        PaStreamParameters out_param;

        // sounds being played currently
        // NOTE: Do not modify without first locking 'L_stream'!
        List<BufferPlay*> curBufPlays;


        // stream position
        unsigned long pos;


        // construct a new engine
        Engine();


        // play a given buffer, starting now
        BufferPlay* play(Buffer* buf, bool loop=false);

    };




}

#endif
