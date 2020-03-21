/*
dcsend was written by oman.
Pick it up at http://www.julesdcdev.com/
It can be used with the IP upload slave 1.002 by Marcus Comstedt 
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char * argv[])
{
  int ClientSocket, RemotePort, status = 0;
  char * RequestedHost = NULL;
  char * RequestedFile = NULL;
  struct hostent * HostPtr = NULL;
  struct sockaddr_in ServerName = { 0 };
    
  char Buffer[0xFFFF];

  FILE * Input = NULL;
 
  RemotePort = 4711;
 
  if (argc != 3)
  {
    fprintf(stderr, "USAGE %s filename.elf dchost\r\n", argv[0]);
    return 1;
  }

  RequestedHost = argv[2];
  RequestedFile = argv[1];

  Input = fopen(RequestedFile, "rb");

  if (Input == NULL)
  {
    fprintf(stderr, "ERROR: Could not open file %s for read\r\n", RequestedFile);
    return 1;
  }

  ClientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (ClientSocket == -1)
  {
    fprintf(stderr, "ERROR: Could not create socket\r\n");
    return 1;
  }

  HostPtr = gethostbyname(RequestedHost); 

  if (HostPtr == NULL)
  {
    HostPtr = gethostbyaddr(RequestedHost, strlen(RequestedHost), AF_INET);
    if (HostPtr == NULL)
    {
      fprintf(stderr, "ERROR: Could not resolve hostname\r\n");
      return 1;
    }
  }
 
  
  ServerName.sin_family = AF_INET;
  ServerName.sin_port = htons(RemotePort);
  (void) memcpy(&ServerName.sin_addr, HostPtr->h_addr, HostPtr->h_length);

  status = connect(ClientSocket, (struct sockaddr*) &ServerName, sizeof(ServerName));
  if (status == -1)
  {
    fprintf(stderr, "ERROR: Could not connect to host\r\n");
    return 1;
  }

  status = fread(Buffer, 1, sizeof(Buffer), Input);
  while (status > 0)
  {
    if (write(ClientSocket, Buffer, status) == -1)
    {
      fprintf(stderr, "ERROR: Error on socket write");
      return 1;
    }
    status = fread(Buffer, 1, sizeof(Buffer), Input);
  } 

  close(ClientSocket);

  return 0;
}
