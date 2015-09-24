// speech.cpp : Defines the exported functions for the DLL application.
//

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "speech.h"
#include "stdafx.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <atomic>


using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;
static FILE grammerfile;
static char grammer_path[128] = "grammer.gram";
static char keyword_path[128] = "kws.file";
map<string,int> grammer;
std::atomic<bool> finish (false);

map<string,int> buildGrammer(const char * options,  int words, int sntc){
	ofstream grammer,kws;
	grammer.open (grammer_path);
	kws.open(keyword_path);
	map<string,int> elems;
	stringstream ss(options);
	string item;
	int n = 0;
	grammer << "#JSGF V1.0;\ngrammar options;\npublic <word> = (<option> | <garbage>+)+;\npublic <option> = \n";
	while (getline(ss, item, '\n')) {
		elems[item]=n++;
		kws << item;
		kws << " /";
		if (item.length() > 10){
			kws << "1e-";
			kws << sntc;
			kws <<"/";
		}
		else
			{
			kws << "1e-";
			kws << words;
			kws <<"/";
		}
		kws << "\n";
		grammer << item;
		grammer << "|";
	}
	long pos = grammer.tellp();
    grammer.seekp (pos-1);
	grammer << ";\n<garbage> = GARBAGE;";
	grammer.close();
	kws.clear();
	return elems;
}

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
	
	char str[256];
	char notfound[256] = "Not found!";
	

	while (!finish) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);
		if (in_speech && !utt_started) {
			utt_started = TRUE;
			
		}
		if (!in_speech && utt_started) {
			// speech -> silence transition, time to start new utterance  
			ps_end_utt(ps);
			hyp = ps_get_hyp(ps, &score);
			if (hyp != NULL){
				strncpy( str, hyp, sizeof( str ) );
				return grammer[string(str)];
			}
			//	else
			//		listenCallback(notfound);

			if (ps_start_utt(ps) < 0)
				perror("Failed to start utterance\n");
			utt_started = FALSE;
			
		}
		Sleep(100);
	}
	ad_close(ad);
	return -1;
}

extern void _cdecl stop(){
	finish = true;
}

extern int _cdecl init(const char* options, bool kws, int words, int sntc, CallbackType lCallback) {
	listenCallback = lCallback;
	finish = false;

	grammer = buildGrammer(options,  words,  sntc);
	if (kws){
	config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", "model\\en-us\\en-us",
		//  "-lm", MODELDIR "/en-us/en-us.lm.dmp",
		//"-jsgf", "grammer.gram",
		"-dict", "model\\en-us\\cmudict-en-us2.dict",
		//  "-keyphrase","can you repeat that",
		 "-kws", "kws.file",
		 "-mllr","model\\mllr_matrix",
		// "-kws_threshold", "1e-10",
		//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
		NULL);
	} else{
		config = cmd_ln_init(NULL, ps_args(), TRUE,
		"-hmm", "model\\en-us\\en-us",
		//  "-lm", MODELDIR "/en-us/en-us.lm.dmp",
		"-jsgf", "grammer.gram",
		"-dict", "model\\en-us\\cmudict-en-us-g.dict",
		//  "-keyphrase","can you repeat that",
		// "-kws", "kws.file",
		// "-kws_threshold", "1e-10",
		//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
		NULL);
	}
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



