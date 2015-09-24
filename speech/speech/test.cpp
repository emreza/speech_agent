// speech.cpp : Defines the exported functions for the DLL application.
//

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "stdafx.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <atomic>


using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;


int recognize_from_microphone() {
	ad_rec_t *ad;
	int16 adbuf[2048];
	uint8 utt_started, in_speech;
	int32 k;
	char const *hyp;
	int* score;
	if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
		(int) cmd_ln_float32_r(config, "-samprate"))) == NULL)
		perror("Failed to open audio device\n");
	if (ad_start_rec(ad) < 0)
		perror("Failed to start recording\n");
	if (ps_start_utt(ps) < 0)
		perror("Failed to start utterance\n");
	utt_started = FALSE;
	printf("READY....\n");
	char str[256];
	char notfound[256] = "Not found!";

	while (true) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);
		if (in_speech && !utt_started) {
			utt_started = TRUE;
			printf("Listening...\n");
		}
		if (!in_speech && utt_started) {
			// speech -> silence transition, time to start new utterance  
			ps_end_utt(ps);
			hyp = ps_get_hyp(ps, score );
			if (hyp != NULL){
				printf("%s, score %f\n",hyp,&score);	
			}
			else
				printf(notfound);
			if (ps_start_utt(ps) < 0)
				perror("Failed to start utterance\n");
			utt_started = FALSE;
			printf("READY....\n");
		}
		Sleep(100);
	}
	ad_close(ad);
	return -1;
}



int main(const char* options) {
	
	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", "model\\en-us\\en-us",
		//  "-lm", MODELDIR "/en-us/en-us.lm.dmp",
		"-jsgf", "grammer.gram",
		"-dict", "model\\en-us\\cmudict-en-us2.dict",
		//  "-keyphrase","can you repeat that",
		// "-kws", "words.file",
		// "-kws_threshold", "1e-100",
		//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
		NULL);
	if (config == NULL)
		return 1;
	ps = ps_init(config);
	if (ps == NULL)
		return 1;

	int out = recognize_from_microphone();

	ps_free(ps);
	cmd_ln_free_r(config);
	return out;
}


