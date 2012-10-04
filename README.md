# Wav File Editor
This project is part of a Computational Audio Course CS591, given at the BU Computer Science department.


## Implemented Functionalityes:
Parse wav file and generate a new file with the same samples (Normal Copy ^^)

``./audiodize 0 beatles.wave beatlesfp.wav `./audio 0 beatles.wave beatlesfp.wav``

Cut wav file amplitude by half.

``./audiodize 1 beatles.wav beatleshalf.wav``

Pan the signal from left to right and back, with 5 seconds pan.

``./audiodize 2 beatlesmono.wav beatlespan.wav``

Create an Envelope for the wav file, and store values in a text file.

``./audiodize 3 beatles.wav beatles.env.txt``

Reshape wave file with the loaded amplitude.

``./audiodize 4 beethoven.wav beatles.env.txt beethoven.as.beatles.wav``
