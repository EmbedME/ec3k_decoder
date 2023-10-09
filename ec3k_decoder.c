/*
Software decoder for EnergyCounter 3000 sensors

Copyright (c) 2013 Thomas Fischl <tfischl@gmx.de>

Compile
gcc ec3k_decoder.c -o ec3k_decoder

Usage
sudo rtl_fm -f 868402000 -s 200000 -A fast - | ./ec3k_decoder

 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define LEVEL_THRESHOLD 47

#define BITTIME 10
#define BITTIME_BOUND_LOWER 9
#define BITTIME_BOUND_UPPER 11

#define PAKET_MIN_BITS 100

int main(int argc, char **argv) {

    char bitbuffer[1000];
    int bufferpos = 0;

    char buffer[2];
    char lastlevel = 0;
    int lastedge = 0;
    for(;;) {
        size_t bytes = fread(buffer,  sizeof(char), 2, stdin);
        if (bytes < 2) {
            if (feof(stdin))
                break;
        } else {
          char level = lastlevel;
          if ((buffer[1] > LEVEL_THRESHOLD) && (buffer[1] < 70)) level = 1;
          else if ((buffer[1] < LEVEL_THRESHOLD) && (buffer[1] > 20)) level = 0;

          if (level != lastlevel) {
              // detected edge

              if (lastedge >= BITTIME_BOUND_LOWER) {
                 // seems to be a bit
                 bitbuffer[bufferpos++] = 0;
              } else {
                 // out of sync
                 if (bufferpos > PAKET_MIN_BITS) {
                     int i;
/*
                     printf("Paket len=%i\n", bufferpos);
                     for (i = 0; i < bufferpos; i++) {
                         printf("%i", bitbuffer[i]);
                     }
                     printf("\n");
*/
                     unsigned char packetbuffer[100];
                     int packetpos = 0;
                     unsigned char packet = 0;
                     char onecount = 0;
                     unsigned char recbyte = 0;
                     char recpos = 0;
                     //printf("Descrambled/unstuffed: \n");
                     for (i = 17; i < bufferpos; i++) {
                         char out;

                         out = bitbuffer[i];
                         if (i > 17) out = out ^ bitbuffer[i - 17];
                         if (i > 12) out = out ^ bitbuffer[i - 12];                                                


                         if (out) {
                             onecount++;
                             recbyte = recbyte >> 1 | 0x80;
                             recpos ++;
                             if ((recpos == 8) && (packet)) {
                                 recpos = 0;
                                 packetbuffer[packetpos++] = recbyte;
                             }
//                             printf("%i", out);

                         } else {
                             if ((onecount < 5)) {
                                 recbyte = recbyte >> 1;
                                 recpos ++;
                                 if ((recpos == 8) && (packet)) {
                                     recpos = 0;
                                     packetbuffer[packetpos++] = recbyte;
                                 }
//                                 printf("%i", out);
                             }
                             if (onecount == 6) {
                                 packet = !packet;
                                 recpos = 0;

                                 if (packetpos == 41) {
                                     // received ec3k packet

                                     unsigned int id = (packetbuffer[0] & 0x0f) << (8 + 4) | (packetbuffer[1]) << 4 | (packetbuffer[2] ) >> 4;
                                     unsigned int wcurrent = (packetbuffer[15] & 0x0f) << (8 + 4) | (packetbuffer[16]) << 4 | (packetbuffer[17] ) >> 4;
                                     unsigned long energy = ((packetbuffer[33] & 0x0f) << (8 + 4) | (packetbuffer[34]) << 4 | (packetbuffer[35] ) >> 4);
                                     energy = energy << 28 |packetbuffer[12] << 20 | packetbuffer[13] << 12 | packetbuffer[14] << 4 | (packetbuffer[15] >> 4);

                                     printf("%lu,%x,%i,%lu\n", (long)time(NULL), id, wcurrent, energy);
                                     fflush(stdout);


                                 }
                                 packetpos = 0;
                             }
                             onecount = 0;
                         }                         

//                         printf("%i", out);
  
                     }
//                     printf("\n");


                 }
                 bufferpos = 0;
              }

              lastedge = 0;
            
          }

          if (lastedge >= BITTIME_BOUND_UPPER) {
              lastedge -= BITTIME;
              bitbuffer[bufferpos++] = 1;
          }

          lastedge++;
          lastlevel = level;

        }
    }
   
    fflush(stdout);

   return EXIT_SUCCESS;
}


