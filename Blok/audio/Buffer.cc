/* Buffer.cc - implementation of an audio buffer */

#include <Blok/Blok.hh>
#include <Blok/Audio.hh>

// for loading wav files
#define DR_WAV_IMPLEMENTATION
#include "./dr_wav.h"

// for loading vorbis files
#include "./stb_vorbis.c"

namespace Blok::Audio {

// cache of loaded constant buffers from files
Map<String, Buffer*> Buffer::cache;

// read a wav file into a buffer, as a const (i.e. do not modify or free it)
// it will be stored in 'cache'
Buffer* Buffer::loadConst(const String& fname) {

    // see if it has been loaded
    if (cache.find(fname) != cache.end()) {
        return cache[fname];
    }

    for (auto path : paths) {

        String cname = path + "/" + fname;

        // read from drWAV
        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 totalPCMFrameCount = 0;
        float* pSampleData = NULL;
        
        const char* fext = strrchr(cname.c_str(), '.');

        if (fext == NULL) {
            blok_trace("Failed to read audio file '%s'; no valid extension", cname.c_str());
            continue;
        } else if (strcmp(fext, ".wav") == 0) {
            // read a wave file
            pSampleData = drwav_open_file_and_read_pcm_frames_f32(cname.c_str(), &channels, &sampleRate, &totalPCMFrameCount);
        } else if (strcmp(fext, ".ogg") == 0) {

            // read a OGG/vorbis file
            int error;
            stb_vorbis* v = stb_vorbis_open_filename(cname.c_str(), &error, NULL);
            if (!v) {
                blok_trace("Failed to read audio file '%s'; stb_vorbis returned NULL", cname.c_str());
                continue;
            }

            channels = -1;
            // current position in pSampleData
            int psn = 0;

            for (;;) {
                int n;
                float *left, *right;
                float **outputs;
                int _chn;
                n = stb_vorbis_get_frame_float(v, &_chn, &outputs);
                if (n == 0) break;

                channels = _chn;
                totalPCMFrameCount += n;

                pSampleData = (float*)realloc(pSampleData, sizeof(*pSampleData) * channels * totalPCMFrameCount);

                // copy them into main sample data
                for (int i = 0; i < n; ++i) {
                    for (int j = 0; j < channels; ++j) {
                        pSampleData[(psn + i) * channels + j] = outputs[j][i];
                    }
                }

                // move forward
                psn += n;
            }
            if (channels < 0) channels = 2;

            stb_vorbis_close(v);

        } else {
            blok_trace("Failed to read audio file '%s'; unrecognized extension '%s'", cname.c_str(), fext);
            continue;
        }


        
        if (pSampleData == NULL) {
            // Error opening and reading WAV file.
            blok_trace("Failed to read audio file '%s'; no data", cname.c_str());
            continue;
        }

        List<vec2> smp;

        if (channels == 1) {
            printf("1chan\n");
            // duplicate mono samples
            for (int i = 0; i < totalPCMFrameCount; ++i) {
                smp.push_back(vec2(pSampleData[i], pSampleData[i]));
            }
        } else if (channels == 2) {
            // store stereo samples
            for (int i = 0; i < totalPCMFrameCount; ++i) {
                smp.push_back(vec2(pSampleData[2*i], pSampleData[2*i+1]));
            }
        } else {
            blok_trace("Failed to read audio file '%s'; had > 2 channels (%i)", cname.c_str(), (int)channels);
            continue;
        }

        Buffer* newbuf = new Buffer(smp, sampleRate);

        // free the data
        free(pSampleData);

        blok_info("Loaded audio file '%s'", fname.c_str());

        // store in cache
        cache[fname] = newbuf;
        return newbuf;
    }

    blok_error("Failed to read audio file '%s'", fname.c_str());


    return NULL;


    //return Clip(pSampleData, totalPCMFrameCount, sampleRate, channels);
}

}


