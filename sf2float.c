/* sf2float.c :  convert soundfile to floats using portsf library */

#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"

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

	if(argc < 3){
		printf("insufficient arguments.\n"
				"usage:\n\t"
				"audiodize infile outfile\n");
		return 1;
	}
	/* be good, and startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}
	
	ifd = psf_sndOpen(argv[1],&props,0);																		  
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
	outformat = psf_getFormatExt(argv[2]);
	if(outformat == PSF_FMT_UNKNOWN){
		printf("outfile name %s has unknown format.\n"
			"Use any of .wav, .aiff, .aif, .afc, .aifc\n",argv[2]);
		error++;
		goto exit;
	}
	props.format = outformat;

	ofd = psf_sndCreate(argv[2],&props,0,0,PSF_CREATE_RDWR);
	if(ofd < 0){
		printf("Error: unable to create outfile %s\n",argv[2]);
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
	printf("copying....\n");

	/* single-frame loop to do copy: report any read/write errors */
	framesread = psf_sndReadFloatFrames(ifd,frame,1);
	totalread = 0;		/* count sample frames as they are copied */
    int downSampleFactor = 1;
    int pushupFactor = 0;
    int down = 1;
	while (framesread == 1){

        /*
         * My own code begins.
         */

            /*
             * Example division by 2
             */
            frame[0] = frame[0]/2;
            frame[1] = frame[1]/2;

        /*
         * My own code ends.
         */


        // if((int)(totalread/downSampleFactor)==(totalread/downSampleFactor))
        for (i = 0; i < downSampleFactor; i++) {
            totalread++;
            if(psf_sndWriteFloatFrames(ofd,frame,1) != 1){
                printf("Error writing to outfile\n");
                error++;
                break;
            }
            // downSampleFactor = (downSampleFactor++)%10;

        }

		framesread = psf_sndReadFloatFrames(ifd,frame,1);
	}
	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("Done. %ld sample frames copied to %s\n",totalread,argv[2]);
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
