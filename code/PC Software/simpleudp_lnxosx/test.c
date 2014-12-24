#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Defines
#define LIGHTS        "192.168.1.230"    // Address of Node
#define BUFLEN        ( 450  )            // Length of data to send to LED strips, 150 LEDs, three byte values per LED , 450
#define PORT        (40002)                // The port the ESP is listening on

void changemode(int dir);

// short cut exit routine
void die(const char *str)
{
    fprintf(stderr, "error: %s\n",str);

    changemode(0);

    // exit with error
    exit(1);
}

void changemode(int dir)
{
  static struct termios oldt, newt;

  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

int _kbhit (void)
{
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);

}


int main(void)
{
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other);
    unsigned char data[BUFLEN];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket create error");
    }

    // setup the target port/address
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr=inet_addr( LIGHTS );

    printf("press space to stop\n");

    changemode(1);
    // wait for a key press
    while (!_kbhit()) {

        // fill the buffer with a simple green fade
                // each LED is three bytes, RGB, so the 2nd led is (index*3)
        for (int i = 0; i < BUFLEN; i+=3) {

            data[i  ] = 0;        // red
            data[i+1] = i>>1;        // green
            data[i+2] = 0;    // blue
        }

        // send to ESP
        if (sendto(s, (const char*)data, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1) {
            die("couldn't sendto");
        }
    }

    //get rid of keypress (fflush)
    getchar();
    changemode(0);

    close(s);

    return 0;
}
