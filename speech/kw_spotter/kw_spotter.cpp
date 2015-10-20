// kw_spotter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "kw_spotter.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <atomic>

ps_decoder_t *ps;
cmd_ln_t *config;
FILE *rawfd;
std::atomic<bool> finished (false);

/*
 * Continuous recognition from mic
 */
int
recognize_from_mic()
{
	ad_rec_t *ad;
    int16 adbuf[2048];
    const char *fname;
	const char* seg;
    int32 k;
	char str[1000]="";
    uint8 utt_started, in_speech;
	
    if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),16000)) == NULL)
		perror("Failed to open audio device\n");
	if (ad_start_rec(ad) < 0)
		perror("Failed to start recording\n");
    
    ps_start_utt(ps);
    utt_started = FALSE;
	ps_seg_t *psegt;
    while (!finished) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
        } 
        if (!in_speech && utt_started) {
            ps_end_utt(ps);
			psegt = ps_seg_iter(ps, NULL);
			while (psegt!=NULL){
				seg = ps_seg_word(psegt);
				strncpy_s( str, seg, strlen(seg));
				listenCallback(str);
				printf("%s\n", seg);
				int prob = ps_seg_prob(psegt,NULL,NULL,NULL);
				printf("%d\n", prob);
				psegt = ps_seg_next(psegt);
			}
            ps_start_utt(ps);
            utt_started = FALSE;
        }
		Sleep(100);
    }
	
    ps_end_utt(ps);
    fclose(rawfd);
	return 0;
}


/*
 * Continuous recognition from a file
 */
int
recognize_from_file()
{
    int16 adbuf[2048];
    const char *fname;
    const char *hyp;
    int32 k;
	char str[1000]="";
    uint8 utt_started, in_speech;
	
    fname = "C:/Users/Reza/Documents/GitHub/speech_agent/presentation_samples/italy1_reza.wav"; 
    rawfd = fopen(fname, "rb");
    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".wav") == 0) {
        char waveheader[44];
		fread(waveheader, 1, 44, rawfd);
    }
    
    ps_start_utt(ps);
    utt_started = FALSE;
	
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
				strncpy_s( str, hyp, strlen(hyp));
				printf("%s\n", hyp);
				listenCallback(str);
			}
            ps_start_utt(ps);
            utt_started = FALSE;
        }
    }
	
    ps_end_utt(ps);
    fclose(rawfd);
	return 0;
	
}

void stop(){
	finished = true;
}

int run(CallbackType callback, char* kpath) {
	//string path ="C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/";
	listenCallback = callback;
	config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", "C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/en-us",
			//"-lm","C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/en-us.lm.dmp",
			//"-lm","C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/cristina.lm",
			//"-jsgf", "grammar.gram",
			"-vad_threshold","3",
			"-kws", kpath,
			"-dict", "C:/Users/Reza/Documents/GitHub/speech_agent/speech/Release/model/en-us/cmudict-en-us2.dict",
			//"-beam", "1e-20", "-pbeam", "1e-20", "-lw", "2.0",
			//"-logfn","model",
			NULL);
	if (config == NULL)
		return -1;
	ps = ps_init(config);
	if (ps == NULL)
		return -1;
	recognize_from_mic();
	//recognize_from_file();
	ps_free(ps);
	cmd_ln_free_r(config);
	return 0;
}

