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
#include <math.h>
#include <ctime>

using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;
int num_words;

int count_words(char str[256]){
	int num = 0;
	for(int i = 0; str[i] != '\0'; i++)
		num++;		
	return num/5;
}


/*
 * Continuous recognition from a file
 */
static void
recognize_from_file()
{
	int n_words; 
    int16 adbuf[2048];
    const char *fname;
    const char *hyp;
    int32 k;
	char str[256];
    uint8 utt_started, in_speech;
  //  int32 print_times = cmd_ln_boolean_r(config, "-time");

    fname = "lazlo.wav";
	rawfd = fopen(fname, "rb");
  
    if (strlen(fname) > 4 && strcmp(fname + strlen(fname) - 4, ".wav") == 0) {
        char waveheader[44];
	fread(waveheader, 1, 44, rawfd);
    }
    
    ps_start_utt(ps);
    utt_started = FALSE;
	long time = 0;
    while ((k = fread(adbuf, sizeof(int16), 2048, rawfd)) > 0) {
		
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
		if (in_speech)
			time = time + k;
        if (in_speech && !utt_started) {
            utt_started = TRUE;
        } 
        if (!in_speech && utt_started) {
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL);

            if (hyp != NULL){
				strncpy_s( str, hyp, sizeof( str ) );
				printf("%s\n", hyp);
				n_words = count_words(str);
				double timei = time*0.0000625;
				printf(", number of words:%d, time elapsed:%fs, speaking rate:%f",
						n_words,timei,n_words/timei*60);
				time = 0;
          //  if (print_times)
        //	print_word_times();
			}
            ps_start_utt(ps);
            utt_started = FALSE;
        }
    }
    ps_end_utt(ps);
    if (utt_started) {
        hyp = ps_get_hyp(ps, NULL);
        if (hyp != NULL)
    	    printf("%s\n", hyp);
    //    if (print_times) {
    //    print_word_times();
	//}
    }
    
    fclose(rawfd);
}


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
	clock_t start;
	double time;
	int n_words; 
	
	while (true) {
		time = (std::clock() - start ) / (double) CLOCKS_PER_SEC;
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);     
		if (in_speech && !utt_started){ 
			utt_started = TRUE;
			start = std::clock();
			num_words = 1;
		}
		if (!in_speech && utt_started) {
			// speech -> silence transition, time to start new utterance  
			
			ps_end_utt(ps);

			hyp = ps_get_hyp(ps, &score);
			if (hyp != NULL){
				strncpy_s( str, hyp, sizeof( str ) );	
				n_words = count_words(str);
				if (n_words>3){
				printf(str);
				
				printf(", number of words:%d, time elapsed:%fs, speaking rate:%f",
					n_words,time,n_words/time*60);
				}
				else
					printf("not enough words");
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
			"-hmm", "model\\en-us\\en-us-c",
			"-lm","model\\en-us\\en-us.lm.dmp",
			"-vad_threshold","3",
			"-dict", "model\\en-us\\cmudict-en-us2.dict",
			//"-logfn","model",
			NULL);
	if (config == NULL)
		return false;
	ps = ps_init(config);
	if (ps == NULL)
		return false;
	//bool out = recognize_from_microphone();
	recognize_from_file();
	//printf(" %d",count_words("one two three"));
	ps_free(ps);
	cmd_ln_free_r(config);
	return 0;
}

