/* sf2float.c :  convert soundfile to floats using portsf library */

#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"
#include <string.h>

int main(int argc, char* argv[])
{
        int i; 
	PSF_PROPS props;
	long framesread;
	long totalread;
	/* init all dynamic resources to default states */
	int ifd = -1,ofd = -1;
	int error = 0;
	PSF_CHPEAK* peaks = NULL;
	float* frame = NULL;
	psf_format outformat =  PSF_FMT_UNKNOWN;
  
  /*
   * variables to be used in my customized program
   */
  int multiplier = 1;
  int pan = 0.5;
  int env_interval = 50; // in milliseconds

	printf("SF2FLOAT: convert soundfile to 32bit floats format\n");

	if(argc < 3){
		printf("insufficient arguments.\n"
				"usage:\n\t"
				"audiodize infile outfile\n");
		return 1;
	}

  /*
   * created variables for saving mods
   */
  int mode_m = 0;
  int mode_sm = 0;
  int mode_ltr = 0;
  int mode_pan = 0;
  int mode_env = 0;

  /*
   * for lack of a good argument parser,
   * odd arguments are argument types,
   * even arguments are argument values.
   */
  int infileIndex = 0;
  int outfileIndex = 0;

  for (i = 1; i < argc-1; i+=2) {
    if (!strcmp(argv[i],"-m")) {
      multiplier = atoi(argv[i+1]);
      mode_m = 1;
    }
    if (!strcmp(argv[i],"-sm")) {
      multiplier = atoi(argv[i+1]);
      mode_sm = 1;
    }
    if (!strcmp(argv[i],"-pan")) {
      pan = atoi(argv[i+1]);
      mode_pan = 1;
    }
    if (!strcmp(argv[i],"-env")) {
      env_interval = atoi(argv[i+1]);
      mode_env= 1;
    }
    if (!strcmp(argv[i],"-ltr")) {
      mode_ltr = 1;
      i--;
    }

    if (!strcmp(argv[i], "-i")) {
      infileIndex = i+1;
    }
    if (!strcmp(argv[i], "-o")) {
      outfileIndex = i+1;
    }

  }
  printf("input file: %s", argv[infileIndex]);
  printf("output file: %s", argv[outfileIndex]);

	/* be good, and startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}
  if(infileIndex==0 || outfileIndex==0) {
    printf("Error: infile or outfile not specified\n");
    return 1;
  }
	
	ifd = psf_sndOpen(argv[infileIndex],&props,0);																		  
	if(ifd < 0){
		printf("Error: unable to open infile %s\n",argv[1]);
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
	/* check file extension of outfile name, so we use correct output file format*/
	outformat = psf_getFormatExt(argv[outfileIndex]);
	if(outformat == PSF_FMT_UNKNOWN && !(mode_env==1)){
		printf("outfile name %s has unknown format.\n"
			"Use any of .wav, .aiff, .aif, .afc, .aifc\n",argv[2]);
		error++;
		goto exit;
	}
	props.format = outformat;

  if(mode_env!=1) {
    ofd = psf_sndCreate(argv[outfileIndex],&props,0,0,PSF_CREATE_RDWR);
    if(ofd < 0){
      printf("Error: unable to create outfile %s\n",argv[2]);
      error++;
      goto exit;
    }
  }

	/* allocate space for one sample frame */
	frame = (float*) malloc(props.chans * sizeof(float));
	if(frame==NULL){
		puts("No memory!\n");
		error++;
		goto exit;
	}
	/* and allocate space for PEAK info */
	peaks  =  (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
	if(peaks == NULL){
		puts("No memory!\n");
		error++;
		goto exit;
	}
	printf("copying....\n");

	/* single-frame loop to do copy: report any read/write errors */
	framesread = psf_sndReadFloatFrames(ifd,frame,1);
	totalread = 0;		/* count sample frames as they are copied */



  /*
   * My own code begins.
   */

  /*
   * for pre processing, do a first visit over the file
   */

  /*
   * mode: multiplication (-m or -sm)
   */
  if(mode_m==1 || mode_sm==1) {
    while(framesread == 1) {
      double xx = frame[0];
      double yy = frame[1];
      frame[0] = frame[0]*multiplier;
      frame[1] = frame[1]*multiplier;
      printf("%f %f\t%f %f\n", xx, yy, frame[0], frame[1]);
      
      totalread++;
      if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
          printf("error writing to outfile\n");
          error++;
          break;
      }
      framesread = psf_sndReadFloatFrames(ifd,frame,1);
    }
  }
  if(mode_ltr==1) {
    while(framesread == 1) {
      frame[1] = frame[0];
      printf("%f %f\n", frame[0], frame[1]);
      
      totalread++;
      if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
          printf("error writing to outfile\n");
          error++;
          break;
      }
      framesread = psf_sndReadFloatFrames(ifd,frame,1);
    }
  }

  /*
   * mode: panning (-pan)
   */
  else if(mode_pan==1) {
    int cycleSampleSize = pan*44100;
    // when left is going up and right is going down
    int cycle1Limit = cycleSampleSize/4;
    // when left is going down and right is going up
    int cycle4Limit = 3*cycleSampleSize/4;
    int currentSample = 0;

    int sampleCount = 0;
    double panFactor = 1;
    
    while (framesread == 1){
      frame[0]*=panFactor;
      frame[1]/=panFactor;
      if (currentSample<=cycle1Limit || currentSample>cycle4Limit) {
        panFactor+=0.000015;
      }
      else {
        panFactor-=0.000015;
      }
      // printf("%f\n", panFactor);
      totalread++;
      currentSample++;
      if(currentSample==pan*44100) {
        currentSample = 0;
      }
      if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
          printf("error writing to outfile\n");
          error++;
          break;
      }
      framesread = psf_sndReadFloatFrames(ifd,frame,1);
    }
  }
  /*
   * envelop creator mode (-env)
   */
  else if(mode_env==1) {
    FILE *file;
    file = fopen(argv[outfileIndex], "w+");
    double sampleInterval = 44100*(double)env_interval/1000;
    double currentSample = 0;
    double max = 0;
    double n =0;
    while (framesread ==1) {
      if(currentSample < sampleInterval) {
        if(max<frame[0]) {
          max=frame[0];
        }
        if (max<frame[1]) {
          max=frame[1];
        }
        currentSample++;
      }
      else {
        currentSample = 0;
        fprintf(file,"%f %f\n", (double)(n*sampleInterval), max);
        max = 0;
        n++;
      }
      totalread++;
      if(currentSample==pan*44100) {
        currentSample = 0;
      }
      
      framesread = psf_sndReadFloatFrames(ifd,frame,1);
    }
    fclose(file);
  }


  /*
   * My own code ends.
   */
	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("Done. %ld sample frames copied to %s\n",totalread,argv[2]);
	/* report PEAK values to user */
  if(mode_env!=1) {
    if(psf_sndReadPeaks(ofd,peaks,NULL) > 0){
      long i;
      double peaktime;
      printf("PEAK information:\n");	
        for(i=0;i < props.chans;i++){
          peaktime = (double) peaks[i].pos / (double) props.srate;
          printf("CH %ld:\t%.4f at %.4f secs\n", i+1, peaks[i].val, peaktime);
        }
    }	
  }
	/* do all cleanup  */    
exit:	 	if(ifd >= 0)
		psf_sndClose(ifd);
	if(ofd >= 0)
		psf_sndClose(ofd);
	if(frame)
		free(frame);
	if(peaks)
		free(peaks);
	psf_finish();
	return error;
}
