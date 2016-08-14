/****************************************************************************
 File        : net.c

 Description : Simple Network server that accepts colors over TCP/UDP and
               throws the colors onto the lights

 License     : Copyright 2011 Phillip Burgess.   www.PaintYourDragon.com
 							 Derived 2016 James Hagerman.      zenpirate.com

               This Program is free software: you can redistribute it and/or
               modify it under the terms of the GNU General Public License as
               published by the Free Software Foundation, either version 3 of
               the License, or (at your option) any later version.

               This Program is distributed in the hope that it will be
               useful, but WITHOUT ANY WARRANTY; without even the implied
               warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
               PURPOSE.  See the GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with this Program.  If not, see
               <http://www.gnu.org/licenses/>.

               Additional permission under GNU GPL version 3 section 7

               If you modify this Program, or any covered work, by linking
               or combining it with libftd2xx (or a modified version of that
               library), containing parts covered by the license terms of
               Future Technology Devices International Limited, the licensors
               of this Program grant you additional permission to convey the
               resulting work.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "p9813.h"


// Will be called if we hit an error:
void error(char *msg)
{
    perror(msg);
    exit(1);
}



int main(int argc,char *argv[])
{
	// Socket stuff:
	int sockfd, newsockfd, portno, clilen, n;
	int bufferSize = 600*3+10; // +10 is just in case...
	char buffer[bufferSize];
	struct sockaddr_in serv_addr, cli_addr;

	struct sockaddr_in {
    short   sin_family;
    u_short sin_port;
    struct  in_addr sin_addr;
    char    sin_zero[8];
	};

	// LED Stuff:
	double        x,s1,s2,s3;
	int           i,totalPixels,
	  nStrands           = 1,
	  pixelsPerStrand    = 25;
	unsigned char r,g,b;
	time_t        t,prev = 0;
	TCstats       stats;
	TCpixel       *pixelBuf;

	while((i = getopt(argc,argv,"s:c:p:")) != -1)
	{
		switch(i)
		{
			case 's':
				nStrands        = strtol(optarg,NULL,0);
				break;
			case 'c':
				pixelsPerStrand = strtol(optarg,NULL,0);
				break;
			case 'p':
				portno          = atoi(optarg);
				break;
			case '?':
			default:
				(void) printf("usage: %s [-s strands] [-c pixel count] [-p port number]\n",argv[0]);
				return 1;
		}
	}

	printf("Listening on port: %i\n", portno);

	/* Allocate pixel array.  One TCpixel per pixel per strand. */
	totalPixels = nStrands * pixelsPerStrand;
	i           = totalPixels * sizeof(TCpixel);
	if(NULL == (pixelBuf = (TCpixel *)malloc(i)))
	{
		printf("Could not allocate space for %d pixels (%d bytes).\n",
		  totalPixels,i);
		return 1;
	}

	/* Initialize library, open FTDI device.  Baud rate errors
	   are non-fatal; program displays a warning but continues. */
	if((i = TCopen(nStrands,pixelsPerStrand)) != TC_OK)
	{
		TCprintError(i);
		if(i < TC_ERR_DIVISOR) return 1;
	}

	/* Initialize statistics structure before use. */
	TCinitStats(&stats);


	// Set up the networking:
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

	listen(sockfd,5);

	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
		error("ERROR on accept");
	}

	/* The demo animation sets every pixel in every frame.  Your code
	   doesn't necessarily have to --  it could just change altered
	   pixels and call TCrefresh().  The example is some swirly color
	   patterns using a combination of sine waves.  There's no meaning
	   to any of this, just applying various constants at each stage
	   in order to avoid repetition between the component colors. */
	for(x=0.0;;x += (double)pixelsPerStrand / 20000.0)
	{
		bzero(buffer, bufferSize);
		n = read(newsockfd, buffer, bufferSize-1);
		if (n < 0) error("ERROR reading from socket");
		printf("Here is the message: %s\n", buffer);

		n = write(newsockfd,"I got your message",18);
		if (n < 0) error("ERROR writing to socket");
		
		s1 = sin(x                 ) *  11.0;
		s2 = sin(x *  0.857 - 0.214) * -13.0;
		s3 = sin(x * -0.923 + 1.428) *  17.0;
		for(i=0;i<totalPixels;i++)
		{
			r   = (int)((sin(s1) + 1.0) * 127.5);
			g   = (int)((sin(s2) + 1.0) * 127.5);
			b   = (int)((sin(s3) + 1.0) * 127.5);
			pixelBuf[i] = TCrgb(r,g,b);
			s1 += 0.273;
			s2 -= 0.231;
			s3 += 0.428;
		}

		if((i = TCrefresh(pixelBuf,NULL,&stats)) != TC_OK)
			TCprintError(i);

		/* Update statistics once per second. */
// 		if((t = time(NULL)) != prev)
// 		{
// #ifdef CYGWIN
// 			printf("\E[2J");
// #else
// 			system("clear");
// #endif
// 			TCprintStats(&stats);
// 			prev = t;
// 		}
	}

	TCclose();
	free(pixelBuf);
	return 0;
}
