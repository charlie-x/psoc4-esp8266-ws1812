// simple_udp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define LIGHTS		L"192.168.1.230"	// Address of Node
#define BUFLEN		( 450  )			// Length of data to send to LED strips, 150 LEDs, three byte values per LED , 450
#define PORT		(40002)				// The port the ESP is listening on


// short cut exit routine
void die(const char *str)
{
	fprintf(stderr, "error: %s\n",str);

	WSACleanup();

	// exit with error
	exit(1);
}

int main(void)
{
	WSADATA  w ;

	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	unsigned char data[BUFLEN];

	/* Open windows connection */
	if (WSAStartup(MAKEWORD(2, 2), &w) != 0) {
		fprintf(stderr, "could not initialise winsock\n");
		exit(0);
	}

	// create a udp socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("socket create error");
	}

	// setup the target port/address
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);


	// convert IP address string to numerical
	if (1 != InetPton(AF_INET, LIGHTS, &si_other.sin_addr)) {

		die("InetPton failed");

	}

	printf("press space to stop\n");

	// wait for a key press
	while (!_kbhit()) {

		// fill the buffer with a simple blue fade
		for (int i = 0; i < BUFLEN; i+=3) {

			data[i  ] = 0;		// red
			data[i+1] = 0;		// green
			data[i+2] = i>>1;    // blue
		}

		// send to ESP
		if (sendto(s, (const char*)data, BUFLEN, MSG_DONTROUTE, (struct sockaddr *) &si_other, slen) == -1) {
			die("couldn't sendto");
		}
	}

	//get rid of keypress (fflush)
	_getch();

	closesocket(s);

	WSACleanup();

	return 0;
}