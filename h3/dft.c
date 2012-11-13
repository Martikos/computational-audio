#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include "portsf.h"

int main(int argc, char* argv[])
{

  printf("**************************\n");
  printf("Assignment 3: DFT Analysis\n");
  printf("**************************\n\n");

  /* 
   * sf2float.c initializations 
   */
	PSF_PROPS props;
	long framesread;
	long totalread;
	/* init all dynamic resources to default states */
	int ifd = -1,ofd = -1;
	int error = 0;
	PSF_CHPEAK* peaks = NULL;
	float* frame = NULL;
	psf_format outformat =  PSF_FMT_UNKNOWN;
  
	if(argc < 2){
		printf("Error: insufficient arguments.\n"
				"Usage:\n\t"
				"dft <wavfile>\n");
		return 1;
	}
	/* be good, and startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}
	ifd = psf_sndOpen(argv[1],&props,0);																		  
	if(ifd < 0){
		printf("Error: unable to open infile %s\n",argv[2]);
		return 1;
	}
	if(props.samptype == PSF_SAMP_16){
		printf("Info: infile is in 16 bit format.\n");		
	}
	if(props.samptype == PSF_SAMP_24){
		printf("Info: infile is in 24 bit format.\n");		
	}
  if(props.samptype == PSF_SAMP_32){
		printf("Info: infile is in 32 bit format.\n");		
	}
	/* we now have a resource, so we use goto hereafter on hitting any error */
	/* tell user if source file is already floats  */
	if(props.samptype == PSF_SAMP_IEEE_FLOAT){
		printf("Info: infile is already in floats format.\n");		
	}
	props.samptype = PSF_SAMP_IEEE_FLOAT;
	props.format = outformat;

  char outputExtension[] = ".env.txt";
  /* use buffer overflow exploit here */
  strcat(argv[1], outputExtension); 
  printf("Info: Writing to file %s.\n", argv[1]);

	/* allocate space for one sample frame */
	frame = (float*) malloc(props.chans * sizeof(float));
	if(frame==NULL){
		puts("No memory!\n");
		error++;
    return 1;
	}
	/* and allocate space for PEAK info */
	peaks  =  (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
	if(peaks == NULL){
		puts("No memory!\n");
		error++;
    return 1;
	}

	framesread = psf_sndReadFloatFrames(ifd,frame,1);
  

  /*
   * This is where my program starts
   */


  /* Variables Initialization */
  float SAMPLE_RATE = 44100;
  double INTERVAL = 0.05;
  float SAMPLE_PER_INTERVAL = SAMPLE_RATE*INTERVAL;

  float START_FREQUENCY = 20;
  float END_FREQUENCY = 10000;
  float FREQUENCY_INCREMENT = 20;

  float absoluteSampleIndex = 0;
  float relativeSampleIndex = 0;
  float sumOfProducts [500] = {0};

  float value, window;
  float sample, frequency;

  /* Open file for writing */
  FILE *file;
  file = fopen(argv[1], "w+");

  for(frequency = START_FREQUENCY; frequency <= END_FREQUENCY; frequency += FREQUENCY_INCREMENT) {
    fprintf(file, "%f\n", frequency);
  }
  fprintf(file, "-1\n");

  printf("Info: Started Processing File.\n");
  while(framesread == 1) {
    if(relativeSampleIndex == SAMPLE_PER_INTERVAL) {
      fprintf(file, "-999 %f\n", (float)(absoluteSampleIndex/SAMPLE_RATE));
      relativeSampleIndex = 0;
      for(frequency = START_FREQUENCY; frequency <= END_FREQUENCY; frequency += FREQUENCY_INCREMENT) {
        int index = ((int)(frequency)/FREQUENCY_INCREMENT)-1;
        fprintf(file, "%f\n", sumOfProducts[index]);
        sumOfProducts[index] = 0;
      }
    }
    // printf("%f %f\n", frame[0], frame[1]);
    for(frequency = START_FREQUENCY; frequency <= END_FREQUENCY; frequency += FREQUENCY_INCREMENT) {
      int index = ((int)(frequency)/FREQUENCY_INCREMENT)-1;
      /* Compute sin value */
      float two = 2;
      value = sin(two*M_PI*frequency*relativeSampleIndex/SAMPLE_RATE);
      // printf("%f\n", value);
      /* Compute Hann's window value */
      // window = -1 * cos(2*M_PI*relativeSampleIndex/SAMPLE_PER_INTERVAL) * 0.5 + 1;
      // window = -1 * cos(2*M_PI*relativeSampleIndex/SAMPLE_PER_INTERVAL) * 0.5 + 0.5;
      window = 0.5 * ( 1 - cos (2*M_PI*relativeSampleIndex/SAMPLE_PER_INTERVAL));
      // window = -1 * cos(2*M_PI*index/500) * 0.5 + 0.5;
      // window = 1;
      // sumOfProducts[(int)(frequency)/440-1] += frame[0] * value * window * 2 / SAMPLE_PER_INTERVAL;
      sumOfProducts[index] += frame[0] * value * window * 2 / SAMPLE_PER_INTERVAL;
      // printf("%f\n", sumOfProducts[index]);
    }
    framesread = psf_sndReadFloatFrames(ifd,frame,1);

    absoluteSampleIndex ++;
    relativeSampleIndex ++;
  }
  printf("Info: Sample rate per interval: %f\n", SAMPLE_PER_INTERVAL);
  printf("Info: Ended Processing File.\n");

  /* Close file after writing */
  fclose(file);
  /*
   * This is where my program ends
   */

  printf("Computed Sine Values\n");

  return 0;
}
