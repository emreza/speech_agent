// tracker.cpp : Defines the entry point for the console application.
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
#include <math.h>

using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;
static char keyword_path[128] = "kws.file";
// 
bool recognize_from_microphone() {
	ad_rec_t *ad;
	int16 adbuf[2048];
	uint8 utt_started, in_speech;
	int32 k;
	char const *hyp;
	int32 score;

	int data_count = 0;
	if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),16000)) == NULL)
		perror("Failed to open audio device\n");
	if (ad_start_rec(ad) < 0)
		perror("Failed to start recording\n");
	if (ps_start_utt(ps) < 0)
		perror("Failed to start utterance\n");
	utt_started = FALSE;	
	char str[256];
	char notfound[256] = "Not found!";
	bool confirm_phase = false;
	while (true) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);     
		if (in_speech && !utt_started) 
			utt_started = TRUE;
		if (!in_speech && utt_started) {
			// speech -> silence transition, time to start new utterance  
			ps_end_utt(ps);
			hyp = ps_get_hyp(ps, &score);
			if (hyp != NULL){
				strncpy_s( str, hyp, sizeof( str ) );				
				printf(str);
			}
			if (ps_start_utt(ps) < 0)
				perror("Failed to start utterance\n");
			utt_started = FALSE;
		}
		Sleep(50);
	}
	ad_close(ad);
	return false;
}
int _tmain(int argc, _TCHAR* argv[]) {
	config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", "model\\en-us\\en-us",
			//"-lm", MODELDIR "/en-us/en-us.lm.dmp",
			//"-jsgf", "grammer.gram",
			//"-vad_threshold","2",
			"-dict", "model\\en-us\\cmudict-en-us2.dict",
			"-kws", "kws.file",
			//"-mllr","model\\mllr_matrix",
			//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
			NULL);
	if (config == NULL)
		return false;
	ps = ps_init(config);
	if (ps == NULL)
		return false;
	bool out = recognize_from_microphone();
	ps_free(ps);
	cmd_ln_free_r(config);
	return out;
}