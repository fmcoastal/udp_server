/* dgramsrvr.c:
 *
 * Example datagram server:
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void print_usage(void)
{
   printf("UDP server - listen for a packet and maybe respond??\n");
   printf("-s w,x,y,z    Server IP Address\n");
   printf("-p <port #>   Server port\n");
//   printf("-b a.b.c.d    Bind IP Address  \n");
//   printf("-c <port>     Bind port  \n");
//   printf("-f <Data File>  Hex File containing Packet Data\n");
//   printf("-l <count>      Number of times to send Packet Data\n");
   printf("-i            Listen for incomming data\n");
//   printf("-r            Wait for Response\n");
//   printf("-m            Open Socket w/ SO_REUSEADDR (MUST Bind)\n");
//   printf("\n");
//   printf("example: \n");
   printf("./socket_server_udp -s 172.4.1.2 -p9090 \n");
//   printf("\n");
//   printf(" --> If talking to an EBB, you may need to add an ARP entry (for the fictious IP address of the EBB\n");
//   printf("  > arp -s 172.4.1.1 00:03:fb:ac:2A:20 \n");
//   printf("\n");
}






void help(void)
{
    printf("socket_server_udp   <Server_IP>\n");

}


/*
 * This function reports the error and
 * exits back to the shell:
 */
static void
bail(const char *on_what) {
    fputs(strerror(errno),stderr);
    fputs(": ",stderr);
    fputs(on_what,stderr);
    fputc('\n',stderr);
    help();
    exit(1);
} 

int
main(int argc,char **argv) {
    int option          = 0;
    int SendTime        = 0;
    int EchoInputPacket = 1;
    int z;
    char srvr_addr[32] ;
    int  srvr_port ;
    struct sockaddr_in adr_inet;/* AF_INET */
    struct sockaddr_in adr_clnt;/* AF_INET */
    socklen_t  len_inet;                /* length  */
    int s;                         /* Socket */
    char dgram[512];         /* Recv buffer */
    char dtfmt[512];   /* Date/Time Result */
    time_t td;    /* Current Time and Date */
    struct tm tm;      /* Date time values */


// set the defaults

     /* Use default address: */
     strcpy(srvr_addr, "127.0.0.23");  // Default Server Address
     srvr_port = 9090;             // Default Server Port


 


  while ((option = getopt(argc, argv,"hp:s:")) != -1) {
         switch (option) {
/*              case 'm' : SoReUseAddr=1 ;     // configure to bind w/ SO_REUSEADDR
                  break;
              case 'r' : WaitForResponse=1 ; // send only(0) or wait for response (1)
                  break;
*/              case 'p' : srvr_port = atoi(optarg); // Server IP Port
                  break;
              case 's' : strcpy(srvr_addr,optarg);     // Server Ip Address
                  break;
/*              case 'b' : strcpy(bind_addr,optarg);     // Interface to Bind To
                  break;
              case 'c' : adr_bind_port = atoi(optarg);   // Bind Port
                  break;
              case 'f' : strcpy(SndFileName,optarg);    // CLI or pkt data from File
                  break;
              case 'l' : g_SndPktDataLoop = atoi(optarg);// #of times to send File Data
                  break;
*/              case 'h' :                               // Print Help
              default:
                  print_usage();
                  exit(EXIT_FAILURE);
         }
     }
     printf("Command Line Arguments:\n");
     printf("  %16s Server IP:Port\n",srvr_addr);
     printf("              %4d Server Port\n",srvr_port);


    /*
     * Create a UDP socket to use:
     */
    s = socket(AF_INET,SOCK_DGRAM,0);
    if ( s == -1 )
        bail("socket()");

    /*
     * Create a socket address, for use
     * with bind(2):
     */
    memset(&adr_inet,0,sizeof adr_inet);
    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(srvr_port);
    adr_inet.sin_addr.s_addr =
        inet_addr(srvr_addr);

    if ( adr_inet.sin_addr.s_addr == INADDR_NONE )
        bail("bad address.");

    len_inet = sizeof adr_inet;

    printf("Server running UDP on %s port %u;\n",
        inet_ntoa(adr_inet.sin_addr),
        (unsigned)ntohs(adr_inet.sin_port));



    /*
     * Bind a address to our socket, so that
     * client programs can contact this
     * server:
     */
    z = bind(s,
        (struct sockaddr *)&adr_inet,
        len_inet);
    if ( z == -1 )
        bail("bind()");

    /*
     * Now wait for requests:
     */
    for (;;) {
        /*
         * Block until the program receives a
         * datagram at our address and port:
         */
        len_inet = sizeof adr_clnt;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr_clnt,/* Addr */
            &len_inet);    /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

      printf("Client from %s port %u;\n",
        inet_ntoa(adr_clnt.sin_addr),
        (unsigned)ntohs(adr_clnt.sin_port));


        /*
         * Process the request:
         */
        dgram[z] = 0;          /* null terminate */
        if ( !strcasecmp(dgram,"QUIT") )  
            break;                /* Quit server */

        if(SendTime != 0 )
        {     
        /*
         * Get the current date and time:
         */
        time(&td);    /* Get current time & date */
        tm = *localtime(&td);  /* Get components */

        /*
         * Format a new date and time string,
         * based upon the input format string:
         */
        strftime(dtfmt,      /* Formatted result */
            sizeof dtfmt,      /* Max result size */
            dgram,      /* Input date/time format */
            &tm);      /* Input date/time values */

        /*
         * Send the formatted result back to the
         * client program:
         */
        z = sendto(s,   /* Socket to send result */
            dtfmt, /* The datagram result to snd */
            strlen(dtfmt), /* The datagram lngth */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(2)");
        }   

     if ( EchoInputPacket != 0)
     {
        /*
         * Send the formatted result back to the
         * client program:
         */
        z = sendto(s,        /* Socket to send result */
            dgram,           /* The datagram result to snd */
            strlen(dgram),   /* The datagram lngth */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(3)");

     }



    }

    /*
     * Close the socket and exit:
     */
    close(s);
    return 0;
}
