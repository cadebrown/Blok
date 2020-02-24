


/* Engine.cc - implementation of the Audio::Engine class */

#include <Blok/Audio.hh>


namespace Blok::Audio {


// the buffer-filling callback from port audio
static int pa_fill_cb(
            const void* inBuf,  // input buffer 
            void* outBuf,       // output buffer
            unsigned long N,    // number of frames
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void *_data         // user data 
            ) {
    
    // capture the engine
    Engine* engine = (Engine*)_data;

    sample* out = (sample*)outBuf;

    // just 0 it out
    for (int i = 0; i < N; ++i) {
        out[i] = {0, 0};
    }

    List<int> eraseIdx;

    // enter a critical section
    bool couldLock = engine->L_stream.try_lock();
    //engine->L_stream.lock();

    // now, sum all the sounds
    for (int bpi = 0; bpi < engine->curBufPlays.size(); ++bpi) {
        auto& bufplay = engine->curBufPlays[bpi];
        unsigned int bsize = bufplay->buf->size();
        for (int i = 0; i < N && (bufplay->loop || bufplay->pos + i < bsize); ++i) {
            out[i] += bufplay->buf->get((bufplay->pos + i) % bsize) * 0.6f;
        }

        bufplay->pos += N;

        // check if we've passed the end
        if (bufplay->pos >= bufplay->buf->size()) {
            if (bufplay->loop) {
                while (bufplay->pos >= bufplay->buf->size()) bufplay->pos -= bufplay->buf->size();
            } else {
                eraseIdx.push_back(bpi);
            }
        }
    }

    // now, erase those we just finished ( do it in reverse order so no shifting is required)
    for (int i = eraseIdx.size() - 1; i >= 0; i--) {
        delete engine->curBufPlays[eraseIdx[i]];
        engine->curBufPlays.erase(engine->curBufPlays.begin() + eraseIdx[i]);
    }

    //engine->L_stream.unlock();
    if (couldLock) engine->L_stream.unlock();

    // continue processing, i.e. do not stop the straem
    return paContinue;
}
    


// construct a new engine
Engine::Engine() {

    hz = DEFAULT_HZ;
    bufsize = DEFAULT_BUFSIZE;

    L_stream.lock();

    out_param.device = Pa_GetDefaultOutputDevice();
    if (out_param.device == paNoDevice) {
        blok_error("PortAudio: No default output device!");
        return;
    }

    // always stereo floats, interleaved
    out_param.channelCount = 2;
    out_param.sampleFormat = paFloat32;

    // high latency, so no underruns are created
    // TODO: expirement with other settings
    out_param.suggestedLatency = Pa_GetDeviceInfo(out_param.device)->defaultHighOutputLatency;

    // do nothing host specific
    out_param.hostApiSpecificStreamInfo = NULL;

    PaError err;

    /* Open an audio I/O stream. */
    err = Pa_OpenStream(
        &paStream, // stream
        NULL, // input (none)
        &out_param, // output (default device, the above settings)
        (double)hz, // sample rate, use the default
        (unsigned long)bufsize, // buffer size, use the default
        paClipOff, // Disable their auto-clipping
        pa_fill_cb, // the function to fill callback
        (void*)this // the user pointer that is referenced in the callback
    );

    if (err != paNoError) {
        blok_error("PortAudio: Couldn't open stream");
        return;
    }

    err = Pa_StartStream(paStream);
    if (err != paNoError) {
        blok_error("PortAudio: Couldn't start stream");
        return;
    }

    L_stream.unlock();

}


// play a given buffer, starting now
BufferPlay* Engine::play(Buffer* buf, bool loop) {
    BufferPlay* res = new BufferPlay(buf);
    res->loop = loop;
    L_stream.lock();
    curBufPlays.push_back(res);
    L_stream.unlock();
    return res;
}

}
