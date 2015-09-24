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
#include <ctime>
#include "wavfile.h"
#include <math.h>

using namespace std;

static ps_decoder_t *ps;
static cmd_ln_t *config;
static FILE *rawfd;
static FILE grammerfile;
static char grammer_path[128] = "grammer.gram";
static char keyword_path[128] = "kws.file";
map<string,int> grammer;
std::atomic<bool> finish (false);

map<string,int> buildGrammer(const char * options, int words, int sntc){
	ofstream grammer,kws;
	grammer.open (grammer_path);
	kws.open(keyword_path);
	map<string,int> elems;
	stringstream ss(options);
	string item;
	int n = 0;
	grammer << "#JSGF V1.0;\ngrammar options;\npublic <option> = \n";
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
	grammer << ";";
	grammer.close();
	kws.clear();
	return elems;
}

void writewavfile(int16* data,int size){
#define S_RATE  (16000)
	//#define BUF_SIZE (S_RATE*2) /* 2 second buffer */
	write_wav("test.wav", size, data, S_RATE);
}

int recognize_from_microphone(double waitTime) {
	ad_rec_t *ad;
	int16 adbuf[2048];
	int16 alldata[160000];
	int16* outData;
	uint8 utt_started, in_speech;
	int32 k;
	char const *hyp;
	int32 score,prob;
	float conf;  
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
	std::clock_t start;
	double duration;
	bool confirm_phase = false;
	while (!finish) {
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
			perror("Failed to read audio\n");
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);     
		if (in_speech){
			if (confirm_phase){
				ad_close(ad);
				return 0;
			}
			for (int i=0; i<k; i++)
				alldata[data_count+i] = adbuf[i];
			data_count += k;
			data_count %= 160000;	
		}
		if (in_speech && !utt_started) 
			utt_started = TRUE;
		if (confirm_phase && ((std::clock() - start ) / (double) CLOCKS_PER_SEC) > waitTime)
		{
			//writewavfile(alldata,data_count);
			ad_close(ad);
			return 1;
		}
		if (!in_speech && utt_started) {
			// speech -> silence transition, time to start new utterance  
			ps_end_utt(ps);
			hyp = ps_get_hyp(ps, &score);
			if (hyp != NULL){
				strncpy( str, hyp, sizeof( str ) );
				outData = new short[data_count];
				copy(alldata, alldata+data_count-1, outData); 
				listenCallback(outData, data_count, grammer[string(str)]);
				start = std::clock();
				confirm_phase = true;
			}
			data_count = 0;
			//	else
			//		listenCallback(notfound);
			if (ps_start_utt(ps) < 0)
				perror("Failed to start utterance\n");
			utt_started = FALSE;
		}
		Sleep(50);
	}
	ad_close(ad);
	return 0;
}

extern void _cdecl stop(){
	finish = true;
}

extern int _cdecl run(const char* options, bool kws, int words, int sntc, double waitTime, double vad, CallbackType lCallback) {
	listenCallback = lCallback;
	finish = false;
	grammer = buildGrammer(options,  words,  sntc);
	if (kws){
		config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-hmm", "model\\en-us\\en-us",
			//"-lm", MODELDIR "/en-us/en-us.lm.dmp",
			//"-jsgf", "grammer.gram",
			"-dict", "model\\en-us\\cmudict-en-us2.dict",
			//"-keyphrase","can you repeat that",
			"-kws", "kws.file",
			"-mllr","model\\mllr_matrix",
			//"-kws_threshold", "1e-10",
			//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
			NULL);
	} else{
		config = cmd_ln_init(NULL, ps_args(), TRUE,
			"-vad_threshold",std::to_string(vad).c_str(),
			"-hmm", "model\\en-us\\en-us",
			//"-lm", MODELDIR "/en-us/en-us.lm.dmp",
			"-jsgf", "grammer.gram",
			"-dict", "model\\en-us\\cmudict-en-us2.dict",
			//"-keyphrase","can you repeat that",
			//"-kws", "kws.file",
			//"-kws_threshold", "1e-10",
			//"-logfn","C:\\Users\\Reza\\Documents\\Speech",
			NULL);
	}
	if (config == NULL)
		return -1;
	ps = ps_init(config);
	if (ps == NULL)
		return -2;

	int out = recognize_from_microphone(waitTime);

	ps_free(ps);
	cmd_ln_free_r(config);
	return out;
}



