/*
 *          		S E R V I D O R
 *
 *	This is an example program that demonstrates the use of
 *	sockets TCP and UDP as an IPC mechanism.  
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>



#define PUERTO 17278
#define ADDRNOTFOUND	0xffffffff	/* return address for unfound host */
#define BUFFERSIZE	1024	/* maximum size of packets to be received */
#define TAM_BUFFER 1000
#define MAXHOST 128

extern int errno;

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the sockets.  It
 *	will loop forever, until killed by a signal.
 *
 */

//PROPIAS
int notImplement501(char * string);
//crearMensaje(char * codigo);
int existe(char *f);

//
 
void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */

int FIN = 0;             /* Para el cierre ordenado */
void finalizar(){ FIN = 1; }

int main(argc, argv)
int argc;
char *argv[];
{

    int s_TCP, s_UDP;		/* connected socket descriptor */
    int ls_TCP;				/* listen socket descriptor */
    
    int cc;				    /* contains the number of bytes read */
     
    struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */
    
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in clientaddr_in;	/* for peer socket address */
	int addrlen;
	
    fd_set readmask;
    int numfds,s_mayor;
    
    char buffer[BUFFERSIZE];	/* buffer for packets to be read into */
    
    struct sigaction vec;

		/* Create the listen socket. */
	ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
	if (ls_TCP == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
		exit(1);
	}
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    addrlen = sizeof(struct sockaddr_in);

		/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should listen on the wildcard address,
		 * rather than its own internet address.  This is
		 * generally good practice for servers, because on
		 * systems which are connected to more than one
		 * network at once will be able to have one server
		 * listening on all networks at once.  Even when the
		 * host is connected to only one network, this is good
		 * practice, because it makes the server program more
		 * portable.
		 */
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = htons(PUERTO);

	/* Bind the listen address to the socket. */
	if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
		exit(1);
	}
		/* Initiate the listen on the socket so remote users
		 * can connect.  The listen backlog is set to 5, which
		 * is the largest currently supported.
		 */
	if (listen(ls_TCP, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
		exit(1);
	}
	
	
	/* Create the socket UDP. */
	s_UDP = socket (AF_INET, SOCK_DGRAM, 0);
	if (s_UDP == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket UDP\n", argv[0]);
		exit(1);
	}

	/* Bind the server's address to the socket. */
	if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address UDP\n", argv[0]);
		exit(1);
    }

		/* Now, all the initialization of the server is
		 * complete, and any user errors will have already
		 * been detected.  Now we can fork the daemon and
		 * return to the user.  We need to do a setpgrp
		 * so that the daemon will no longer be associated
		 * with the user's control terminal.  This is done
		 * before the fork, so that the child will not be
		 * a process group leader.  Otherwise, if the child
		 * were to open a terminal, it would become associated
		 * with that terminal as its control terminal.  It is
		 * always best for the parent to do the setpgrp.
		 */
	setpgrp();

	switch (fork()) {
		case -1:		/* Unable to fork, for some reason. */
			perror(argv[0]);
			fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
			exit(1);

		case 0:     /* The child process (daemon) comes here. */

				/* Close stdin and stderr so that they will not
				 * be kept open.  Stdout is assumed to have been
				 * redirected to some logging file, or /dev/null.
				 * From now on, the daemon will not report any
				 * error messages.  This daemon will loop forever,
				 * waiting for connections and forking a child
				 * server to handle each one.
				 */
			fclose(stdin);
			fclose(stderr);

				/* Set SIGCLD to SIG_IGN, in order to prevent
				 * the accumulation of zombies as each child
				 * terminates.  This means the daemon does not
				 * have to make wait calls to clean them up.
				 */
			if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
	            perror(" sigaction(SIGCHLD)");
	            fprintf(stderr,"%s: unable to register the SIGCHLD signal\n", argv[0]);
	            exit(1);
	        }
	            
			    /* Registrar SIGTERM para la finalizacion ordenada del programa servidor */
	        vec.sa_handler = (void *) finalizar;
	        vec.sa_flags = 0;
	        if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
	            perror(" sigaction(SIGTERM)");
	            fprintf(stderr,"%s: unable to register the SIGTERM signal\n", argv[0]);
	            exit(1);
	        }
	        
			while (!FIN) {
	            /* Meter en el conjunto de sockets los sockets UDP y TCP */
	            FD_ZERO(&readmask);
	            FD_SET(ls_TCP, &readmask);
	            FD_SET(s_UDP, &readmask);
	            /* 
	            Seleccionar el descriptor del socket que ha cambiado. Deja una marca en 
	            el conjunto de sockets (readmask)
	            */ 
	    	    if (ls_TCP > s_UDP) s_mayor=ls_TCP;
	    		else s_mayor=s_UDP;

	            if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
	                if (errno == EINTR) {
	                    FIN=1;
			            close (ls_TCP);
			            close (s_UDP);
	                    perror("\nFinalizando el servidor. SeÃal recibida en elect\n "); 
	                }
	            }else{ 

	                /* Comprobamos si el socket seleccionado es el socket TCP */
	                if (FD_ISSET(ls_TCP, &readmask)) {
	                    /* Note that addrlen is passed as a pointer
	                     * so that the accept call can return the
	                     * size of the returned address.
	                     */
	    				/* This call will block until a new
	    				 * connection arrives.  Then, it will
	    				 * return the address of the connecting
	    				 * peer, and a new socket descriptor, s,
	    				 * for that connection.
	    				 */
		    			s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
		    			if (s_TCP == -1) exit(1);
		    			switch (fork()) {
		        			case -1:	/* Can't fork, just exit. */
		        				exit(1);
		        			case 0:		/* Child process comes here. */
		                    			close(ls_TCP); /* Close the listen socket inherited from the daemon. */
		        				serverTCP(s_TCP, clientaddr_in);
		        				exit(0);
		        			default:	/* Daemon process comes here. */
		    					/* The daemon needs to remember
		    					 * to close the new accept socket
		    					 * after forking the child.  This
		    					 * prevents the daemon from running
		    					 * out of file descriptor space.  It
		    					 * also means that when the server
		    					 * closes the socket, that it will
		    					 * allow the socket to be destroyed
		    					 * since it will be the last close.
		    					 */
		        				close(s_TCP);
		        		}
	             	} /* De TCP*/
		          	/* Comprobamos si el socket seleccionado es el socket UDP */
		          	if (FD_ISSET(s_UDP, &readmask)) {
		                /* This call will block until a new
		                * request arrives.  Then, it will
		                * return the address of the client,
		                * and a buffer containing its request.
		                * BUFFERSIZE - 1 bytes are read so that
		                * room is left at the end of the buffer
		                * for a null character.
		                */
				
					memset(buffer, '\0',sizeof(buffer));					


	                cc = recvfrom(s_UDP, buffer, BUFFERSIZE - 1, 0,
	                   (struct sockaddr *)&clientaddr_in, &addrlen);
	                if ( cc == -1) {
	                    perror(argv[0]);
	                    printf("%s: recvfrom error\n", argv[0]);
	                    exit (1);
	                }
	                /* Make sure the message received is
	                * null terminated.
	                */
	                buffer[cc]='\0';

	                serverUDP (s_UDP, buffer, clientaddr_in);
	                }
	          	}
			}   /* Fin del bucle infinito de atención a clientes */
	        /* Cerramos los sockets UDP y TCP */
	        close(ls_TCP);
	        close(s_UDP);
	    
	        printf("\nFin de programa servidor!\n");
	        
		default:		/* Parent process comes here. */
			exit(0);
	}

}

/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverTCP(int s, struct sockaddr_in clientaddr_in)
{
	FILE *fp;
	fp = fopen("peticiones.log","a");


	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[TAM_BUFFER];		/* This example uses TAM_BUFFER byte messages. */
	char buf2[TAM_BUFFER];		/* This example uses TAM_BUFFER byte messages. */
	char hostname[MAXHOST];		/* remote host's name string */

	int len, len1, status;
    struct hostent *hp;		/* pointer to host info for remote host */
    long timevar;			/* contains time returned by time() */
    
    struct linger linger;		/* allow a lingering, graceful close; */
    				            /* used when setting SO_LINGER */
    				
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
    status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),hostname,MAXHOST,NULL,0,0);
    if(status){
           	/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
			 /* inet_ntop para interoperatividad con IPv6 */
            if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
            	perror(" inet_ntop \n");
            }
    /* Log a startup message. */
    time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Startup from %s port %u at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

		/* Set the socket for a lingering, graceful close.
		 * This will cause a final close of this socket to wait until all of the
		 * data sent on it has been received by the remote host.
		 */
	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		errout(hostname);
	}

	
	fprintf(fp,">>Host: %s,\tDireccion IP: %s,\tPuerto: %u,\tProtocolo: TCP,\tA las: %s\n",
		hostname, inet_ntoa(clientaddr_in.sin_addr), ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));		
	
	//ARCHIVO A USAR
	FILE *f;
	char ruta[150];
	memset (buf, '\0', sizeof (ruta)); 	//reset ruta


	while (len = recv(s, buf, TAM_BUFFER, 0)) {
		if (len == -1) errout(hostname); /* error from recv */
			/* The reason this while loop existe is that there
			 * is a remote possibility of the above recv returning
			 * less than TAM_BUFFER bytes.  This is because a recv returns
			 * as soon as there is some data, and will not wait for
			 * all of the requested data to arrive.  Since TAM_BUFFER bytes
			 * is relatively small compared to the allowed TCP
			 * packet sizes, a partial receive is unlikely.  If
			 * this example had used 2048 bytes requests instead,
			 * a partial receive would be far more likely.
			 * This loop will keep receiving until all TAM_BUFFER bytes
			 * have been received, thus guaranteeing that the
			 * next recv at the top of the loop will start at
			 * the begining of the next request.
			 */
		while (len < TAM_BUFFER) {
			len1 = recv(s, &buf[len], TAM_BUFFER-len, 0);
			if (len1 == -1) errout(hostname);
			len += len1;
		}
			/* Increment the request count. */
		reqcnt++;
			/* This sleep simulates the processing of the
			 * request that a real server might do.
			 */
		sleep(1);

		///////AQUI COMENZAMOS TRATAMIENTO DEL MENSAJE/////////
				
		//VARIABLES A USAR
		//FILE *f;
		//char ruta[150];
		char codigo[40];
		int it,it2;
		int tammen;
		char archivo[100], mensaje[1024], ka[150];
		/*mensaje[0]='\0'; //RESETEAMOS
		archivo[0]='\0';
		ruta[0]='\0';*/

		memset (mensaje,'\0' , sizeof (mensaje));
		memset (archivo, '\0', sizeof (archivo));
		//memset (ruta, '\0', sizeof (ruta));



		switch(buf[1])
		{
			case '2':	//CLIENTE QUIERE ESCRIBIR

				//sprintf(buf2,"He recibido que quieren escribir.\n");
				//02fichero.txt0octet0 tengo que quitar:|0octet0| = 6 +1 xq es de 0 a n-1 
				tammen = strlen(buf);
				it2 = 0;
				for (it = 2; it < tammen-7; ++it)
				{
					buf2[it2] = buf[it];
					it2++;
				}
				//tenemos el fichero en buf2
				//Establecemos ruta:
				sprintf(ruta,"ficherosTFTPserver/");
				strcat(ruta,buf2);

				if(existe(ruta)){
					//AQUíIMPLEMENTAR ERROR SOBREESCRITURA
					sprintf(buf2,"0506Error: No puede sobreescribir.0\n");
				}else{
					//Mensaje confirmación
					//sprintf(buf2,"04bloque0");
				}

			break;

			case '3':	//SERVIDOR RECIBIENDO DATOS DEL CLIENTE

				sprintf(buf2,"Estamos escribiendo en %s.\n",ruta);
				tammen = strlen(buf);

				if((f = fopen(ruta,"a")) == NULL){
					perror("Fallo al crear/escribir documento.");
				}else{
					for (it = 4; it < tammen; ++it)
					{
						fputc(buf[it],f);
					}
				}
				fclose(f);


			break;

		}



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     	memset (buf, '\0', sizeof (buf));



					/* Send a response back to the client. */
		if (send(s, buf2, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);

		memset (buf2, '\0', sizeof (buf2));
	}

		/* The loop has terminated, because there are no
		 * more requests to be serviced.  As mentioned above,
		 * this close will block until all of the sent replies
		 * have been received by the remote host.  The reason
		 * for lingering on the close is so that the server will
		 * have a better idea of when the remote has picked up
		 * all of the data.  This will allow the start and finish
		 * times printed in the log file to reflect more accurately
		 * the length of time this connection was used.
		 */
	fprintf(fp,"Conexion cerrada...\n\n");
    fclose(fp);
	close(s);

		/* Log a finishing message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Completed %s port %u, %d requests, at %s\n",
		hostname, ntohs(clientaddr_in.sin_port), reqcnt, (char *) ctime(&timevar));
}

/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);     
}


/*
 *				S E R V E R U D P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in)
{

	char hostname[MAXHOST];		/* remote host's name string */

	long timevar;			/* contains time returned by time() */
	    
	FILE *fp3;
	fp3 = fopen("peticiones.log","a");

	if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL){
		perror(" inet_ntop \n");
	}

	getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
	                           hostname,MAXHOST,NULL,0,0);             //OBTENEMOS EL NOMBRE DEL SERVIDOR


	fprintf(fp3,">>Host: %s,\tDireccion IP: %s,\tPuerto: %u,\tProtocolo: UDP,\tA las: %s\n",
			hostname, inet_ntoa(clientaddr_in.sin_addr), ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

	printf("%s<<<<<<<<<<<<<<<<<<",buffer);

	printf("\n\nMENSAJE UDP RECIBIDO.........\n\n");

	//printf("%s<<<<<<<<<<<<<<<<<<",buffer);
			char buffer2[BUFFERSIZE];

	//////////////////////////////////////////////////////////////////



	char codigo2[40];
	int itUDP,itUDP2;
	char archivoUDP[100],mensajeUDP[1024], kaUDP[150];
	char rutaUDP[150];
		mensajeUDP[0]=	'\0'; //RESETEAMOS
		archivoUDP[0]=	'\0';
		rutaUDP[0]=		'\0';


	memset (mensajeUDP, '\0', sizeof (mensajeUDP));
	memset (archivoUDP, '\0', sizeof (archivoUDP));
	memset (rutaUDP, '\0', sizeof (rutaUDP));


	sprintf(rutaUDP,"www/");
	printf("\n\nruta: %s\n\n",rutaUDP); 

	//KEEP-ALIVE

	itUDP2 = 0;
	for(itUDP=0;buffer[itUDP]!='\0';itUDP++)
	{
		if(buffer[itUDP] == '\n') itUDP2++;
	}

	if(itUDP2==3){
		sprintf(kaUDP,"\nConnection: keep-alive<CR><LF>\n");
	}else if(itUDP2==2){
		sprintf(kaUDP,"\nConnection: close<CR><LF>\n");
	}



	for(itUDP=0;buffer[itUDP]!='/';itUDP++);

	for(itUDP2=itUDP;buffer[itUDP2]!=' ';itUDP2++);

	int k=0;
	for(itUDP=itUDP+1;itUDP!=itUDP2;itUDP++){
		archivoUDP[k] = buffer[itUDP];
		k++;
	}
	printf("\nVALOOOOR rutaUDP: ->%s<-\n\n",archivoUDP);


	strcat(rutaUDP,archivoUDP);


	printf("\nVALOOOOR ruta: ->%s<-\n\n",rutaUDP);

	//501 Not Implemented
	if(notImplement501(buffer) == -1){
	  	sprintf(codigo2,"501: Not implemented");
		sprintf(buffer2,"HTTP/1.1 %s<CR><LF>\nServer: Servidor del Francho y del Jisu<CR><LF>%s<CR><LF>\n<html><body><h1>501 Not Implemented</h1></body></html>",codigo2,kaUDP);
	}else{


		FILE *fp4;
		char aux[1024];

		//404 Not Found
		if((fp4 = fopen(rutaUDP,"r")) == NULL) 
		{
			sprintf(codigo2,"404: Not found");
			sprintf(buffer2,"HTTP/1.1 %s<CR><LF>\nServer: Servidor del Francho y del Jisu<CR><LF>%s<CR><LF>\n<html><body><h1>404 Not Found</h1></body></html>",codigo2,kaUDP);
		}else{ //200 OK
		 	sprintf(codigo2,"200: OK");
			fp4 = fopen(rutaUDP,"r");
			while(!feof(fp4)){
				fgets(aux,1024,fp4);
				strcat(mensajeUDP,aux);
				sprintf(buffer2,"HTTP/1.1 %s<CR><LF>\nServer: Servidor del Francho y del Jisu<CR><LF>%s<CR><LF>\n%s",codigo2,kaUDP,mensajeUDP);
			}
		}
	}

	if(!strcmp(codigo2,"200: OK"))fprintf(fp3,"Objeto solicitado: %s -> Todo correcto\n", rutaUDP);
	else fprintf(fp3,"Objeto solicitado: %s -> Error: %s\n", rutaUDP, codigo2);












//////////////////////////////////////////////////////////////////


    //struct in_addr reqaddr;	/* for requested host's address */
    struct hostent *hp;		/* pointer to host info for requested host */
    int nc, errcode;

    struct addrinfo hints, *res;

	int addrlen;
    
   	addrlen = sizeof(struct sockaddr_in);

      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
		/* Treat the message as a string containing a hostname. */
	    /* Esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta. */
    errcode = getaddrinfo (buffer, NULL, &hints, &res); 
    //if (errcode != 0){printf("\n\nerrcode........\n\n");
		/* Name was not found.  Return a
		 * special value signifying the error. */
		//reqaddr.s_addr = ADDRNOTFOUND;
    //  }
    //else {
		/* Copy address of host into the return buffer. */
		//reqaddr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	//}
     //freeaddrinfo(res);

	printf("\n%s\n\n",buffer2);
	fprintf(fp3,"\nConexion cerrada\n\n\n");
	fclose(fp3);
	nc = sendto (s, buffer2, sizeof(buffer2), 0, (struct sockaddr *)&clientaddr_in, addrlen);
	if ( nc == -1) {
		 perror("serverUDP");
		 printf("%s: sendto error\n", "serverUDP");
		 return;
  	}   
	memset(buffer2, '\0',sizeof(buffer2));

}









//PROPIAS



int notImplement501(char * string){

	if(string[0] == 'G' && string[1] == 'E' && string[2] == 'T' && string[3]==' ') return 0;

	return -1;
}


int existe(char *r)
{
	FILE *f;
  if((f = fopen(r, "r")) == NULL) return 0;
  /* Si existe, cerramos */
  fclose(f);
  return 1;
}


