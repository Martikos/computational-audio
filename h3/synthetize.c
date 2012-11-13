#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"
#include <math.h>

int main(int argc, char* argv[])
{
  printf("*********************************\n");
  printf("Assignment 3: Additive Synthesis.\n");
  printf("*********************************\n\n");

	PSF_PROPS props;
	int ofd = -1;
	int error = 0;
	PSF_CHPEAK* peaks = NULL;
	float* frame = NULL;
	psf_format outformat =  PSF_FMT_UNKNOWN;

	if(argc < 3){
		printf("Error: insufficient arguments.\n"
				"usage:\n\t"
				"sf2float <inputfile> <outputfile>\n");
		return 1;
	}
	/* be good, and startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}
	
  /* Setup props for writing to wav file */
  props.srate = 44100;
  props.chans = 2; 
  props.samptype = PSF_SAMP_IEEE_FLOAT;
  props.format = PSF_STDWAVE; 
  props.chformat = STDWAVE; 

  ofd = psf_sndCreate(argv[2],&props,0,0,PSF_CREATE_RDWR);

  if(ofd < 0){
    printf("Error: unable to create outfile %s\n",argv[3]);
    error++;
    goto exit;
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

  /*
   * Where my synthesis code starts.
   */

  /* Open file to read */
  FILE *file;
  file = fopen(argv[1], "r");


  float SAMPLE_RATE = 44100;

  float sineFrequencies[500] = {0};
  float sineAmplitudes[500] = {0};
  float prevSineAmplitudes[500] = {0};
  float intervalLimit = 0;
  float value;
  float time;

  float sineFrequency; 

  float currentSample = 0;
  int arraySize = 0;
  int arrayIndex = 0;

  /* for linear interpolation */
  float prevIntervalLimit = 0;
  float slope = 0;
  float intercept = 0;
  float factor = 1;

  /* Read file and store frequencies in array */
  printf("Info: started processing file.\n");
  while (!feof(file)) {
    fscanf(file, "%f", &sineFrequency);
    if(sineFrequency!=-1) {
      sineFrequencies[arraySize] = sineFrequency;
      arraySize++;
    }
    else {
      break;
    }
  }

  float sum = 0;
  arrayIndex = 0;
  int samplesWritten = 0;
  while(!feof(file)){

    /* in all cases save previous value */
    fscanf(file, "%f", &value);

    if(value == -999) {

      /* save lower interval limit */
      prevIntervalLimit = intervalLimit;
      /* save upper interval limit */
      fscanf(file, "%f", &intervalLimit);

      int ss = nearbyint(SAMPLE_RATE * intervalLimit);
      float sampleLimit = (float)ss;

      float current = 1;
      for(currentSample = currentSample; currentSample < sampleLimit; currentSample++) {
        // printf("%f\n", currentSample);
        float sum = 0;
        int frequencyIndex;
        float two = 2;

        int y = nearbyint((intervalLimit-prevIntervalLimit)*SAMPLE_RATE);
        for(frequencyIndex = 0; frequencyIndex < arraySize; frequencyIndex ++) {
          /* compute slope */
          slope = (sineAmplitudes[frequencyIndex]-prevSineAmplitudes[frequencyIndex])/(intervalLimit-prevIntervalLimit);
          intercept = prevSineAmplitudes[frequencyIndex];
          factor = ((int)currentSample%y)*slope/SAMPLE_RATE + intercept;
          sum += factor * sin(two * M_PI * sineFrequencies[frequencyIndex]*current/SAMPLE_RATE);
        }
        //printf("current sample: %f, sample limit: %f, sum: %d %f\n", currentSample, sampleLimit, yyy, sum);

          frame[0] = sum;
          frame[1] = sum;
          if(psf_sndWriteFloatFrames(ofd, frame, 1) !=1 ) {
            printf("Error writing to outfile\n");
            error ++;
            break;
          }
          current++;
          samplesWritten ++;
      }
      // printf("%d samples were written in interval %f\n", samplesWritten, sampleLimit);
      arrayIndex = 0;

      sum = 0;
    } else {
      // printf("arrayIndex: %d\n", arrayIndex);
      prevSineAmplitudes[arrayIndex] = sineAmplitudes[arrayIndex];
      sineAmplitudes[arrayIndex] = value;
      arrayIndex ++;
    }
    /* Compute the time of the current sample */

  }
  printf("Info: finished processing file.\n");
  printf("Info: %d samples written to file\n", samplesWritten);

  /*
   * Where my synthesis code ends.
   */

	/* do all cleanup  */    
exit:	 	
	if(ofd >= 0)
		psf_sndClose(ofd);
	if(frame)
		free(frame);
	if(peaks)
		free(peaks);
	psf_finish();
	return error;
}
