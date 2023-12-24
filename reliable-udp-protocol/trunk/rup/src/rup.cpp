// Filename:    rup.cpp
// Author:      John Van Drasek
// Date:        29 October 2006
// Description: Source file for RUP protocol
//
#include "../include/rup.h"

// Forward declarations
int sendDataPkt_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to);
int pktSentSuccessfully_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to);
int stopConfirmation_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to);
int receiveDataPkt_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* from);
int ACK_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* to);
int stopConfirmation_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* to);

//
// rup_open
//
// Description: Open a socket and return an ingeger file descriptor which
//               will be used by all other RUP calls.
//
// Input: NA
// Output: int sock - Return a descriptor to be used in all other RUP calls.
int rup_open()
{
	// Variable declarations
	int sock;

#ifdef _WIN32_
	// Winsock requres a call to WSAStartup before any calls to socket
	WSAData msg;
	int result;
	if ((result = WSAStartup(MAKEWORD(1, 1), &msg)) != 0)
	{
		printf("UDP Error: WSAStartup() failed!\n");
		exit(0);
	}
#endif

	// Variable assignments
	sock = socket(AF_INET, SOCK_DGRAM, 0);

#ifdef _WIN32_
	// Example of how the preferred method is according to online resources
	if(sock == INVALID_SOCKET)
#else
	if ( sock < 0 )
#endif
	{
		printf("UDP Error: socket() call in rup_open\n");
		exit(0);
	}
	return sock;
}

//
// rup_bind
//
// Description: A file descriptor and a portnumber are used to bind UDP to a socket.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: int portno - A port number to bind the UDP protocol to.
// Output: int - Returns 0 on success and -1 on failure.
int rup_bind(int rfd, int portno)
{
	// Variable declarations
	int ret;
	struct sockaddr_in server;

	// Variable assignments
	ret = 0;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;   /* ok from any machine */
	server.sin_port = htons((unsigned short)portno);      /* specific port */

	// Bind protocol to socket
	if (bind(rfd, (struct sockaddr*)(&server), sizeof(server)))
	{
		printf("binding udp socket\n");
		ret = -1; 
	}
	return ret;
}

//
// rup_close
//
// Description: close a socket using the incoming file descriptor.
//
// Input: int rfd - A valid RUP file descriptor.
// Output: NA
void rup_close(int rfd)
{
#ifdef _WIN32_
	rfd = 0;
	WSACleanup();
#else
	close(rfd);
#endif
}

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
int rup_write(int rfd, void* buf, int cc, struct sockaddr_in* to)
{
	// Variable declarations
	int ret;

	// Variable assignments
	ret = 0;

	// send pkt to receiver
	//   A true return value indicates to the sender
	//   that the receiver has received the pkt
	if(sendDataPkt_FromSender(rfd, buf, cc, to))
	{
		//printf("ret = %d\n", ret);
		//printf("sendDataPkt_FromSender has succeded!!!\n");
		// send pkt sent successfully ACK to receiver
		//   A true return value indicates that the sender
		//   has told the receiver it knows the pkt arrived
		//   at the receiver successfully
		if(pktSentSuccessfully_FromSender(rfd, buf, cc, to))
		{
			//printf("pktSentSuccessfully_FromSender has succeded!!!\n");
			// send stop ack to receiver
			//   A true return value indicates the reveiver is aware
			//   the sender has sent pkt successfully, and has received
			//   an ACK from the receiver for the pkt being sent
			//   successfully.
			//
			// At this point the sender knows there is no need to
			//   keep sending ACK's or receiving ACK's.  The sender
			//   can timeout and leave function without getting a
			//   response back from receiver without the possiblity
			//   of the data pkt not being delivered..
			if(stopConfirmation_FromSender(rfd, buf, cc, to))
			{
				//printf("stopConfirmation_FromSender has succeded!!!\n");
				ret = 1;
			}
			else
			{
				//printf("stopConfirmation_FromSender has FAILED\n");
				ret = 1;
			}
		}
		else
		{
			//printf("pktSentSuccessfully_FromSender has FAILED\n");
			ret = 0;
		}
	}
	else
	{
		//printf("sendDataPkt_FromSender has FAILED\n");
		ret = 0;
	}
	return ret;
}

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
int rup_read(int rfd, void* buf, int cc, struct sockaddr_in* from)
{
	// Variable declarations
	int ret;

	// Variable assignments
	ret = 0;

	while(ret != 1)
	{
		// wait to receive the data pkt from sender
		if(receiveDataPkt_FromReceiver(rfd, buf, cc, from))
		{
			//printf("receiveDataPkt_FromReceiver has succeded!!!\n");
			// send ACK to sender
			//   A true return value indicates the receiver knows the
			//   sender knows the original data pkt was delivered
			//   to the receiver successfully  
			if(ACK_FromReceiver(rfd, buf, cc, from))
			{
				//printf("ACK_FromReceiver has succeded!!!\n");
				// send stop confirmation to sender
				//   A true return value indicates the sender has confirmed
				//   the ack sending can be stopped.  Original data pkt has
				//   been delivered and acks have been sent and received by
				//   both receiver and sender. 
				if(stopConfirmation_FromReceiver(rfd, buf, cc, from))
				{
					//printf("stopConfirmation_FromReceiver has succeded!!!\n");
					ret = 1;
				}
				else
				{
					//printf("stopConfirmation_FromReceiver has FAILED\n");
					ret = 0;
				}
			}
			else
			{
				//printf("ACK_FromReceiver has FAILED\n");
				ret = 0;
			}
		}
		else
		{
			//printf("ReceiveDataPkt_FromReceiver has FAILED\n");
			ret = 0;
		}
	}
	return ret;
}

//
// performChecksum
//
// Description: Compute a checksum for on the pkt _msgbuf data.
//
// Input: struct pkt* p - A pointer to the pkt to perform the checksum.
// Output: int result - Returns the integer value of the checksum computed.
int performChecksum(struct pkt* p)
{
	// Variable declarations
	int i, j, result;
	char c;
	struct pkt* curPkt;

	// Variable assignments
	i = 0;
	j = 0;
	result = 0;
	curPkt = p;

	// loop through the max length of the payload, BUFSIZE.
	for(i = 0;i < BUFSIZE;++i)
	{
		// assign the temp char, c, to each char in msgbuf.
		c = curPkt->_msgbuf[i];

		// loop through the char bit by bit
		for(j = 0;j < ((sizeof(char))*8);++j)
		{
			// if the AND process, &, results in a one(1), then add
			//   one to the result
			if(c & (1<<j))
			{
				result++;
			}
		}
	}
	return result;
}

//
// sendDataPkt_FromSender
//
// Description: Send a pkt to a remote system and receive an ACK to confirm
//                the data was received.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          ip address and port number of the local machine.
// Output: int sendPkt - Returns 1 on success and 0 on failure.
int sendDataPkt_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to)
{
	// Variable declarations
	int rc, sendPkt, selret, pktID, inChecksum, sendingPkt;
	unsigned int fromlen;
	char* tns;
	char* fns;
	struct pkt inBuf;
	struct sockaddr_in from;
	struct timeval tval;
	fd_set rfds;

	// Variable assignments
	sendPkt = 0;
	selret = 0;
	pktID = ((struct pkt*)buf)->_id;
	sendingPkt = 1;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)&inBuf,0,sizeof(struct pkt));

	// getting the to address into a variable
	tns = inet_ntoa(to->sin_addr);

	while(sendPkt != 1)
	{
		//printf("sendDataPkt_FromSender(): sendPkt = %d\n", sendPkt);
		if(sendingPkt)
		{
			// Send a pkt to reader
			if( sendto(rfd, (char*)buf, cc, 0, (struct sockaddr*)to, fromlen) < 0 )
			{
				printf("ERROR in sendDataPkt_FromSender() - sendto()");
				exit(0);
			}

			tval.tv_sec = 0;
			tval.tv_usec = 100000;

			// set socket for select call
			FD_ZERO(&rfds); 
			FD_SET(rfd,&rfds);

			// Set timer for reading on the socket
			selret = select(rfd+1,&rfds,NULL,NULL,&tval);

			if(selret != 0)
			{
				// Wait for ACK
#ifdef _WIN32_
				if((rc=recvfrom(rfd, (char*)&inBuf, sizeof(struct pkt), 0, (struct sockaddr*)(&from), (int*)&fromlen)) < 0 )
#else
				if((rc=recvfrom(rfd, &inBuf, sizeof(struct pkt), 0, (struct sockaddr*)(&from), &fromlen)) < 0 )
#endif
				{
					printf("ERROR in sendDataPkt_FromSender().\n");
					printf("Write error: errno %d\n",errno);
					printf("reading datagram");
					exit(0);
				}

				// assign checksum to checksum pkt
				inChecksum = performChecksum(((struct pkt*)(&inBuf)));

				// getting the from address into a variable
				fns = inet_ntoa(from.sin_addr);

				// Check pkt for ACK
				if((inBuf._ackvar == ACK)&&(from.sin_port == to->sin_port)&&(pktID == inBuf._id)&&(!(strcmp(fns,tns))))
				{
					//printf("sendDataPkt_FromSender() - setting return value to 1\n");
					//printf("ackvar = %c\n", inBuf._ackvar);
					sendPkt = 1;
				}
				else
				{
					//printf("received someone else's data pkt and disregarding it\n");
					sendPkt = 0;
				}
			}
			else
			{
				//printf("No pkt received(sendDataPkt_FromSender)...select returned %d\n",selret);
				sendPkt = 0;
			}
		}
		else
		{
			sendingPkt = 1;
		}
	}
	return sendPkt;
}

//
// pktSentSuccessfully_FromSender
//
// Description: Send an ACK to notify the receiver that the sender
//                is aware the pkt was received at the receiver successfully.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          ip address and port number of the local machine.
// Output: int pktSent - Returns 1 on success and 0 on failure.
int pktSentSuccessfully_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to)
{
	// Variable declarations
	int rc, pktSent, selret, pktID, inChecksum;
	unsigned int fromlen;
	char* tns;
	char* fns;
	struct pkt* outPktSentAck;
	struct pkt inPktSentAck;
	struct sockaddr_in from;
	struct timeval tval;
	fd_set rfds;

	// Variable assignments
	pktSent = 0;
	selret = 0;
	pktID = ((struct pkt*)buf)->_id;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)&inPktSentAck,0,sizeof(struct pkt));

	// getting the to address into a variable
	tns = inet_ntoa(to->sin_addr);

	char command[1] = "";
	char buffer[1] =  "";
	// Creating ACK pkt
	outPktSentAck = createPkt(((struct pkt*)buf)->_id,&command[0], &((struct pkt*)buf)->_client, ((struct pkt*)buf)->_client_name,((struct pkt*)buf)->_client_password,&buffer[0],ACK);

	// assign checksum to checksum pkt
	outPktSentAck->_checksum = performChecksum(outPktSentAck);

	while(pktSent != 1)
	{
		// Send a pkt to reader
		if( sendto(rfd, (char*)outPktSentAck, cc, 0, (struct sockaddr*)to, fromlen) < 0 ) {
			printf("ERROR in pktSentSuccessfully_FromSender() - sendto()");
			exit(0);
		}

		tval.tv_sec = 0;
		tval.tv_usec = 100000;

		// set socket for select call
		FD_ZERO(&rfds); 
		FD_SET(rfd,&rfds);

		// Set timer for reading on the socket
		selret = select(rfd+1,&rfds,NULL,NULL,&tval);

		if(selret != 0)
		{
			// Wait for pktSentAck response
#ifdef _WIN32_
			if ((rc=recvfrom(rfd, (char*)&inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), (int*)&fromlen)) < 0 )
#else
			if ((rc=recvfrom(rfd, &inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), &fromlen)) < 0 )
#endif
			{
				printf("ERROR in pktSentSuccessfully_FromSender().\n");
				printf("Write error: errno %d\n",errno);
				printf("reading datagram");
				exit(0);
			}

			// assign checksum to checksum pkt
			inChecksum = performChecksum(((struct pkt*)(&inPktSentAck)));

			// getting the from address into a variable
			fns = inet_ntoa(from.sin_addr);

			// Check pkt for ACK
			if((inPktSentAck._ackvar == ACK)&&(pktID == inPktSentAck._id)&&(from.sin_port == to->sin_port)&&(!(strcmp(fns,tns))))
			{
				pktSent = 1;
			}
			else
			{
				//printf("fs2-received someone else's pkt and disregarding it\n");
				pktSent = 0;
			}
		}
		else
		{
			//printf("No ack pkt received(pktSentSuccessfully_FromSender)...select returned %d\n",selret);
			pktSent = 0;
		}
	}
	delete outPktSentAck;
	return pktSent;
}

//
// stopConfirmation_FromSender
//
// Description: Send a final ACK to notify the receiver that the sender
//                is aware the data has been received.
//               At this point the sender knows there is no need to
//                keep sending ACK's or receiving ACK's.  The sender
//                can timeout and leave function without getting a
//                response back from receiver without the possiblity
//                of the data pkt not being delivered.. 
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          ip address and port number of the local machine.
// Output: int pktSent - Returns 1 on success and 0 on failure.
int stopConfirmation_FromSender(int rfd, void* buf, int cc, struct sockaddr_in* to)
{
	// Variable declarations
	int rc, numtimeouts, pktSent, selret, pktID, inChecksum;
	unsigned int fromlen;
	char* tns;
	char* fns;
	struct pkt* outPktSentAck;
	struct pkt inPktSentAck;
	struct sockaddr_in from;
	struct timeval tval;
	fd_set rfds;

	// Variable assignments
	numtimeouts = 0;
	pktSent = 0;
	selret = 0;
	pktID = ((struct pkt*)buf)->_id;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)&inPktSentAck,0,sizeof(struct pkt));

	// getting the to address into a variable
	tns = inet_ntoa(to->sin_addr);

	char command[1] = "";
	char buffer[1] = "";
	// Creating ACK pkt
	outPktSentAck = createPkt(((struct pkt*)buf)->_id,&command[0], &((struct pkt*)buf)->_client, ((struct pkt*)buf)->_client_name,((struct pkt*)buf)->_client_password,&buffer[0],FINALACK);

	// assign checksum to checksum pkt
	outPktSentAck->_checksum = performChecksum(outPktSentAck);

	for(numtimeouts = 0; numtimeouts < 3 && (pktSent != 1);numtimeouts++)
	{
		// Send a pkt to reader
		if( sendto(rfd, (char*)outPktSentAck, cc, 0, (struct sockaddr*)to, fromlen) < 0 )
		{
			printf("ERROR in stopConfirmation_FromSender() - sendto()");
			exit(0);
		}

		tval.tv_sec = 0;
		tval.tv_usec = 100000;

		// set socket for select call
		FD_ZERO(&rfds); 
		FD_SET(rfd,&rfds);

		// Set timer for reading on the socket
		selret = select(rfd+1,&rfds,NULL,NULL,&tval);

		if(selret != 0)
		{
			// Wait for pktSentAck response 
#ifdef _WIN32_
			rc = recvfrom(rfd, (char*)&inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), (int*)&fromlen);
			if (rc < 0 )
#else
			if ((rc=recvfrom(rfd, &inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), &fromlen)) < 0 )
#endif
			{
				//printf("recvfrom error\n");
				//printf("rc = %d\n", rc);
				//printf("ERROR in stopConfirmation_FromSender().\n");
				//printf("Write error: errno %d\n",errno);
				//printf("reading datagram");
				//exit(0);
				return 0;
			}

			// assign checksum to checksum pkt
			inChecksum = performChecksum(((struct pkt*)(&inPktSentAck)));

			// getting the from address into a variable
			fns = inet_ntoa(from.sin_addr);

			// Check pkt for ACK
			if((inPktSentAck._ackvar == FINALACK)&&(pktID == inPktSentAck._id)&&(from.sin_port == to->sin_port)&&(!(strcmp(fns,tns))))
			{
				pktSent = 1;
			}
			else
			{
				//printf("fs3-received someone else's pkt and disregarding it\n");
				pktSent = 0;
			}
		}
		else
		{
			//printf("No ack pkt received(stopConfirmation_FromSender)...select returned %d\n",selret);
			numtimeouts++;
			pktSent = 0;
		}
	}
	if(numtimeouts > 1)
	{
		//printf("stopConfirmation_FromSender has timedout...GOOD!\n");
		pktSent = 0;
	}

	delete outPktSentAck;
	return pktSent;
}

//
// receiveDataPkt_FromReceiver
//
// Description: Wait on a port until data is received.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* from - A socket address structure containing
//          the remote ip address and port number.
// Output: int ret - Returns 1 on success and 0 on failure.
int receiveDataPkt_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* from)
{
	// Variable declarations
	int rc, ret, inChecksum;
	unsigned int fromlen;

	// Variable assignments
	ret = 0;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)buf,0,sizeof(struct pkt));

	while(ret != 1)
	{
#ifdef _WIN32_
		if ((rc=recvfrom(rfd, (char*)buf, cc, 0, (struct sockaddr*)from, (int*)&fromlen)) < 0 )
#else
		if ((rc=recvfrom(rfd, buf, cc, 0, (struct sockaddr*)from, &fromlen)) < 0 )
#endif
		{
			printf("receiveDataPkt_FromReceiver() - recvfrom() error: errno %d\n",errno);
			printf("reading datagram");
			exit(0);
		}

		// assign checksum to checksum pkt
		inChecksum = performChecksum( ((struct pkt*)buf) );

		if( (((struct pkt*)buf)->_ackvar != FINALACK) &&  (((struct pkt*)buf)->_ackvar != ACK) && (inChecksum == ((struct pkt*)buf)->_checksum) )
		{
			ret = 1;
		}
		else
		{
			ret = 0;
			//printf("ERROR: either final pkt received(X) , ack pkt received(Z), or Bad checksum!\n");
		}
	}
	return ret;
}


// ACK_FromReceiver
//
// Description: An ACK is sent to notify the sender that the receiver
//                has received the data pkt successfully.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          the remote ip address and port number.
// Output: int pktSent - Returns 1 on success and 0 on failure.
int ACK_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* to)
{
	// Variable declarations
	int rc, numtimeouts, pktSent, selret, pktID, inChecksum;
	unsigned int fromlen;
	char* tns;
	char* fns;
	struct pkt* outPktSentAck;
	struct pkt inPktSentAck;
	struct sockaddr_in from;
	struct timeval tval;
	fd_set rfds;

	// Variable assignments
	numtimeouts = 0;
	pktSent = 0;
	selret = 0;
	pktID = ((struct pkt*)buf)->_id;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)&inPktSentAck,0,sizeof(struct pkt));

	// getting the to address into a variable
	tns = inet_ntoa(to->sin_addr);

	char command[1] = "";
	char buffer[1] = "";
	// Creating ACK pkt
	outPktSentAck = createPkt(((struct pkt*)buf)->_id,&command[0], &((struct pkt*)buf)->_client, ((struct pkt*)buf)->_client_name,((struct pkt*)buf)->_client_password,&buffer[0],ACK);

	// assign checksum to checksum pkt
	outPktSentAck->_checksum = performChecksum(outPktSentAck);

	while(pktSent != 1)
	{
		// Send a pkt to reader
		if( sendto(rfd, (char*)outPktSentAck, cc, 0, (struct sockaddr*)to, fromlen) < 0 )
		{
			printf("ERROR in ACK_FromReceiver() - sendto()");
			exit(0);
		}

		tval.tv_sec = 0;
		tval.tv_usec = 100000;
		// set socket for select call
		FD_ZERO(&rfds); 
		FD_SET(rfd,&rfds);

		// Set timer for reading on the socket
		selret = select(rfd+1,&rfds,NULL,NULL,&tval);

		if(selret != 0)
		{

			// Wait for pktSentAck response 
#ifdef _WIN32_
			if ((rc=recvfrom(rfd, (char*)&inPktSentAck, sizeof(struct pkt), 0, ((struct sockaddr*)(&from)), (int*)&fromlen)) < 0 )
#else
			if ((rc=recvfrom(rfd, &inPktSentAck, sizeof(struct pkt), 0, ((struct sockaddr*)(&from)), &fromlen)) < 0 )
#endif
			{
				printf("ERROR in ACK_FromReceiver().\n");
				printf("Write error: errno %d\n",errno);
				printf("reading datagram");
				exit(0);
			}

			// assign checksum to checksum pkt
			inChecksum = performChecksum(((struct pkt*)(&inPktSentAck)));

			// getting the from address into a variable
			fns = inet_ntoa(from.sin_addr);

			// Check pkt for ACK
			if( (inPktSentAck._ackvar == ACK)	&&
				(pktID == inPktSentAck._id)		&&
				(from.sin_port == to->sin_port) &&
				(!(strcmp(fns,tns))) )
			{
				pktSent = 1;
			}
			else
			{
				//printf("fr2-received someone else's pkt and disregarding it #%d\n",numtimeouts);
				pktSent = 0;
				if(numtimeouts > 2)
				{
					numtimeouts = 0;
					return 0;
					//receiveDataPkt_FromReceiver(rfd, buf, cc, to);
				}
				else
				{
					numtimeouts++;
				}
			}
		}
		else
		{
			//printf("No ack pkt received(ACK_FromReceiver)...select returned %d\n",selret);
			pktSent = 0;
		}
	}
	delete outPktSentAck;
	return pktSent;
}

//
// stopConfirmation_FromReceiver
//
// Description: An ACK is sent to notify the sender to stop sending ACK's
//                When a final ACK is received, the function will finish,
//                or it will simply timeout becuase it knows the sender has
//                already confirmed the data has been sent successfully.
//
// Input: int rfd - A valid RUP file descriptor.
// Input: void* buf - A pointer to the data being sent.
// Input: int cc - The byte count/size of buf.
// Input: struct sockaddr_in* to - A socket address structure containing
//          the remote ip address and port number.
// Output: int pktSent - Returns 1 on success and 0 on failure.
int stopConfirmation_FromReceiver(int rfd, void* buf, int cc, struct sockaddr_in* to)
{

	// Variable declarations
	int rc, numtimeouts, pktSent, selret, pktID, inChecksum;
	unsigned int fromlen;
	char* tns;
	char* fns;
	struct pkt* outPktSentAck;
	struct pkt inPktSentAck;
	struct sockaddr_in from;
	struct timeval tval;
	fd_set rfds;

	// Variable assignments
	numtimeouts = 0;
	pktSent = 0;
	selret = 0;
	pktID = ((struct pkt*)buf)->_id;
	fromlen = sizeof(struct sockaddr_in);

	memset((char*)&inPktSentAck,0,sizeof(struct pkt));

	// getting the to address into a variable
	tns = inet_ntoa(to->sin_addr);

	char command[1] = "";
	char buffer[1] = "";
	// Creating ACK pkt
	outPktSentAck = createPkt(((struct pkt*)buf)->_id,&command[0], &((struct pkt*)buf)->_client, ((struct pkt*)buf)->_client_name,((struct pkt*)buf)->_client_password,&buffer[0],ACK);

	// assign checksum to checksum pkt
	outPktSentAck->_checksum = performChecksum(outPktSentAck);

	for(numtimeouts = 0;numtimeouts < 3 && (pktSent != 1); numtimeouts++)
	{
		// Send a pkt to reader
		if( sendto(rfd, (char*)outPktSentAck, cc, 0, (struct sockaddr*)to, fromlen) < 0 )
		{
			printf("ERROR in stopConfirmation_FromReceiver() - sendto()");
			exit(0);
		}

		tval.tv_sec = 0;
		tval.tv_usec = 100000;

		// set socket for select call
		FD_ZERO(&rfds); 
		FD_SET(rfd,&rfds);

		// Set timer for reading on the socket
		selret = select(rfd+1,&rfds,NULL,NULL,&tval);

		if(selret != 0)
		{
			// Wait for pktSentAck response 
#ifdef _WIN32_
			if ((rc=recvfrom(rfd, (char*)&inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), (int*)&fromlen)) < 0 )
#else
			if ((rc=recvfrom(rfd, &inPktSentAck, sizeof(struct pkt), 0, (struct sockaddr*)(&from), &fromlen)) < 0 )
#endif
			{
				printf("ERROR in stopConfirmation_FromReceiver().\n");
				printf("Write error: errno %d\n",errno);
				printf("reading datagram");
				exit(0);
			}

			// assign checksum to checksum pkt
			inChecksum = performChecksum(((struct pkt*)(&inPktSentAck)));

			// getting the from address into a variable
			fns = inet_ntoa(from.sin_addr);

			// Check pkt for ACK
			if( (inPktSentAck._ackvar == FINALACK) &&(pktID == inPktSentAck._id)&&(from.sin_port == to->sin_port)&&(!(strcmp(fns,tns))))
			{
				pktSent = 1;
			}
			else
			{
				//printf("fr3-received someone else's pkt and disregarding it\n");
				pktSent = 0;
			}
		}
		else
		{
			//printf("No ack pkt received(stopConfirmation_FromReceiver)...select returned %d\n",selret);
			numtimeouts++;
			pktSent = 0;
		}
	}
	delete outPktSentAck;
	return pktSent;
}

//
// printPkt
//
// Description: Print a pkt.
//
// Input: struct pkt* ppPkt - A pkt pointer of the pkt to be printed.
// Output: NA
void printPkt(struct pkt* ppPkt)
{
	printf("ppPkt->_checksum = %d\n",ppPkt->_checksum);
	printf("ppPkt->_id = %d\n",ppPkt->_id);
	printf("ppPkt->_command = %s\n",ppPkt->_command);
	printf("ppPkt->_client.sin_family = %d\n",ppPkt->_client.sin_family);
	printf("ppPkt->_client.sin_addr = %s\n",inet_ntoa(ppPkt->_client.sin_addr));
	printf("ppPkt->_client.sin_port = %d\n",ppPkt->_client.sin_port&0xffff);
	printf("ppPkt->_client_name = %s\n",ppPkt->_client_name);
	printf("ppPkt->_client_password = %s\n",ppPkt->_client_password);
	printf("ppPkt->_msgbuf = %s\n",ppPkt->_msgbuf);
	printf("ppPkt->_ackvar = %c\n",ppPkt->_ackvar);
}

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
struct pkt* createPkt(int cpId, char* cpCommand, struct sockaddr_in* cpClient, char* cpCName, char* cpCPassword, char* cpMsgbuf, char cpAckvar)
{
	// Variable declarations
	struct pkt* newPkt;

	// Variable assignments
	newPkt = new struct pkt;

	memset((char*)newPkt,0,sizeof(struct pkt));

	newPkt->_checksum = 0;
	newPkt->_id = cpId;
	strncpy_s(newPkt->_command,COMMANDSIZE,cpCommand,strlen(cpCommand));
	newPkt->_client.sin_family = cpClient->sin_family;
	newPkt->_client.sin_addr = cpClient->sin_addr;
	newPkt->_client.sin_port = cpClient->sin_port&0xffff;
	strncpy_s(newPkt->_client_name,NAMESIZE,cpCName,strlen(cpCName));
	strncpy_s(newPkt->_client_password,PASSWORDSIZE,cpCPassword,strlen(cpCPassword));
	strncpy_s(newPkt->_msgbuf,BUFSIZE,cpMsgbuf,strlen(cpMsgbuf));
	newPkt->_ackvar = cpAckvar;

	return newPkt;
}

//
// copyPkt
//
// Description: Create a new pkt and copy the incoming pkt contents into it.
//
// Input: struct pkt* inPkt - A pkt pointer of the source pkt to be copied
// Output: struct pkt* - The pkt pointer of the newly created pkt is returned.
struct pkt* copyPkt(struct pkt* inPkt)
{
	// Variable declarations
	struct pkt* newPkt;

	// Variable assignments
	newPkt = new struct pkt;

	memset((char*)newPkt,0,sizeof(struct pkt));

	newPkt->_checksum = inPkt->_checksum;
	newPkt->_id = inPkt->_id;
	strncpy_s(newPkt->_command,COMMANDSIZE,inPkt->_command,strlen(inPkt->_command));
	newPkt->_client.sin_family = inPkt->_client.sin_family;
	newPkt->_client.sin_addr = inPkt->_client.sin_addr;
	newPkt->_client.sin_port = inPkt->_client.sin_port&0xffff; 
	strncpy_s(newPkt->_client_name,NAMESIZE,inPkt->_client_name,strlen(inPkt->_client_name));
	strncpy_s(newPkt->_client_password,PASSWORDSIZE,inPkt->_client_password,strlen(inPkt->_client_password));
	strncpy_s(newPkt->_msgbuf,BUFSIZE,inPkt->_msgbuf,strlen(inPkt->_msgbuf));
	newPkt->_ackvar = inPkt->_ackvar;
	return newPkt;
}
