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

#undef  ENABLE_DUMP_BUFFER

#define SPAWN_THREADS

#ifdef SPAWN_THREADS

#include <pthread.h>  // for thread Items

// the struct below is passed to the two threads.
typedef struct
{
   int done;         /* flag for termination */
   int fd;           /* file handle for logging */
   int socket;       /* Socket  */

}datablock_t;

#endif


struct sockaddr_in g_adr_clnt;/* AF_INET */

#ifdef ENABLE_DUMP_BUFFER
static void ffPrintBuff(uint8_t * buffer, int64_t bufferSize,uint8_t * Address,char * title);
#endif
int  ffReadFileToBuffer(char * filename, char **buf,int *  bufsz);

int g_verbose = 0;
#define VERBOSE(x) (g_verbose >= x)
#define VERBOSE_WAI    0x01



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
#ifdef ENABLE_DUMP_BUFFER
   printf("-d            Dump Received packet as Binary&hex Data\n");
#endif
//   printf("\n");
//   printf("example: \n");
   printf("./socket_server_udp -s 172.4.1.2 -p9090 \n");
//   printf("\n");
//   printf(" --> If talking to an EBB, you may need to add an ARP entry (for the fictious IP address of the EBB\n");
//   printf("  > arp -s 172.4.1.1 00:03:fb:ac:2A:20 \n");
//   printf("\n");
}


#ifdef SPAWN_THREADS
pthread_t g_tid[2];             // posix thread array


#endif




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




// need to use the escape key to send special charactes
// this is because getchar does not return Ctrl Sequences
#define STATE_NORMAL  0
#define STATE_ESC     1

#define COMMAND_LINE_BUFFER 256
int DoConsole(datablock_t * pdb)
{
    char c;
    int s = pdb->socket;
    int d = 0;
    int z = 0;
    int l;
    int state = STATE_NORMAL;
    int index = 0;
    char cmd[COMMAND_LINE_BUFFER];
    socklen_t  len_inet;                /* length  */

    len_inet = sizeof g_adr_clnt;
    while( d == 0 )
    {
       c = getchar();
//       printf("%c",c);   // local echo??

       switch (state) {
       case STATE_NORMAL:
           if( c == 0x1b)
           {
               state = STATE_ESC ;
           }
           else if ((c == 0x0d) || (c == 0x0a))
           {
              d = 1;
              cmd[index++] = 0x00 ;
           }
           else
           {
              //CollectCharacters into the Buffer
              cmd[index++] = c ;
           }
           break;

       case STATE_ESC:
           if( c == 0x1b)
           {
               d = 1;
           }
           else if ( c == 'q' ) // ctrl - Q
           {
              // quit
              pdb->done = 0;
           }
           else if ( c == 'u' ) // ctrl - u
           {
              printf("ctrl-u\n");
           }
           state = STATE_NORMAL;

           break;
       } // end state switch

   } //end while  d == 0

       l = strlen(cmd)+1;  // lenth of command
       printf("Send '%s' (%d)  to client at %s port %u;\n",cmd,l,
                                 inet_ntoa(g_adr_clnt.sin_addr),
                                 (unsigned)ntohs(g_adr_clnt.sin_port));

        /*
         * Send the formatted result back to the
         * client program:
         */
        z = sendto(s,   /* Socket to send result */
            cmd, /* The datagram result to snd */
            l, /* The datagram lngth */
            0,               /* Flags: no options */
            (struct sockaddr *)&g_adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(2)");


    return 0;
}



#ifdef SPAWN_THREADS


// TX THREAD
int tx(void* arg)
{
    datablock_t * pdb =(datablock_t *)arg;
//    char c;
//    char * pc = &c ;
//    int r;

    printf("Starting Tx Thread\n");
    printf("  Serial Handle: %d\n",pdb->fd);
    while( pdb->done == 0 )
    {
#ifdef NO_COMMAND_FUNCTION
       usleep(10000);    // let the OS have the machine if noting to go out.
#else
       DoConsole(arg );
#endif
    }
    printf("Ending Tx Thread\n");
    return 1;
}


// RX_THREAD
int rx(void* arg)
{
    datablock_t * pdb =(datablock_t *)arg;
    printf("Starting Rx Thread\n");

//    int z;
//    unsigned int sz;
//    struct sockaddr_in adr;            /* AF_INET */
//    char dgram[512];               /* Recv buffer */
//    int s = pdb->socket;

    while( pdb->done == 0) // this will be set by teh back door
    {

#ifdef RECEIVE_LOOP
zzzzzzzzzzzzzzzzzzzzzzzzzzzzz
        /*
         * Wait for a response:
         */
        sz = sizeof adr;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr,     /* Addr */
            &sz);             /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

        dgram[z] = 0;          /* null terminate */

        /*
         * Report Result:
         */
        printf("\nResult from %s port %u :'%s'\n",
            inet_ntoa(adr.sin_addr),
            (unsigned)ntohs(adr.sin_port),
            dgram);
        printf("Enter format string :");
        fflush(stdout);i
#endif


    }
    printf("End of  Rx Thread\n");
    return 1;
}

// assign functions to worker threads created
void *WorkerThread(void *arg)
{
pthread_t id = pthread_self();

   if(pthread_equal(id,g_tid[0]))
   {
       // tx
       tx(arg);
   }
   else
  {
      // rx
      rx(arg);
  }
  return NULL;

}
#endif

int g_dump_received_packet = 0;


int
main(int argc,char **argv) {
    int option          = 0;
    int SendTime        = 0;
    int EchoInputPacket = 1;
    int z;
    char srvr_addr[32] ;
    int  srvr_port ;
    struct sockaddr_in adr_inet;/* AF_INET */
    socklen_t  len_inet;                /* length  */
    int s;                         /* Socket */
    char dgram[512];         /* Recv buffer */
    char dtfmt[512];   /* Date/Time Result */
    time_t td;    /* Current Time and Date */
    struct tm tm;      /* Date time values */
// for time stamps
   time_t rawtime ;  
   struct tm * timeinfo;

#ifdef SPAWN_THREADS
    int i;
    int err;
    datablock_t db;

// Initialize Default Values
    memset((void*)&db,0,sizeof(datablock_t));


#endif

// set the defaults

     /* Use default address: */
     strcpy(srvr_addr, "127.0.0.23");  // Default Server Address
     srvr_port = 9090;             // Default Server Port
     g_dump_received_packet = 0;       // default to not dump rx packet

  while ((option = getopt(argc, argv,"dhp:s:")) != -1) {
         switch (option) {
              case 'd' : g_dump_received_packet=1 ;  // configure to dump Rx Packet
                  break;
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

// do we need Bind Address???

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


#ifdef SPAWN_THREADS

//  Launch Tx and Rx Thread
    db.done   = 0;  // Init Flag to run.
    db.fd     = 0;  // Read/Write File Handle.
    db.socket = s;  // pass the socket handle to the THreads

    while(i < 2)
    {
        err = pthread_create(&(g_tid[i]), NULL, &WorkerThread, &db);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
        i++;
    }

#endif


    /*
     * Now wait for requests:
     */
    for (;;) {
        /*
         * Block until the program receives a
         * datagram at our address and port:
         */
        len_inet = sizeof g_adr_clnt;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&g_adr_clnt,/* Addr */
            &len_inet);    /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

/*
      printf("Client from %s port %u;\n",
        inet_ntoa(g_adr_clnt.sin_addr),
        (unsigned)ntohs(g_adr_clnt.sin_port));
*/

        if(g_dump_received_packet != 0)
        {
#ifdef ENABLE_DUMP_BUFFER
            ffPrintBuff((uint8_t *) dgram, (int32_t) z, 0 , "Packet Data:\n" );
#else
            printf(" need to re-compile to enable Printing Packet Data\n");
#endif
        }


        /*
         * Process the request:
         */
        dgram[z] = 0;          /* null terminate */
        if ( !strcasecmp(dgram,"QUIT") )  
            break;                /* Quit server */

        time (&rawtime);
        timeinfo = localtime (&rawtime);
        //printf("%d %d:%d:%d %s:%u %s \n",timeinfo.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,
        printf("s%d:%d:%d %s:%u %s \n",timeinfo->tm_hour,timeinfo->tm_min
                               ,timeinfo->tm_sec
                               , inet_ntoa(g_adr_clnt.sin_addr)
                               ,(unsigned)ntohs(g_adr_clnt.sin_port)
                               ,dgram);  



    




#if 0
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
            (struct sockaddr *)&g_adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(2)");
        }
#endif   
#if 0
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
            (struct sockaddr *)&g_adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(3)");

     }
#endif


    } // end For loop

    /*
     * Close the socket and exit:
     */
    close(s);
    return 0;
}

#ifdef ENABLE_DUMP_BUFFER
#define LLU  long long unsigned
#define fsprint printf

static void ffPrintBuff(uint8_t * buffer, int64_t bufferSize,uint8_t * Address    ,char * title)
{
    uint8_t * tmpptr0  = buffer;
    uint8_t * tmpptr1  = tmpptr0;
    int64_t  i          = 0 ;
    int64_t  m          = 0 ;
    int64_t  n          = 0 ;
    int64_t  j          = 0 ;
    int64_t  PrintCount = 0 ;   // used as counter to denote when to print \    nadderss
    int64_t  BlankCnt   = 0 ;

    if( title != NULL)   fsprint("\n%s\n",title);
#if 1

    fsprint("  buf:%p  sz:0x%016llx  add:%p\n",buffer,(LLU)bufferSize,Address);

#endif
    if( buffer == NULL)
    {
        printf("Cannot access address %p\n",buffer);
        return;
    }


    // align the lead
    BlankCnt = (unsigned long)Address & 0x000000000f;

    // print the lead
    if( BlankCnt != 0)  // if 0 jump to main body
    {
        for ( PrintCount = 0 ; PrintCount < BlankCnt ; PrintCount++ )
        {
            if( PrintCount == 0) // space between fields
            {
                fsprint("\n%016x",(unsigned)((unsigned long)Address & ~0x000000000f));
                tmpptr1 = tmpptr0;
            }
            if( (PrintCount % 8) == 0)
            {
                fsprint(" ");
            }
            fsprint("   ");
        }
        PrintCount--;  // remove last increment of printcount
        // print PrintCount data
        for ( m = 0  ; (PrintCount < 0xf) && (i < bufferSize); i++, m++,PrintCount++)
        {
            if(PrintCount % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" %02x",(unsigned char)(*tmpptr0++));
            Address++;
        }

        // special case here when count is less than one line and not starting at zero
        if ( i == bufferSize)
        {
            // print out the last space
            for (      ; (PrintCount < 0x0f) ; PrintCount++ )
            {
                if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint("   ");
            }
            // print PrintCount text space
            for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
            {
                if( PrintCount == 0)   // space between fields
                {
                    fsprint(" ");
                }
                else if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint(" ");
            }
            // print PrintCount characters
            for( n = 0 ; (n < m) ; n++)
            {
                if( n == 0 ) printf(" ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
            printf("\n");
            return;
        } // end i == bufferSize

        // print PrintCount text space
        for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
        {
            if( PrintCount == 0)   // space between fields
            {
                fsprint(" ");
            }
            else if( PrintCount  % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" ");
        }
        // print PrintCount characters
        n = 0;
        for( n = 0 ; (PrintCount <= 0xf) && (n < m) ; n++,PrintCount++)
        {
            if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                fsprint("%c",*tmpptr1);
            else
                fsprint(".");
            tmpptr1++;
        }
    }

    // print the body
    PrintCount = 0;
    for (   ; i < bufferSize ; i++)
    {
        if( PrintCount == 0 )
        {
            fsprint("\n%016llx",((LLU)Address & ~0x0f));
            tmpptr1 = tmpptr0;
        }
        if(PrintCount % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint(" %02x",(unsigned char)(*tmpptr0++));
        Address++;
        PrintCount ++;
        if( PrintCount  > 0x0f)
        {
            PrintCount = 0;
            for( j = 0 ; j < 16 ; j++)
            {
                if( j == 0 ) printf("  ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
        }
    }

    // print out the last space
    m = PrintCount;
    for (      ; (PrintCount <= 0x0f) ; PrintCount++ )
    {
        if( PrintCount  % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint("   ");
    }

    // print PrintCount characters
    for( n = 0 ; (n < m) ; n++)
    {
        if( n == 0 ) printf("  ");
        if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
            fsprint("%c",*tmpptr1);
        else
            fsprint(".");
        tmpptr1++;
    }
    fsprint("\n");
}
#endif

