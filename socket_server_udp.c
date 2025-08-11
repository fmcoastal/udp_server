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



#undef ENABLE_DUMP_BUFFER      // prints raw contents of packet received

#undef SEND_TIME_TO_CLIENT     // will send this machines time of day as
                               // part of what happens when a packet is 
                               // recieived
#undef ECHO_INPUT_PACKET       // echo input pact back to sender
#undef RUN_INPUT_AS_LINUX_CMD  // execute incomming packet as a Linux Cmd 
                               // Line argument  




#define SPAWN_THREADS          // used to check compile problems

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifdef SPAWN_THREADS
#include <pthread.h>  // for thread Items

// the struct below is passed to the two threads.
typedef struct
{
   int done;         /* flag for termination */
   int fd;           /* file handle for logging */
   int socket;       /* Socket  */

}datablock_t;

pthread_t g_tid[2];             // posix thread array

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
        fflush(stdout);
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
    int         option          = 0;
#ifdef ECHO_INPUT_PACKET
    int         EchoInputPacket = 1;
#endif
    int         z;
    char        srvr_addr[32] ;
    int         srvr_udp_port ;
    struct      sockaddr_in adr_inet; /* AF_INET */
    socklen_t   len_inet;             /* length  */
    int         s;                    /* Socket */
    char        dgram[512];           /* Recv buffer */
#ifdef SEND_TIME_TO_CLIENT
    int         SendTime        = 0;
    char        dtfmt[512];           /* Date/Time Result */
    time_t      td;                   /* Current Time and Date */
#endif
    struct tm   tm;                   /* Date time values */
// for time stamps
   time_t       rawtime ;  
   struct tm    *timeinfo = &tm;



     
#ifdef BIND_TO_NETDEV 
    int   flag_bind_to_local_interface = 0;    // flag not to bind to local interface
    const char* local_interface_name = "eth0"; // Replace with your desired interface
#endif

#ifdef SPAWN_THREADS
    int         i;
    int         err;
    datablock_t db;

// Initialize Default Values
    memset((void*)&db,0,sizeof(datablock_t));
#endif

// set the defaults

     /* Use default address: */
     strcpy(srvr_addr, "127.0.0.23");  // Default Server Address
     srvr_udp_port          = 9090;    // Default Server Port
     g_dump_received_packet = 0;       // default to not dump rx packet

  while ((option = getopt(argc, argv,"dhp:s:")) != -1) {
         switch (option) {
              case 'd' : g_dump_received_packet=1 ;  // configure to dump Rx Packet
                  break;
/*              case 'm' : SoReUseAddr=1 ;                // configure to bind w/ SO_REUSEADDR
                  break;
              case 'r' : WaitForResponse=1 ;              // send only(0) or wait for response (1)
                  break;
*/ 
              case 'p' : srvr_udp_port = atoi(optarg);    // Server IP Port
                  break;
              case 's' : strcpy(srvr_addr,optarg);        // Server Ip Address
                  break;
#ifdef BIND_TO_IP_ADDR
              case 'i' : strcpy(bind_to_ipaddr,optarg);   // local ip address to bind to
                  break;
#endif
/*              case 'c' : adr_bind_port = atoi(optarg);  // Bind Port
                  break;
              case 'f' : strcpy(SndFileName,optarg);      // CLI or pkt data from File
                  break;
              case 'l' : g_SndPktDataLoop = atoi(optarg); // #of times to send File Data
                  break;
*/
#ifdef BIND_TO_NETDEV 
              case 'n' : strcpy(bind_to_netdev,optarg);   // local netdev to bind to 
                         flag_bind_to_local_interface = 1;
                  break;
#endif 


              case 'h' :                               // Print Help
              default:
                  print_usage();
                  exit(EXIT_FAILURE);
         }
     }
     printf("Command Line Arguments:\n");
     printf("  %16s Server IP:Port\n",srvr_addr);
     printf("              %4d Server Port\n",srvr_udp_port);

// do we need Bind Address???

    /*
     * Create a UDP socket to use:
     */
    s = socket(AF_INET,SOCK_DGRAM,0);
    if ( s == -1 )
        bail("socket()");

#ifdef  BIND_TO_NETDEV
// AI: in c how do you bind to a network interface

// In C, you can bind a socket to a specific network interface using the 
// SO_BINDTODEVICE socket option with setsockopt(). This allows you to 
// restrict a socket's communication to a particular interface, rather 
// than allowing it to bind to any available interface.
//
// Here's how to do it: Create a socket.


    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Or SOCK_DGRAM for UDP
    if (sockfd < 0) {
        perror("socket");
        // Handle error
    }

// Prepare the interface name.
// You need the name of the network interface you want to bind to (e.g., 
// "eth0", "wlan0", "enp0s3").


    const char* interface_name = "eth0"; // Replace with your desired interface

// Use SO_BINDTODEVICE with setsockopt().


    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
        perror("setsockopt SO_BINDTODEVICE");
        // Handle error
    }

// SOL_SOCKET specifies the socket-level options.
// SO_BINDTODEVICE is the option to bind the socket to a specific device.
// ifr is a struct ifreq that contains the interface name.
// Proceed with bind() (optional but common).
// After binding to the device, you can still use bind() to associate a local
//  IP address and port with the socket. If you want to bind to a specific IP 
//  address on that interface, you would specify it in the sockaddr_in 
//  structure. If you want to bind to any IP address on that interface, 
//  you can use INADDR_ANY.


    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Or a specific IP on the interface
    serv_addr.sin_port = htons(8080); // Your desired port

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        // Handle error
    }

// Important Notes:
//   Permissions: Binding to a specific device often requires elevated
//     privileges (e.g., root or CAP_NET_RAW capability), especially 
//     for raw sockets.
//   Availability: The specified interface must exist and be up.
//   IPv6: For IPv6, you would use AF_INET6 and sockaddr_in6, and the 
//     sin6_scope_id field can be used with the interface index 
//     obtained from if_nametoindex().

#endif



#ifdef BIND_TO_IP_ADDR
//AI: in C how do you bind to an IP address
    
// In C, to bind a socket to a specific IP address, you use the bind() 
//   function. This function associates a local address (IP address and 
//   port number) with an unbound socket.
// Here's a breakdown of the process: Include necessary headers.


//    #include <sys/socket.h> // For socket, bind, etc.
//    #include <netinet/in.h> // For sockaddr_in, INADDR_ANY, etc.
//    #include <arpa/inet.h>  // For inet_addr
//    #include <stdio.h>      // For perror
//    #include <stdlib.h>     // For exit

// Create a socket.


    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // For TCP socket
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

// Prepare the sockaddr_in structure: This structure holds the address 
// information.

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(8080); // Port number (e.g., 8080), convert to network byte order
    
    // To bind to a specific IP address:
    server_addr.sin_addr.s_addr = inet_addr("192.168.1.100"); // Replace with your desired IP
    
    // To bind to all available IP addresses on the host:
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

// Call the bind() function.


    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
// Explanation:
//
//       socket(AF_INET, SOCK_STREAM, 0): Creates a TCP socket for IPv4 communication.
//
// sockaddr_in: A structure used to define socket addresses for the 
//             IPv4 family.
// sin_family: Set to AF_INET for IPv4.
// sin_port: The port number you want to bind to. htons() converts the 
//             host byte order to network byte order.
// sin_addr.s_addr: The IP address.
// inet_addr("IP_ADDRESS_STRING"): Converts a dotted-decimal IP string 
//             to a network byte order integer.
// INADDR_ANY: A special constant that binds the socket to all available 
//             network interfaces on the machine. htonl() converts the 
//             host byte order to network byte order for INADDR_ANY.
// bind(sockfd, (const struct sockaddr *)&server_addr, 
//             sizeof(server_addr)): Attempts to bind the sockfd to 
//             the address specified in server_addr. The (const struct 
//             sockaddr *) cast is necessary because bind expects a 
//             generic sockaddr pointer.
// After successfully binding, you can proceed with other socket operations
//             like listen() for a server or connect() for a client.

#endif
    /*
     * Create a socket address, for use
     * with bind(2):
     */
    memset(&adr_inet,0,sizeof adr_inet);
    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(srvr_udp_port);
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

    i = 0;
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

#ifdef RUN_INPUT_AS_LINUX_CMD

//   system() 
// int status = system("gzip foo");
{
  FILE *fp;
  char path[1035];

  /* Open the command for reading. */
//  fp = popen("/bin/ls /etc/", "r");
  fp = popen( dgram , "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path), fp) != NULL) {
    
        // print results locally
        printf("%s", path);
        /*
         * Send the formatted result back to the
         * client program:
         */
        z = sendto(s,        /* Socket to send result */
            path,           /* The datagram result to snd */
            strlen(path),   /* The datagram lngth */
            0,               /* Flags: no options */
            (struct sockaddr *)&g_adr_clnt,/* addr */
            len_inet);  /* Client address length */
        if ( z < 0 )
            bail("sendto(3)");

        }  // end while

        /* close */
        pclose(fp);

}
#endif

#ifdef SEND_TIME_TO_CLIENT
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
#ifdef ECHO_INPUT_PACKET
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

