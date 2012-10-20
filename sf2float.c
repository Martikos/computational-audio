/* sf2float.c :  convert soundfile to floats using portsf library */

#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"
#include <math.h>

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

	printf("SF2FLOAT: convert soundfile to 32bit floats format\n");

	if(argc < 4){
		printf("insufficient arguments.\n"
				"usage:\n\t"
				"sf2float option infile outfile\n");
		return 1;
	}
	/* be good, and startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}
	
	ifd = psf_sndOpen(argv[2],&props,0);																		  
	if(ifd < 0 && atoi(argv[1])!=5){
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
	/* check file extension of outfile name, so we use correct output file format*/

  // TODO: needs to be changed because output is not always the same format
  if (atoi(argv[1])<3 || atoi(argv[1])==5) {
    outformat = psf_getFormatExt(argv[3]);
    printf("output file format: %d", outformat);
    if(outformat == PSF_FMT_UNKNOWN){
      printf("outfile name %s has unknown format.\n"
        "Use any of .wav, .aiff, .aif, .afc, .aifc\n",argv[3]);
      error++;
      goto exit;
    }
  }
  else if (atoi(argv[1])==4) {
    outformat = psf_getFormatExt(argv[4]);
    printf("output file format: %d", outformat);
    if(outformat == PSF_FMT_UNKNOWN){
      printf("outfile name %s has unknown format.\n"
        "Use any of .wav, .aiff, .aif, .afc, .aifc\n",argv[4]);
      error++;
      goto exit;
    }
  }
	props.format = outformat;

	if(atoi(argv[1])<3 || atoi(argv[1])==5) {
    props.srate = 44100;
    props.chans = 2; 
    props.samptype = PSF_SAMP_IEEE_FLOAT;
    props.format = PSF_STDWAVE; 
    props.chformat = STDWAVE; 

    ofd = psf_sndCreate(argv[3],&props,0,0,PSF_CREATE_RDWR);

    if(ofd < 0){
      printf("Error: unable to create outfile %s\n",argv[3]);
      error++;
      goto exit;
    }
  }
  else if (atoi(argv[1])==4) {
    ofd = psf_sndCreate(argv[4],&props,0,0,PSF_CREATE_RDWR);
    if(ofd < 0){
      printf("Error: unable to create outfile %s\n",argv[4]);
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

   // if(atoi(argv[1])!=5) {
     //printf("copying....\n");
    /* single-frame loop to do copy: report any read/write errors */
    // framesread = psf_sndReadFloatFrames(ifd,frame,1);
    // totalread = 0;		/* count sample frames as they are copied */
  // }

  int part = atoi(argv[1]);

  /*
   * Switch depending on the part of the assignment
   */
  switch(part) {

    /*
     * Part 0: Output the file as is.
     */
    case 0:
      while (framesread == 1){
        totalread++;
        if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
            printf("Error writing to outfile\n");
            error++;
            break;
        }
        framesread = psf_sndReadFloatFrames(ifd,frame,1);
      }
      break;

    /*
     * Part 1: Divide the value of each sample by 2.
     */
    case 1: {
              while (framesread == 1){
                frame[0] = frame[0]/2;
                frame[1] = frame[1]/2;
                totalread++;
                if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
                    printf("Error writing to outfile\n");
                    error++;
                    break;
                }
                framesread = psf_sndReadFloatFrames(ifd,frame,1);
              }
            break;
            }

    /*
     * Part 2: Pan the output from left to right, with an interval of 5 seconds.
     */
    case 2: {
              double cycle = 5; // cycle of 5 seconds
              double period = 4 * cycle; // sine or cosine period
              double sampleLimit = period * 44100;
              while (framesread == 1){
                frame[0] = frame[0]*sin(2*M_PI*totalread/sampleLimit);
                frame[1] = frame[1]*cos(2*M_PI*totalread/sampleLimit);

                totalread++;
                if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
                    printf("Error writing to outfile\n");
                    error++;
                    break;
                }
                framesread = psf_sndReadFloatFrames(ifd,frame,1);
              }
              break;
            }
    /*
     * Part 3: Create envelope file.
     */
    case 3:
            {
              double max = 0; // to keep track of the max in the interval
              int currentSample = 0;
              double intervalTime = 0.05;
              double sampleLimit = 44100*intervalTime;
              int interval = 1;
              FILE *file;
              file = fopen(argv[3], "w+");
              printf("sampleLimit %f\n", sampleLimit);
              while (framesread == 1){
                if (currentSample < sampleLimit) {
                  if (max < frame[0]) {
                    max = frame[0];
                  }
                  if (max < -1*frame[0]) {
                    max = -1*frame[0];
                  }
                  if (max < frame[1]) {
                    max = frame[1];
                  }
                  if (max < -1*frame[1]) {
                    max = -1*frame[1];
                  }
                  currentSample++;
                }
                else {
                  fprintf(file, "%f %f\n", (double)(interval*intervalTime), max);
                  max = 0;
                  currentSample = 0;
                  interval++;
                }

                totalread++;
                framesread = psf_sndReadFloatFrames(ifd,frame,1);
              }
              fclose(file);
              break;
            }
    /*
     * apply envelope on file
     */
    case 4:
            {
              // open up file, put the reading index on the first line
              FILE *file;
              file = fopen(argv[3], "r");
              double intervalTime = 0.05;

              char intervalLimitString[20], valueString[20], space[1];
              double intervalLimit=-1, value=0;
              // to do the linear regression and get the factor
              double prevValue = 0;
              double slope = 0;
              double intercept = 0;
              double factor = 1;
              double current=0;

              while (framesread == 1) {
                if (!feof(file)) {
                  if(current/44100 >= intervalLimit ) {
                    if(fgets(intervalLimitString, 10, file) != NULL) {
                      intervalLimit = strtod(intervalLimitString, NULL);
                    }
                    if(fgets(space, 1, file) != NULL) {}
                    if(fgets(valueString, 12, file) != NULL) {
                      prevValue = value;
                      value = strtod(valueString, NULL);
                      slope = (value-prevValue)/intervalTime;
                      intercept = prevValue;
                    }
                  }
                  factor = ((double)(totalread%((int)(44100*intervalTime)))/44100)*slope + intercept;
                  frame[0] = frame[0]*factor;
                  frame[1] = frame[1]*factor;
                  int tt = totalread%2205;
                  // printf("time: %d \t value: %f  \t slope %f \t intercep %f \t will multiply by: %f\n", tt, value, slope, intercept, factor);

                }
                current++;
                totalread++;
                if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
                    printf("Error writing to outfile\n");
                    error++;
                    break;
                }
                framesread = psf_sndReadFloatFrames(ifd,frame,1);
              }
              break;
            }
    /*
     * add up sine waves
     */
    case 5: {
              FILE *file;
              file = fopen(argv[2], "r");
              char sineAmplitudeString[20], sineFrequencyString[20], space[1];
              double sineAmplitude, sineFrequency; 
              double sineAmplitudes[20], sineFrequencies[20];
              int currentIndex = 0;
              int arraySize=0;
              while (!feof(file)) {
                if(fgets(sineFrequencyString, 5, file) != NULL) {
                      sineFrequency = strtod(sineFrequencyString, NULL);
                  if(sineFrequency==-1)
                    break;
                  else {
                      sineFrequencies[currentIndex] = sineFrequency;
                    if(fgets(space, 1, file) != NULL) {}
                    if(fgets(sineAmplitudeString, 20, file) != NULL) {
                      sineAmplitude = strtod(sineAmplitudeString, NULL);
                      sineAmplitudes[currentIndex] = sineAmplitude;
                      currentIndex++;
                    }
                  }
                }
              }
              arraySize = currentIndex;
              currentIndex = 0;
              int i;
              for(i=0; i<arraySize; i++) {
                printf(" %f %f \n", sineAmplitudes[i], sineFrequencies[i]);
              }
              // do i assume it is 0.05?
              double intervalTime = 0.05;
              char intervalLimitString[20], valueString[20];
              double intervalLimit=-1, value=0;
              // to do the linear regression and get the factor
              double prevValue = 0;
              double slope = 0;
              double intercept = 0;
              double factor = 1;
              double current=0;

              while(!feof(file)){
                  if(current/44100 >= intervalLimit ) {
                    if(fgets(intervalLimitString, 10, file) != NULL) {
                      intervalLimit = strtod(intervalLimitString, NULL);
                    }
                    if(fgets(space, 1, file) != NULL) {}
                    if(fgets(valueString, 12, file) != NULL) {
                      prevValue = value;
                      value = strtod(valueString, NULL);
                      slope = (value-prevValue)/intervalTime;
                      intercept = prevValue;
                    }
                  }
                  factor = ((double)((int)current%((int)(44100*intervalTime)))/44100)*slope + intercept;
                  printf("%f\n", factor);
                  // compute frame[1] and frame[2]
                  frame[0] = 0;
                  int i;
                  for(i=0; i<arraySize; i++) {
                    frame[0] += sineAmplitudes[i]*sin(2*M_PI*current*sineFrequencies[i]/44100);
                  }
                  frame[0] = frame[0]*factor;
                  frame[1] = frame[0];

                  if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
                      printf("Error writing to outfile\n");
                      error++;
                      break;
                  }
                  current++;
              }
              break;
            }
    default:
      printf("default case");
      break;
  }

	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else {
    if (atoi(argv[1])<3) {
      printf("Done. %ld sample frames copied to %s\n",totalread,argv[3]);
    }
    else if (atoi(argv[1])==4) {
      printf("Done. %ld sample frames copied to %s\n",totalread,argv[4]);
    }
  }
	/* report PEAK values to user */
	if(psf_sndReadPeaks(ofd,peaks,NULL) > 0){
		long i;
		double peaktime;
		printf("PEAK information:\n");	
			for(i=0;i < props.chans;i++){
				peaktime = (double) peaks[i].pos / (double) props.srate;
				printf("CH %ld:\t%.4f at %.4f secs\n", i+1, peaks[i].val, peaktime);
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
