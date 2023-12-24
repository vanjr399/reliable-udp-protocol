/* Filename:    README
 * Author:      John Van Drasek
 * Course:      CS494 Internetworking Protocols
 * Instructor:  Binkley
 * Date:        29October2006
 * Description: This file contains all documentation for assignment1
 */


-Architecture Description

  The over all implementation of the protocol is probably very similar to TCP in the way it confirms the transfer of data between a reader and a writer.

  The function calls to create a socket, bind a socket, and close a socket are basically just wrappers around the normal socket library function calls.

  The rup_write and rup_read functions are "primitive handlers" of sorts;  I use three primitives for each of the two calls.  The rup_write and rup_read functions call their own three specific functions in order to ensure a packet(pkt) of data is sent successsfully to the intended receiver. All but one of the primitives,receiveDataPkt_FromSender(..) , contain both a sendto() and a recvfrom() function call.  The purpose of using these two function calls in pairs is to confirm anything which is sent out at each step of the message transfer/ACK transfer process.  Within each primitive, the sendto function call sends a pkt or ACK to a specified target and will only continue when it receives a confirmation ACK, or itimes out.

  There is a layer2 simulator function which uses the rand() and srand() functions to simulate the different network conditions in a real world transfer process of pkt's.  The code which handles the introduction of errors in to the pkt transfer is stuffed into one of the primitives.  Admitting this is not in any way a modular approach, I was short on time and needed to get it to work.  Further revisions of this code will separate this simulation and streamline other parts of the code.

  The listing of primitives in the rup_library, and an explanation of each follows:

int rup_open():

  The rup_open function creates a socket and returns an integer socket descriptor.


int rup_bind(int rfd, int portno):

  The rup_bind function binds a protocol to a socket type.  It takes in a file descriptor and a port number as its arguments and returns an integer return value to indicate success or failure. 


rup_error():

  The rup_error function has not been implemented.  It would have been used to streamline the error messages, and will be implemented in further revisions.


int rup_write(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The rup_write function handles a write to a specific reader and returns an integer value to indicate success or failure.  This is done by managing three primitives sendDataPkt_FromSender,pktSentSuccessfully_FromSender, and stopConfirmation_FromSender.  The SendDataPkt_FromSender will send a data pkt to a specifed reader.  The pktSentSuccessfully_FromSender function will confirm the ACK sent from the sendDataPkt_FromSender function.  The stopConfirmation_FromSender function will confirm the ACK send for the pktSentSuccessfully function and will time out if it doesn't receive any messages within a specified amount of time.


int rup_read(int rfd, void* buf, int cc, struct sockaddr_in* from):

  The rup_read function handles the reading of information coming in from any writer, or a specific writer, depending on how it is used.  This is done by managing three primitives receiveDataPkt_FromReceiver, ACK_FromReceiver, and stopConfirmation_FromReceiver.  The receiverDataPkt_FromReceiver allows a write to be accepted.  After a write has been received, the ACK_FromReceiver is called to start the ACK process with the sender to verify the data has been sent and is in good health.  Next, the stopConfirmation_FromReceiver confirms the ACK from the ACK_FromReceiver function and timesout.


int rup_layer2_simulation(int lose_packets, int dup_packets, int reo
rder_packets, int damage_packets):

  The rup_layer2_simulation function is is triggered by command line arguments to simulate a lose of packets, duplicate packet, reordering of packets, or damaging of packets in the transfer process of a packet  This function returns an integer indicating if the rand() function has set the specific error type to be applied to the current pkt in transit.  This function is called from the sendDataPkt_FromSender at the moment, but will be moved into a separate module for later revisions to isolate it from the rest of the senders code.


serverPrintFrom(from)struct sockaddr_in *from:

  The serverPrintFrom function prints a sockaddr_in struct type to stdout.


clientPrintFrom(from,fromlen)struct sockaddr_in *from:

  The clientPrintFrom function prints a sockaddr_in struct type to stdout.


int performChecksum(struct pkt* pkt):

  The performChecksum function checksum's the pkt's data portion.  It returns the integer representation of the one's in each characters binary digits added up.


int sendDataPkt_FromSender(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The sendDataPkt_FromSender function sends a pkt of data to an intended receiver and gets an ACK packet before it returns a success or failure.  This function will continue to send a pkt until the intended receiver responds.  Future revisions will allow for a timeout of this function as to make it more practical in a real world setting.  The rup_layer2_simulation function is embedded in this function to introduce errors in the transfer process and will be moved to its own module in future revisions.

   
int pktSentSuccessfully_FromSender(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The int pktSentSuccessfully_FromSender confirms the ACK was received from the previous function, sendDataPkt_FromSender, by sending an ACK to the sender.  This function returns an integer value indicating success or failure.


int stopConfirmation_FromSender(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The stopConfirmation_FromSender function will confirm the ACK was received for the previous function, pktSentSuccessfully_FromSender, by sending an ACK in return.  The function will wait for a return ACK but will never get one, and will timout after three sends.


int receiveDataPkt_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* from):

  The receiveDataPkt_FromReceiver function will loop until it receives a data pkt.  This function returns an integer value to indicate a pkt was received and the checksum indeed represents the data portion of the received pkt.


int ACK_FromReceiver(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The ACK_FromReceiver function will send an ACK to confirm the previous functino has recveived the data pkt successfully.  The function will receive an ACK in return, if transmission is successful, to indicate the ACK was received.


int stopConfirmation_FromReceiver(int rfd, void *buf, int cc, struct sockaddr_in *to):

  The stopConfirmation_FromReceiver function will send an ACK to confirm the previous function's(ACK_FromReceiver) ACK was received.  This function will never receiver a response back for this ACK sent and will timeout after three sends.


The list.h file contains datastructures to support the RUP library and the ones used are as follows:

struct pkt {
  int _checksum;
  int _id;
  struct sockaddr_in _client;
  char _client_name[NAMESIZE];
  char _msgbuf[BUFSIZE];
  int _count;
};

The _checksum contains the pkt's checksum value.  The id contains the pkt's sequence number.  The _client_name contains the name of the client in which the pkt was sent.  The _msgbuf is the data portion of the pkt. and the _count is the number of bytes the msgbuf contains and is not used in this implementation of the RUP echo server.  The _client field is not necessary for the RUP echo server, but were included to allow for flexibility in subsequent implementations of RUP.

Also included is a pktList and pktNode struct to build a link list of pktNodes in which the pkt struct is pointed to by the _data field of the pktNode struct.  This list structure is used to generate a list of packets from command line parameters and to send them to the rup_write function for transmission.

There are supporting functions here for the list structures, in addition to the general List struct, which was meant to be used for further implementation of the RUP library.  These supporting functions will remain undocumented for now and are there for reading enjoyment ;)


The rupserver.c file contains the RUP echo server implementation and has command line parameters:

executable name, port number, lose packets, duplicate packets, reorder packets, damage packets

Executable name: The name of the executable in and in this case is "rupserver"

Port number: The port to bind the rupserver to.

Lose packets: A boolean value to indicate to randomly lose packets.

Duplicate packets: A boolean value to indicate to randomly duplicate packets.

Reorder packets: A boolean value to indicate to randomly reorder packets.

Damage packets: A boolean value to indicate to randomly damage packets.


The rupclient.c file contains the RUP echo client implementation ad has the following command line parameters: 

executable name, server name, port number, client name, lose packets, duplicate packets, reorder packets, damage packets

Executable name: The name of the executable in and in this case is "rupclient"

Server name: The name of the server to connect to eg. "localhost"

Port number: The port the RUP echo server is bound to.

Client name: The instance specific name for the RUP echo client.

Lose packets: A boolean value to indicate to randomly lose packets.

Duplicate packets: A boolean value to indicate to randomly duplicate packets.

Reorder packets: A boolean value to indicate to randomly reorder packets.

Damage packets: A boolean value to indicate to randomly damage packets. 


-Test Plan
The test plan includes all varaiations of a server with no errors specified on the command line, 0 0 0 0, one to four clients and a test for each error condition on all clients run with a packet count of 100.

Suite 1: Run one client, 100 packets and one server accepting.
Desired result: Client receives pkts in order, without duplicates, and are not corrupted.
T1: No errors           0 0 0 0      PASS
T2: Lose packets        1 0 0 0      PASS 
T3: Dup packets         0 1 0 0      PASS
T4: Reorder packets     0 0 1 0      PASS 
T5: Damage packets      0 0 0 1      PASS
Example command line for each test in this suite:
./rupclient localhost 10000 <client_name> 100 X X X X

Suite 2: Run two clients, 100 packets each and one server accepting.
Desired result: Each client receives pkts in order, without duplicates, and are not corrupted.
T6: No errors           0 0 0 0      PASS
T7: Lose packets        1 0 0 0      PASS
T8: Dup packets         0 1 0 0      PASS
T9: Reorder packets     0 0 1 0      PASS
T10: Damage packets     0 0 0 1      PASS
Example command line for each test in this suite (x2, each one in a different client window):
./rupclient localhost 10000 <client_name> 100 X X X X

Suite 3: Run 3 clients, 100 packets each and one server accepting.
Desired result: Each client receives pkts in order, without duplicates, and are not corrupted.
T11: No errors          0 0 0 0      PASS
T12: Lose packets       1 0 0 0      PASS
T13: Dup packets        0 1 0 0      PASS
T14: Reorder packets    0 0 1 0      PASS
T15: Damage packets     0 0 0 1      PASS
Example command line for each test in this suite(x3, each one in a different client window):
./rupclient localhost 10000 <client_name> 100 X X X X

Suite 4: Run 4 clients, 100 packets each and one server accepting.
Desired result: Each client receives pkts in order, without duplicates, and are not corrupted.
T16: No errors          0 0 0 0      PASS 
T17: Lose packets       1 0 0 0      PASS
T18: Dup packets        0 1 0 0      PASS
T19: Reorder packets    0 0 1 0      PASS
T20: Damage packets     0 0 0 1      PASS
Example command line for each test in this suite(x4, each one in a different client window):
./rupclient localhost 10000 <client_name> 100 X X X X

Further testing would be necessary to confirm the server can introduce errors when sending pkts.  Since the tests produce so much output, test results will be included as text files rather than printed versions.  The results are a series of output lines stating the intended sequence number to be received and the packets contents.  This would need to be done for each of the clients and would result in a lot more time than I have before this drop dead date of 4pm, 6November2006.



