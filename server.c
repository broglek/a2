#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORTNUM 5000
#define MAXHOSTNAME 512
#define WORKERS 6
#define MAXWORKERS 30
#define MAXSLOTS 30
#define M 5

struct circ_payload{
  int priority;
  int worker_id;
  char *message;
  //video frame goes here
};


struct in_addr* client_address;
struct circ_payload circbuf[MAXSLOTS];
int workpointer, dispatchpointer,slotfill;
pthread_mutex_t fullbuf_mutex; //for buffer full scenario
pthread_cond_t fullbuf_cv;     //for buffer full scenario
pthread_mutex_t dispatch_mutex;
pthread_cond_t dispatch_cv;
pthread_mutex_t worker_mutex[MAXWORKERS];
pthread_cond_t worker_cv[MAXWORKERS];


struct worker_data{
  int id;
  int active;
  int t;
  pthread_t thread;
};


int establish(unsigned short portnum)
{

  char myname[MAXHOSTNAME+1];

  int s;
  struct sockaddr_in sa;
  struct hostent *hp;
  memset(&sa, 0, sizeof(struct sockaddr)); /* clear our address */
  gethostname(myname, MAXHOSTNAME); /* who are we? */
  hp =  gethostbyname(myname);    /* get our address info */
 
  if (hp == NULL)                       /* we don't exist !? */
    return(-1);
  sa.sin_family= hp->h_addrtype; /* this is our host address */
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port= htons(portnum);             /* this is our port number */
  if ((s= socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
    return(-1);
  if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) < 0) 
  {
    close(s);
    return(-1);          /* bind address to socket */
  }
  printf("Listening Now on port 5000\n\n");
  listen(s, 10);           /* max # of queued connects */
 
  return(s); 
}

/* wait for a connection to occur on a socket created with establish() */
int get_connection(int s, struct sockaddr* addr,socklen_t *cliaddr) 
{
  int t;         /* socket of connection */
  if ((t = accept(s,addr,cliaddr)) < 0) /* accept connection if there is one */
    return(-1);
  return(t); 
}

void* worker_routine(void * n)
{
  struct worker_data *mydata;
  struct circ_payload job;
  mydata = (struct worker_data *) n;
  printf("I am a worker number %d\n",mydata->id);
  pthread_mutex_lock(&worker_mutex[mydata->id]);
  while(mydata->active != 1)
    {
      pthread_cond_wait(&worker_cv[mydata->id], &worker_mutex[mydata->id]);
  printf("Woken Up with job.  Thread id %d socket desc %d\n Going to put in buffer.\n",mydata->id,mydata->t);
  pthread_mutex_lock(&fullbuf_mutex);
  while(slotfill == MAXSLOTS)
    {
      pthread_cond_wait(&fullbuf_cv,&fullbuf_mutex);
    }

      job.worker_id = mydata->id;
      job.priority = mydata->id;
      job.message = malloc(300);
      sprintf(job.message,"Thread id is %d socket is %d\n",mydata->id,mydata->t);
      circbuf[workpointer] = job;
      printf("in worker active is %d %d\n",mydata->id,mydata->active);
      //printf("%s\n",circbuf[workpointer].message);
      workpointer = (workpointer + 1) % MAXSLOTS;
      pthread_mutex_unlock(&fullbuf_mutex);
      mydata->active = 0;

  pthread_mutex_unlock(&worker_mutex[mydata->id]);
    }
}

void* dispatch_routine(void * n)
{
  printf("Dispatcher thread reporting in.\n");
}

int main() 
{
  
  int s, t, i;
  //pthread_mutex_init(&circmutex,NULL);
  workpointer = 0;
  dispatchpointer = 0;
  slotfill = 0;
  //struct worker_thread worker_threads[MAXWORKERS];
  pthread_t dispatcher;
  struct worker_data thread_data[MAXWORKERS];
  pthread_mutex_init(&fullbuf_mutex,NULL);
  pthread_cond_init(&fullbuf_cv,NULL);
  for (i=0;i<MAXWORKERS;i++)
    {
      pthread_mutex_init(&worker_mutex[i],NULL);
      pthread_cond_init(&worker_cv[i],NULL);
    }
  if(pthread_create(&dispatcher,NULL,dispatch_routine,NULL))
    {
      printf("pthread_create failed on dispatcher.\n");
      exit(1);
    }
  for(i=0;i<WORKERS;i++)
    {
      //thread_data[i] = (struct worker_data *)malloc(sizeof(int));
      thread_data[i].id = i;
      thread_data[i].active = 0;
      
      if (pthread_create(&(thread_data[i].thread), NULL, worker_routine, (void *)&thread_data[i]))
        {
          printf("pthread_create failed on thread %d.\n",i);
          exit(1);
        }
    }

  if ((s = establish(PORTNUM)) < 0)
  {
    perror("establish");
    exit(1);
  }
  for (;;)
 {             
   struct sockaddr * cliaddr = malloc(10000);
   socklen_t clilen = 10000;
    /* get a connection */
   if ((t= get_connection(s,cliaddr,&clilen)) < 0)
    {          
      perror("accept");          /* bad */
      exit(1);
    }
    char ip[46];
    unsigned short port;
 
    switch (cliaddr->sa_family) {
    case AF_INET:
      inet_ntop (AF_INET, &(((struct sockaddr_in *)cliaddr)->sin_addr), ip, sizeof (ip));
      port = ntohs (((struct sockaddr_in *)cliaddr)->sin_port);
      break;
    case AF_INET6:
      inet_ntop (AF_INET6, &(((struct sockaddr_in6 *)cliaddr)->sin6_addr), ip, sizeof (ip));
      port = ntohs (((struct sockaddr_in6 *)cliaddr)->sin6_port);
      break;
    default:
      snprintf (ip, sizeof (ip), "UNKNOWN FAMILY: %d", cliaddr->sa_family);
      port = 0;
      break;
    }
    printf("connection from %s, port %hu\n", ip, port);
    //client_address =  &(((struct sockaddr_in *)cliaddr)->sin_addr);
    int f;
    for(f=0;f<MAXWORKERS;f++)
      {
	if(&thread_data[f] != NULL)
	  {
	    if(thread_data[f].active == 0)
	      {
		pthread_mutex_lock(&worker_mutex[f]);
		thread_data[f].t = t;
		thread_data[f].active = 1;
		printf("thread %d takes it\n",f);
		//while(slotfill == MAXSLOTS);
		pthread_cond_signal(&worker_cv[f]);
		pthread_mutex_unlock(&worker_mutex[f]);
		break;
	      }
	  }
      }
    
 }
}
