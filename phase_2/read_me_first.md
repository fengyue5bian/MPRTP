In phase 2 we successfully achieve sending the packets (including FEC packets) to the receiver side and play them out by VLC player by using SPD file currently.

The rtp directory currently is just for single path RTP packet sending, I will modify it to achieve MP-RTP very soon the moment we finish designing the MP-RTP architecture.

The test.c file in FEC directory is the original test file written by Luigi Rizzo for testing the encoding and decoding speed, which is not quite clear to us for general usage. So I write a my_test.c file so that you guys can understand the usage and detailed information behind the algorithm.
