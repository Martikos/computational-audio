# Wav File Editor
This project is part of a Computational Audio Course CS591, given at the BU Computer Science department.


#### Parses wav file and generates a new file based on the same samples (Normal Copy ^^)
./audio 0 beatles.wave beatlesfp.wav 
#### Cuts wav file amplitude by half.
./audio 1 beatles.wav beatleshalf.wav         
#### Pans the signal from left to right and back, implemented 5 seconds pan.
./audio 2 beatlesmono.wav beatlespan.wav      
#### Creates an Envelope for the wav file, and stores values in a text file.
./audio 3 beatles.wav beatles.env.txt         
#### Reshape wave file with the loaded amplitude.
./audio 4 beethoven.wav beatles.env.txt beethoven.as.beatles.wav  
