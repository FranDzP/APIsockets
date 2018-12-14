/*

** Fichero: cliente.c
** Autores: 
** Jesús García Fernández 80103448Z
** Francisco Díaz Plaza 70820100H
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>

#define PUERTO 17278
#define TAM_BUFFER 1000
#define ADDRNOTFOUND	0xffffffff  //valor devuelto para host desconocido
#define RETRIES	5		//max intentos
#define BUFFERSIZE	1024	//tamaño maximo paquetes recibid
#define TIMEOUT 6
#define MAXHOST 512


/*
 *			H A N D L E R
 */
void handler()
{
 	printf("Alarma recibida \n");
}


//PROPIAS
int existe(char *f);



/*
 *			M A I N
 */

int main(argc, argv)
int argc;
char *argv[];
{

	//----------------------------------------------------------------
	//----------------------TCP---------------------------------------
	//----------------------------------------------------------------


	if(!strcmp(argv[2],"TCP")){

	    int s;
	   	struct addrinfo hints, *res;
	    long timevar;
	    struct sockaddr_in myaddr_in;	//dir local socket
	    struct sockaddr_in servaddr_in;	//dir servidor socket
		int addrlen, i, j, errcode;
		char buf[TAM_BUFFER];
		char buf2[TAM_BUFFER],buf3[TAM_BUFFER];

		//-------------------------------------------------------------------//
		if (argc != 5) {
			fprintf(stderr, "uso:  %s <host> <protocolo> <accion> <fichero ordenes>\n", argv[0]);
			exit(1);
		}
		s = socket (AF_INET, SOCK_STREAM, 0); //CREAMOS EL SOCKET
		if (s == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to create socket\n", argv[0]);
			exit(1);
		}

		memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
		memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));


		servaddr_in.sin_family = AF_INET; //direccion por la que nos contectamos

	    memset (&hints, 0, sizeof (hints));
	    hints.ai_family = AF_INET;

	    errcode = getaddrinfo (argv[1], NULL, &hints, &res); //comprobación errores en la dirección
	    if (errcode != 0){
			fprintf(stderr, "%s: No es posible resolver la IP de %s\n",	argv[0], argv[1]);
			exit(1);
	    }else{
			servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr; //direccion servidor
		}
	    freeaddrinfo(res);
		servaddr_in.sin_port = htons(PUERTO); //puerto de servidor


		if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) { //CONECTAMOS CON EL SERVIDOR
			perror(argv[0]);
			fprintf(stderr, "%s: unable to connect to remote\n", argv[0]);
			exit(1);
		}

	  	addrlen = sizeof(struct sockaddr_in);

		if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) { //NUMERO PUERTO CLIENTE != -1 (comprobacion fallos)
			perror(argv[0]);
			fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
			exit(1);
		 }
		
		time(&timevar);

		printf("Connected to %s on port %u at %s",argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

		///----------------------BLOQUE DE CODIGO API C - TCP

		///----------------------BLOQUE DE CODIGO escribir (enviar archivo) - TCP

		if(!strcmp(argv[3],"e")){
			printf("Estoy dentro de escritura de fichero\n");

			//variables a usar
			char mensaje[1024],aux[1024];
   			//memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
   			//Memset (aux, '\0', sizeof (aux)); 	//reset aux
			mensaje[0] = aux[0] = '\0';

			//PRIMERO creamos la peticion de escritura

			sprintf(mensaje,"02%s0octet0",argv[4]);
			printf("%s\n", mensaje);

			if (send(s, mensaje, TAM_BUFFER, 0) != TAM_BUFFER){ 								
				fprintf(stderr, "%s: Connection aborted on error ",	argv[0]); 
				fprintf(stderr, "on send number %d\n", i);
				exit(1);
			} 
	        
	        sleep(1);

			i = recv(s, buf2, TAM_BUFFER, 0);
		    
		    if (i == -1) {
		    	perror(argv[0]);
				fprintf(stderr, "%s: error reading result\n", argv[0]);
				exit(1);
			}

			printf("\n>>Respuesta del servidor: \n%s\n", buf2);

			if(buf2[1] != '4'){	//Servidor nos avisa si ya tiene el archivo o se le puede enviar.

				printf("El fichero ya existe.\n"); 
			
			}else{

				//COMENZAMOS A MANDAR EL ARCHIVO POR BLOQUES

				FILE *f;
				
				//Establecimiento de la ruta
				char ruta[150];
				memset (ruta, '\0', sizeof (ruta)); 	//reset ruta
				//ruta[0]='\0';
				sprintf(ruta,"ficherosTFTPclient/");
				strcat(ruta,argv[4]);
				printf("\n\nruta: %s\n\n",ruta);


				//Abrimos fichero		
				if((f = fopen(ruta,"r")) == NULL) 
				{
		  			printf("404: Not found");
				}else{ //200 OK
				 	printf("200: OK");
				 	int j = 0; //para contar las iteraciones
	 				int nume = 512; //NUMERO BITS LEIDOS (inicializamos en 512 para entrar en el while)
					while(nume == 512){
						j++;
						memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
			   			memset (aux, '\0', sizeof (aux)); 	//reset aux

			   			//mensaje[0]='\0';
			   			//aux[0]='\0';
					
						//CANTIDADDEBYTESLEIDOS=FREAD(CONTENIDO,NºBYTESXELEMENTO,NUMERODEELEMENTOS,FICHERO)

						nume = fread(&mensaje,1,512,f);	
						printf("\n\nLEEMOS DE ARCHIVO %d: \n\n",nume);
						//printf("%s\n",mensaje);

						//CREAMOS EL MENSAJE

						//Como todo va sobre ruedas, vamos a darle formato
						if(j < 10) sprintf(aux,"030%d",j);
						else sprintf(aux,"03%d",j);

						printf("%s\n",aux);
						sleep(3);
						strcat(aux,mensaje);
						printf("\nNUMERODEBYTESMENSAJE:%ld\n",strlen(aux));
						sleep(1);

						//MANDAMOS EL MENSAJE
						memset (buf, '\0', sizeof (buf)); 	//reset mensaje
						strcpy(buf,aux);

						if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER){ 								
							fprintf(stderr, "%s: Connection aborted on error ",	argv[0]); 
							fprintf(stderr, "on send number %d\n", i);
							exit(1);
						} 
				        
				        sleep(1);

						i = recv(s, buf2, TAM_BUFFER, 0);
					    
					    if (i == -1) {
					    	perror(argv[0]);
							fprintf(stderr, "%s: error reading result\n", argv[0]);
							exit(1);
						}

						printf("\n>>Respuesta del servidor: \n%s\n", buf2);

					}//FIN WHILE CONTENIDO

					fclose(f);

				}//FIN FICHERO

			}//FIN Bloque de confirmado (Servidor)

			sleep(5);//sleep de comprobacion

		}//FIN ESCRITURA FICHERO




		///----------------------BLOQUE DE CODIGO leer (recibir archivo) - TCP
		
		if(!strcmp(argv[3],"l")){
			printf("Estoy dentro de lectura de fichero\n");

			//variables a usar
			char mensaje[1024],aux[1024];
   			//memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
   			//Memset (aux, '\0', sizeof (aux)); 	//reset aux
			mensaje[0] = aux[0] = '\0';


			//Primero se mira si el cliente tiene el fichero para no sobreescribir
			FILE *f;
				
			//Establecimiento de la ruta
			char ruta[150];
			memset (ruta, '\0', sizeof (ruta)); 	//reset ruta
			//ruta[0]='\0';
			sprintf(ruta,"ficherosTFTPclient/");
			strcat(ruta,argv[4]);
			printf("\n\nruta: %s\n\n",ruta);

			if(existe(ruta)){
					//mensaje ERROR NO ENCONTRADO
					printf("Cuidado, ya tiene un archivo con este nombre.\n"); //bloque 0
					//break;
			}else{

				//Creamos la peticion de escritura

				sprintf(mensaje,"01%s0octet0",argv[4]);
				printf("%s\n", mensaje);

				if (send(s, mensaje, TAM_BUFFER, 0) != TAM_BUFFER){ 								
					fprintf(stderr, "%s: Connection aborted on error ",	argv[0]); 
					fprintf(stderr, "on send number %d\n", i);
					exit(1);
				} 
		        
		        sleep(1);

				i = recv(s, buf2, TAM_BUFFER, 0);
			    
			    if (i == -1) {
			    	perror(argv[0]);
					fprintf(stderr, "%s: error reading result\n", argv[0]);
					exit(1);
				}

				printf("\n>>Respuesta del servidor: \n%s\n", buf2);

				if(buf2[1] != '3'){
					printf("No tiene el archivo.\n");
				}else{

					//COMENZAMOS A RECIBIR EL ARCHIVO POR BLOQUES

					//Variables:
					int j;
					int tammen = 516;
					

					while(tammen == 516){

						tammen = strlen(buf2);

						if((f = fopen(ruta,"a")) == NULL){
							perror("Fallo al crear/escribir documento.");
						}else{
							for (j = 4; j < tammen; ++j)
							{
								fputc(buf2[j],f);
							}
						}
						fclose(f);

						//mensaje asentimiento
						sprintf(buf,"04%c%c",buf2[2],buf2[3]);

						if (send(s, buf, TAM_BUFFER, 0) != TAM_BUFFER){ 								
							fprintf(stderr, "%s: Connection aborted on error ",	argv[0]); 
							fprintf(stderr, "on send number %d\n", i);
							exit(1);
						} 
				        
				        sleep(1);

				        if(tammen == 516){ //ESPERA A RECIBIR SIEMPRE QUE NO SEA EL ÚLTIMO MENSAJE

							i = recv(s, buf2, TAM_BUFFER, 0);
						    
						    if (i == -1) {
						    	perror(argv[0]);
								fprintf(stderr, "%s: error reading result\n", argv[0]);
								exit(1);
							}

							printf("\n>>Respuesta del servidor: \n%s\n", buf2);

						}

					}


				}

			}

		}

			

		
		if (shutdown(s, 1) == -1) { //CIERRA LA CONEXION
			perror(argv[0]);
			fprintf(stderr, "%s: unable to shutdown socket\n", argv[0]);
			exit(1);
		}


		time(&timevar); //ESCRIBE EL TIEMPO SOBRE TIMEVAR PARA INDICAR A QUE HORA SE HA DESCONECTADO EN UN PRINTF
		printf("All done at %s", (char *)ctime(&timevar));
	}


	//----------------------------------------------------------------
	//----------------------UDP---------------------------------------
	//----------------------------------------------------------------

	if(!strcmp(argv[2],"UDP")){

		int i, errcode;
		int retry = RETRIES;		/* holds the retry count */
	    int s;				/* socket descriptor */
	    long timevar;                       /* contains time returned by time() */
	    struct sockaddr_in myaddr_in;	/* for local socket address */
	    struct sockaddr_in servaddr_in;	/* for server socket address */
	    struct in_addr reqaddr;		/* for returned internet address */
	    int	addrlen, n_retry;
	    struct sigaction vec;
	   	char hostname[MAXHOST];
	   	struct addrinfo hints, *res;

	   	char buf2[TAM_BUFFER],buf[TAM_BUFFER];
	   	//char mensaje[1024];

		if (argc != 5) {
			fprintf(stderr, "uso:  %s <host> <protocolo> <accion> <fichero ordenes>\n", argv[0]);
			exit(1);
		}
		
			/* Create the socket. */
		s = socket (AF_INET, SOCK_DGRAM, 0);
		if (s == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to create socket\n", argv[0]);
			exit(1);
		}
		
	    /* clear out address structures */
		memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
		memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));
		
				/* Bind socket to some local address so that the
			 * server can send the reply back.  A port number
			 * of zero will be used so that the system will
			 * assign any available port number.  An address
			 * of INADDR_ANY will be used so we do not have to
			 * look up the internet address of the local host.
			 */
		myaddr_in.sin_family = AF_INET;
		myaddr_in.sin_port = 0;
		myaddr_in.sin_addr.s_addr = INADDR_ANY;
		if (bind(s, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
			perror(argv[0]);
			fprintf(stderr, "%s: unable to bind socket\n", argv[0]);
			exit(1);
		}



	    addrlen = sizeof(struct sockaddr_in);
	    if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
	        perror(argv[0]);
	        fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
	        exit(1);
	    }

	            /* Print out a startup message for the user. */
	    time(&timevar);
	            /* The port number must be converted first to host byte
	             * order before printing.  On most hosts, this is not
	             * necessary, but the ntohs() call is included here so
	             * that this program could easily be ported to a host
	             * that does require it.
	             */
	    printf("Connected to %s on port %u at %s", argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));




		/* Set up the server address. */
		servaddr_in.sin_family = AF_INET;
			/* Get the host information for the server's hostname that the
			 * user passed in.
			 */
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_INET;
	 	 /* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
	    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
	    if (errcode != 0){
				/* Name was not found.  Return a
				 * special value signifying the error. */
			fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
					argv[0], argv[1]);
			exit(1);
	    }
	    else {
				/* Copy address of host */
			servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
		}
	    freeaddrinfo(res);
	    /* puerto del servidor en orden de red*/
		servaddr_in.sin_port = htons(PUERTO);

	   	/* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
	    vec.sa_handler = (void *) handler;
	    vec.sa_flags = 0;
	    if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
	        perror(" sigaction(SIGALRM)");
	        fprintf(stderr,"%s: unable to register the SIGALRM signal\n", argv[0]);
	        exit(1);
	    }
		
	    n_retry=RETRIES;

	    
		///----------------------BLOQUE DE CODIGO API C - UDP

		///----------------------BLOQUE DE CODIGO escribir (enviar archivo) - UDP

	    if(!strcmp(argv[3],"e")){
			printf("Estoy dentro de escritura de fichero\n");

			//variables a usar
			char mensaje[1024],aux[1024];
   			//memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
   			//Memset (aux, '\0', sizeof (aux)); 	//reset aux
			mensaje[0] = aux[0] = '\0';

			//PRIMERO creamos la peticion de escritura

			sprintf(mensaje,"02%s0octet0",argv[4]);
			printf("%s\n", mensaje);




			while (n_retry > 0) {
		        if (sendto (s, mensaje, strlen(mensaje), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		        		perror(argv[0]);
		        		fprintf(stderr, "%s: unable to send request\n", argv[0]);
		        		exit(1);
		        	}
				sleep(1);
			    alarm(TIMEOUT); //SE AJUSTA EL TIMEOUT
		        if (recvfrom (s, buf2, sizeof(buf2), 0,
								(struct sockaddr *)&servaddr_in, &addrlen) == -1) {
		    		/*if (errno == EINTR) {
		 		         printf("attempt %d (retries %d).\n", n_retry, RETRIES);
		  	 		     n_retry--; 
		                    } 
		            else  {
						printf("Unable to get response from");
						exit(1); 
		                }*/
		              } 
		        else {
					printf("\n\n>>>RESPUESTA DEL SERVIDOR:\n%s\n\n",buf2);

		            alarm(0);
		            break;	
	            }
		  	}

		    if (n_retry == 0) {
		       printf("Unable to get response from");
		       printf(" %s after %d attempts.\n", argv[1], RETRIES);
		    }




			if(buf2[1] != '4'){	//Servidor nos avisa si ya tiene el archivo o se le puede enviar.

				printf("El fichero ya existe.\n"); 
			
			}else{

				//COMENZAMOS A MANDAR EL ARCHIVO POR BLOQUES

				FILE *f;
				
				//Establecimiento de la ruta
				char ruta[150];
				memset (ruta, '\0', sizeof (ruta)); 	//reset ruta
				//ruta[0]='\0';
				sprintf(ruta,"ficherosTFTPclient/");
				strcat(ruta,argv[4]);
				printf("\n\nruta: %s\n\n",ruta);


				//Abrimos fichero		
				if((f = fopen(ruta,"r")) == NULL) 
				{
		  			printf("404: Not found");
				}else{ //200 OK
				 	printf("200: OK");
				 	int j = 0; //para contar las iteraciones
	 				int nume = 512; //NUMERO BITS LEIDOS (inicializamos en 512 para entrar en el while)
					while(nume == 512){
						j++;
						memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
			   			memset (aux, '\0', sizeof (aux)); 	//reset aux

			   			//mensaje[0]='\0';
			   			//aux[0]='\0';
					
						//CANTIDADDEBYTESLEIDOS=FREAD(CONTENIDO,NºBYTESXELEMENTO,NUMERODEELEMENTOS,FICHERO)

						nume = fread(&mensaje,1,512,f);	
						printf("\n\nLEEMOS DE ARCHIVO %d: \n\n",nume);
						//printf("%s\n",mensaje);

						//CREAMOS EL MENSAJE

						//Como todo va sobre ruedas, vamos a darle formato
						if(j < 10) sprintf(aux,"030%d",j);
						else sprintf(aux,"03%d",j);

						printf("%s\n",aux);
						sleep(3);
						strcat(aux,mensaje);
						printf("\nNUMERODEBYTESMENSAJE:%ld\n",strlen(aux));
						sleep(1);

						//MANDAMOS EL MENSAJE
						memset (buf, '\0', sizeof (buf)); 	//reset mensaje
						strcpy(buf,aux);

						//-------------------------ENVIAMOS EL MENSAJE

						while (n_retry > 0) {
					        if (sendto (s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
					        		perror(argv[0]);
					        		fprintf(stderr, "%s: unable to send request\n", argv[0]);
					        		exit(1);
					        	}
							sleep(1);
						    alarm(TIMEOUT); //SE AJUSTA EL TIMEOUT
					        if (recvfrom (s, buf2, sizeof(buf2), 0,
											(struct sockaddr *)&servaddr_in, &addrlen) == -1) {
					    		/*if (errno == EINTR) {
					 		         printf("attempt %d (retries %d).\n", n_retry, RETRIES);
					  	 		     n_retry--; 
					                    } 
					            else  {
									printf("Unable to get response from");
									exit(1); 
					                }*/
					              } 
					        else {
								printf("\n\n>>>RESPUESTA DEL SERVIDOR:\n%s\n\n",buf2);

					            alarm(0);
					            break;	
				            }
					  	}

					    if (n_retry == 0) {
					       printf("Unable to get response from");
					       printf(" %s after %d attempts.\n", argv[1], RETRIES);
					    }

					}//FIN WHILE CONTENIDO

					fclose(f);

				}//FIN FICHERO

			}//FIN Bloque de confirmado (Servidor)

			sleep(5);//sleep de comprobacion

		}//FIN ESCRITURA FICHERO



		///----------------------BLOQUE DE CODIGO leer (recibir archivo) - UDP
		
		if(!strcmp(argv[3],"l")){
			printf("Estoy dentro de lectura de fichero\n");

			//variables a usar
			char mensaje[1024],aux[1024];
   			//memset (mensaje, '\0', sizeof (mensaje)); 	//reset mensaje
   			//Memset (aux, '\0', sizeof (aux)); 	//reset aux
			mensaje[0] = aux[0] = '\0';


			//Primero se mira si el cliente tiene el fichero para no sobreescribir
			FILE *f;
				
			//Establecimiento de la ruta
			char ruta[150];
			memset (ruta, '\0', sizeof (ruta)); 	//reset ruta
			//ruta[0]='\0';
			sprintf(ruta,"ficherosTFTPclient/");
			strcat(ruta,argv[4]);
			printf("\n\nruta: %s\n\n",ruta);

			if(existe(ruta)){
					//mensaje ERROR NO ENCONTRADO
					printf("Cuidado, ya tiene un archivo con este nombre.\n"); //bloque 0
					//break;
			}else{

				//Creamos la peticion de escritura

				sprintf(mensaje,"01%s0octet0",argv[4]);
				printf("%s\n", mensaje);

				while (n_retry > 0) {
			        if (sendto (s, mensaje, strlen(mensaje), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
			        		perror(argv[0]);
			        		fprintf(stderr, "%s: unable to send request\n", argv[0]);
			        		exit(1);
			        	}
					sleep(1);
				    alarm(TIMEOUT); //SE AJUSTA EL TIMEOUT
			        if (recvfrom (s, buf2, sizeof(buf2), 0,
									(struct sockaddr *)&servaddr_in, &addrlen) == -1) {
			    		/*if (errno == EINTR) {
			 		         printf("attempt %d (retries %d).\n", n_retry, RETRIES);
			  	 		     n_retry--; 
			                    } 
			            else  {
							printf("Unable to get response from");
							exit(1); 
			                }*/
			              } 
			        else {
						printf("\n\n>>>RESPUESTA DEL SERVIDOR:\n%s\n\n",buf2);

			            alarm(0);
			            break;	
		            }
			  	}

			    if (n_retry == 0) {
			       printf("Unable to get response from");
			       printf(" %s after %d attempts.\n", argv[1], RETRIES);
			    }




				if(buf2[1] != '3'){
					printf("No tiene el archivo.\n");
				}else{

					//COMENZAMOS A RECIBIR EL ARCHIVO POR BLOQUES

					//Variables:
					int j;
					int tammen = 516;
					

					while(tammen == 516){

						tammen = strlen(buf2);

						if((f = fopen(ruta,"a")) == NULL){
							perror("Fallo al crear/escribir documento.");
						}else{
							for (j = 4; j < tammen; ++j)
							{
								fputc(buf2[j],f);
							}
						}
						fclose(f);

						//mensaje asentimiento
						sprintf(buf,"04%c%c",buf2[2],buf2[3]);

						//-------------------------ENVIAMOS EL MENSAJE

						while (n_retry > 0) {
					        if (sendto (s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
					        		perror(argv[0]);
					        		fprintf(stderr, "%s: unable to send request\n", argv[0]);
					        		exit(1);
					        	}
							sleep(1);

							if(tammen != 516) break;

						    alarm(TIMEOUT); //SE AJUSTA EL TIMEOUT
					        if (recvfrom (s, buf2, sizeof(buf2), 0,
											(struct sockaddr *)&servaddr_in, &addrlen) == -1) {
					    		/*if (errno == EINTR) {
					 		         printf("attempt %d (retries %d).\n", n_retry, RETRIES);
					  	 		     n_retry--; 
					                    } 
					            else  {
									printf("Unable to get response from");
									exit(1); 
					                }*/
					              } 
					        else {
								printf("\n\n>>>RESPUESTA DEL SERVIDOR:\n%s\n\n",buf2);

					            alarm(0);
					            break;	
				            }
					  	}

					    if (n_retry == 0) {
					       printf("Unable to get response from");
					       printf(" %s after %d attempts.\n", argv[1], RETRIES);
					    }					


					} //FIN WHILE


				}

			}

		}


		
	}//FIN UDP
}




int existe(char *r)
{
	FILE *f;
  if((f = fopen(r, "r")) == NULL) return 0;
  /* Si existe, cerramos */
  fclose(f);
  return 1;
}
