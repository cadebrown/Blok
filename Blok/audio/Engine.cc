


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

    // now, sum all the sounds
    for (int bpi = 0; bpi < engine->curBufPlays.size(); ++bpi) {
        auto& bufplay = engine->curBufPlays[bpi];
        unsigned int bsize = bufplay.buf->size();
        for (int i = 0; i < N && (bufplay.loop || bufplay.pos + i < bsize); ++i) {
            out[i] += bufplay.buf->get((bufplay.pos + i) % bsize) * 0.6f;
        }

        bufplay.pos += N;

        // check if we've passed the end
        if (bufplay.pos >= bufplay.buf->size()) {
            if (bufplay.loop) {
                while (bufplay.pos >= bufplay.buf->size()) bufplay.pos -= bufplay.buf->size();
            } else {
                eraseIdx.push_back(bpi);
            }
        }
    }

    // now, erase those we just finished ( do it in reverse order so no shifting is required)
    for (int i = eraseIdx.size() - 1; i >= 0; i--) {
        engine->curBufPlays.erase(engine->curBufPlays.begin() + eraseIdx[i]);
    }

    // continue processing, i.e. do not stop the straem
    return paContinue;
}
    


// construct a new engine
Engine::Engine() {

    hz = DEFAULT_HZ;
    bufsize = 512;

    out_param.device = Pa_GetDefaultOutputDevice();
    if (out_param.device == paNoDevice) {
        blok_error("PortAudio: No default output device!");
        return;
    }

    out_param.channelCount = 2;
    out_param.sampleFormat = paFloat32;
    out_param.suggestedLatency = Pa_GetDeviceInfo(out_param.device)->defaultHighOutputLatency;
    out_param.hostApiSpecificStreamInfo = NULL;

    PaError err;

    /* Open an audio I/O stream. */
    err = Pa_OpenStream(&paStream, NULL, &out_param, (int)hz, bufsize, paClipOff, pa_fill_cb, (void*)this);

    if (err != paNoError) {
        blok_error("PortAudio: Couldn't open stream");
        return;
    }

    err = Pa_StartStream(paStream);
    if (err != paNoError) {
        blok_error("PortAudio: Couldn't start stream");
        return;
    }
}

}
