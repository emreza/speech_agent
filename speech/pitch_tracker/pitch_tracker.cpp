
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


int
main(int argc, char *argv[])
{
  string line;
  float prob, pitch;
  double pitch_array[1000] = {} ;
  ifstream myfile (argv[1]);
  int n=0;
  if (myfile.is_open())
  {
    while ( getline (myfile,line) )
    {
		sscanf (line.c_str(),"%*f %f %f", &prob, &pitch);
		if (prob>0.5 && pitch<400 && pitch>60)
			pitch_array[n++]=pitch;
    }
    myfile.close();
  }
  else cout << "Unable to open file"; 

  double sum = 0;
  for (int i=0;i<n;i++){
	  sum +=pitch_array[i];
  }
  double mean = sum/n;
  sum = 0;
  for(int i=0; i<n;i++)
	  sum+=(pitch_array[i]-mean)*(pitch_array[i]-mean);
  printf("average pitch: %f\n",mean);
  printf("std pitch: %f", sqrt(sum/n));

  return 0;
}


