#include "portaudio.h"
#include <cstdio>

class Beep {
  
  private:
    typedef struct {
      float left_phase;
      float right_phase;
    } paData;
    
    PaError err;
    PaStreamParameters outputParameters;
    PaStream *stream;
    paData data;
  
  static int PaStreamCallback(const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData )
  {

    paData *data = (paData*) userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
 
    for(i = 0; i < framesPerBuffer; i++) {

      *out++ = data->left_phase;  // left 
      *out++ = data->right_phase;  // right 

      data->left_phase += 0.008f;
      if(data->left_phase >= 1.0f) data->left_phase -= 2.0f;
      data->right_phase += 0.008f;
      if(data->right_phase >= 1.0f) data->right_phase -= 2.0f;
  }
  
  return 0;
  }

  static void StreamFinished(void* userData){
    paData *data = (paData *) userData;
  }

  public:
    ~Beep() {
      err = Pa_CloseStream( stream );
      if( err != paNoError ) {
        Error();
      }
      Pa_Terminate();
    }

    void Error() {
      Pa_Terminate();
      fprintf( stderr, "An error occurred while using the portaudio stream\n" );
      fprintf( stderr, "Error number: %d\n", err );
      fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    }
    
    void Play() {
      err = Pa_StartStream( stream );
      if(err != paNoError) {
        Error();
        return;
      }
    }

    void Stop() {
      err = Pa_StopStream(stream);
      if(err != paNoError){
        Error();
        return;
      }
    }
  
    void Init() {
      err = Pa_Initialize();
      if(err != paNoError) {
        Error();
        return;
      }
      
      outputParameters.device = Pa_GetDefaultOutputDevice();
      if (outputParameters.device == paNoDevice) {
          fprintf(stderr,"Error: No default output device.\n");
          Error();
          return;
      }

      outputParameters.channelCount = 2;       /* stereo output */
      outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
      outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
      outputParameters.hostApiSpecificStreamInfo = NULL;
     
      const int SAMPLE_RATE = 44100;
      const int FRAMES_PER_BUFFER = 2048;

      data.left_phase = data.right_phase = 0.0;

      err = Pa_OpenStream(
            &stream,
            NULL, /* no input */
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paClipOff,      /* we won't output out of range samples so don't bother clipping them */
            PaStreamCallback,
            &data);
      
      if(err != paNoError) {
        Error();
        return;
      }

      err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
      if(err != paNoError){
        Error();
        return;
      }
  }


};

