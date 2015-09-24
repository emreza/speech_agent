#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "stdafx.h";
#include "vad.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <atomic>


using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;
static char keyword_path[128] = "kws.file";
std::atomic<bool> finish (false);

int recognize_from_microphone() {
	ad_rec_t *ad;
	int16 adbuf[2048];
	uint8 utt_started, in_speech;
	int32 k;
	char const *hyp;
	int32 score,prob;
	float conf;  

	if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
		(int) cmd_ln_float32_r(config, "-samprate"))) == NULL)
		perror("Failed to open audio device\n");
	if (ad_start_rec(ad) < 0)
		perror("Failed to start recording\n");
	if (ps_start_utt(ps) < 0)
		perror("Failed to start utterance\n");
	utt_started = FALSE;
	printf("READY....\n");
	char start[256] = "start";
	char end[256] = "end";

	while (!finish) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);
		if (in_speech && !utt_started) {
			utt_started = TRUE;
			listenCallback(start);
			printf("voice start ...\n");
		}
		if (!in_speech && utt_started) {
			utt_started = FALSE;
			listenCallback(end);
			printf("voice end\n");
		}
		Sleep(100);
	}
	ad_close(ad);
	return -1;
}

extern void _cdecl stop(){
	finish = true;
}

extern int _cdecl init(const char* options, CallbackType lCallback) {
	listenCallback = lCallback;
	finish = false;

	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", "model\\en-us\\en-us",
		//  "-lm", MODELDIR "/en-us/en-us.lm.dmp",
		//"-jsgf", "grammer.gram",
		"-dict", "model\\en-us\\cmudict-en-us2.dict",
		 "-kws", "kws.file",
		NULL);

	if (config == NULL)
		return 1;
	ps = ps_init(config);
	if (ps == NULL)
		return 1;

	int out = recognize_from_microphone( );

	ps_free(ps);
	cmd_ln_free_r(config);
	return out;
}

