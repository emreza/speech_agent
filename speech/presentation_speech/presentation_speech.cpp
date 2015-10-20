// speakrate.cpp : Defines the entry point for the console application.
//

#include <sapi.h>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "stdafx.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>


using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;

/*
 * Continuous recognition from a file
 */
static void
recognize_from_file()
{
    int16 adbuf[2048];
    const char *fname;
    const char *hyp;
    int32 k;
	char str[1000];
    uint8 utt_started, in_speech;

    fname = "../../presentation_samples/italy1_reza.wav"; 
	if ((rawfd = fopen(fname, "rb")) == NULL) {
       cout << "Failed to open file '%s' for reading",
                       fname;
    } 
  
    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".wav") == 0) {
        char waveheader[44];
		try{
			 fread(waveheader, 1, 44, rawfd);
		}
		catch (int e) {
			cout << "An exception occurred. Exception Nr. " << e << '\n';
		}
    }
    
    ps_start_utt(ps);
    utt_started = FALSE;
	long time = 0;
    while ((k = fread(adbuf, sizeof(int16), 2048, rawfd)) > 0) {
		
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
        } 
        if (!in_speech && utt_started) {
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL);

            if (hyp != NULL){
				strncpy_s( str, hyp, sizeof( str ) );
				printf("%s\n", hyp);
			}
            ps_start_utt(ps);
            utt_started = FALSE;
        }
    }
    ps_end_utt(ps);
    
    fclose(rawfd);
}


int _tmain(int argc, _TCHAR* argv[]) {
	//string path ="C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/";
	config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", "C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/en-us",
			//"-lm","C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/en-us.lm.dmp",
			//"-lm","C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/cristina.lm",
			//"-jsgf", "grammar.gram",
			//"-vad_threshold","3",
			"-kws", "C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/kws.file",
			"-dict", "C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/cmudict-en-us2.dict",
			//"-beam", "1e-20", "-pbeam", "1e-20", "-lw", "2.0",
			//"-logfn","model",
			NULL);
	if (config == NULL)
		return false;
	ps = ps_init(config);
	if (ps == NULL)
		return false;
	recognize_from_file();
	ps_free(ps);
	cmd_ln_free_r(config);
	return 0;
}
