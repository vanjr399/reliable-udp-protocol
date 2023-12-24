/* Filename:    rup.h
 * Author:      John Van Drasek
 * Date:        29 October 2006
 * Description: Header file for RUP protocol
 */

// Uncomment to compile windows version
//#define _WIN32_

//////////////////////////////////////////////////////////////////////////////
// Usage //                                                                 //
///////////                                                                 //
//  Server: rup_open, rup_bind, loop on rup_read and respond                //
//          with rup_write, then rup_close.                                 //
//  Client: rup_open, rup_write and rup_read to get response                //
//          then rup_close.                                                 //
//////////////////////////////////////////////////////////////////////////////

#ifndef __RUP_H
#define __RUP_H

// Platform check
#ifdef _WIN32_

// Windows includes
#include <winsock2.h>
#include <iostream>
using namespace std;

#else

// Linux includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif

// Defines
#define ACK 'Z'
#define FINALACK 'X'
#define BUFSIZE 1024
#define COMMANDSIZE 20
#define NAMESIZE 20
#define PASSWORDSIZE 10
#define SERVERLOGINPORT 10000
#define SERVERCHATPORT 10001
#define CLIENTCHATPORT 10002
#define ADDRESSSIZE 16

// FIXME: This needs to be changed to a buffer with an identifier and length for
//        this to be more generic
// Packet struct
struct pkt
{
  int _checksum;
  int _id;
  char _command[COMMANDSIZE];
  struct sockaddr_in _client;
  char _client_name[NAMESIZE];
  char _client_password[PASSWORDSIZE];
  char _msgbuf[BUFSIZE];
  char _ackvar;
};

//
// rup_open
//
// Description: Open a socket and return an ingeger file descriptor which
//               will be used by all other RUP calls.
//
// Input: NA
// Output: int sock - Return a descriptor to be used in all other RUP calls.
int rup_open();

//
// rup_bind
//
// Description: A file descriptor and a portnumber are used to bind UDP to a socket.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: int portno - A port number to bind the UDP protocol to.
// Output: int - Returns 0 on success and -1 on failure.
int rup_bind(int rfd, int portno);

//
// rup_close
//
// Description: close a socket using the incoming file descriptor.
//
// Input: int rfd - A valid RUP file descriptor.
// Output: NA
void rup_close(int rfd);

//
// rup_write
//
// Description: Write data to a remote ip address and port number.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          ip address and port number of the local machine.
// Output: int ret - Returns 1 on success and 0 on failure.
int rup_write(int rfd, void* buf, int cc, struct sockaddr_in* to);

//
// rup_read
//
// Description: Read data to from remote ip address and port number.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* from - A socket address structure containing
//          the remote ip address and port number.
// Output: int ret - Returns 1 on success and 0 on failure.
//
int rup_read(int rfd, void* buf, int cc, struct sockaddr_in* from);

//
// createPkt
//
// Description: Create a new pkt and return a pointer to it.
//
// Input: int cpId - An id number the new pkt is to be assigned to for network code
//          to determine if the incoming pkt is out of order.
// Input: char* cpCommand - A text command used to run specific functions on the server
// Input: struct sockaddr_in* cpClient - A socket address structure containing
//          ip address and port number.
// Input: char* cpCName - A char pointer containing the user name
// Input: char* cpCPassword - A char pointer containing user password
// Input: char* cpMsgbuf - A char pointer containing some text data
// Input: int cpCount - A total number of pkts being sent
// Output: struct pkt* - The pkt pointer of the newly created pkt is returned.
struct pkt* createPkt(int cpId, char* cpCommand, struct sockaddr_in* cpClient, char* cpCName, char* cpCPassword, char* cpMsgbuf, char cpAckvar);

//
// performChecksum
//
// Description: Compute a checksum for on the pkt _msgbuf data.
//
// Input: struct pkt* p - A pointer to the pkt to perform the checksum.
// Output: int result - Returns the integer value of the checksum computed.
int performChecksum(struct pkt* p);

#endif

