#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int openread (int port) {
  int sd;
  struct sockaddr_in name;
  
  if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("(openread): socket() error");
    return (-1);
  }
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind (sd, (struct sockaddr *)&name, sizeof(name)) < 0) {
    perror("(openread): bind() error");
    return(-1);
  }
  return (sd);
}


int opensend (char *host, int port) {
  int sd;
  struct sockaddr_in name;
  struct hostent *hent;
  
  if ((sd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("(opensend): socket() error");
    return(-1);
  }
  
  if ((hent = gethostbyname (host)) == NULL)
    fprintf (stderr, "Host %s not found.\n", host);
  else
    bcopy (hent->h_addr, &name.sin_addr, hent->h_length);
  
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  
  /* connect port */
  if (connect (sd, (struct sockaddr *)&name, sizeof(name)) < 0) {
    perror("(opensend): connect() error");
    return(-1);
  }

  return(sd);
}


int readdata (int sd, char *data, int size) {
  return (read (sd, data, size));
}


int senddata (int sd, char *data, int size) {
  return (write (sd, data, size));
}


int main () {

  int sd = opensend ("blackadder", 5050);
  write (sd, "Hello, World!", 14);

  return 0;
}
