//Compilacion del programa:  gcc Servidor.c  -o Servidor -pthread
//Ejecucion del programa:  ./Servidor [ip] [puerto], Cliente [ip] [puerto]
//Librerias necesarias 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
//para tener disponiblela estructura in_addr
#include <arpa/inet.h>
//Para los tipos in_port_t(16 bits) y in_addr_t(32 bits)
#include <netinet/in.h>
#define MAX 100 



//Estructura de tipo cliente 
typedef struct Client{
    int cfd; 
    char name[40]; 
}Client;

//Numero maximo de clientes que se pueden albergar 
Client client[MAX] = {}; 

//size_t: un tipo de datos sin signo nunca representa un valor negativo
size_t cnt = 0; 

//Inicializacion del mutex 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 


//Desarrollo de la funciones *************************************************
//Funcion de transimision 
void broadcast(char *msg, Client c){
    size_t i; 
    //Bloquea el mutex si no lo tiene nadie. Si alguien tiene bloqueado el 
    //mutex el proceso espera hasta que el que lo tiene lo libera 
    pthread_mutex_lock(&mutex); 
    for (i = 0; i < cnt; i++){
        if (client[i].cfd != c.cfd){
            //send en caso de exito devuelve el nº de bytes enviados, error -1
            if (send(client[i].cfd,msg,strlen(msg),0) <= 0){
                //En caso de error se sale 
                break; 
            }
        }
    }
    //desbloqueamos el mutex 
    pthread_mutex_unlock(&mutex); 
} 

//Funcion de procesamiento de hilos, recepcion, procesamiento y reenvio de 
//mensajes 
void *pthread_run(void *arg){
    //casteamos arg a tipo estructura Cliente 
    Client cl = *(Client*)(arg); 
    while (1){
        char buf[1024]={};
        strcpy(buf,cl.name);
        //concatena (:) con lo que hay en buffer  
        strcat(buf," :");
        //recibimos el socket, recv devuelve el nº de bytes recibidos -1 error
        int ret = recv(cl.cfd,buf+strlen(buf),1024-strlen(buf),0);
        if (ret <= 0){
            size_t i; 
            for (i = 0; i < cnt; i++){
                if (client[i].cfd == cl.cfd){
                    client[i] = client[cnt-1];
                    --cnt;  
                    strcpy(buf,"Tu amigo");
                    strcat(buf,cl.name);
                    strcat(buf,"dejar");
                    break; 
                }
            }
            broadcast(buf,cl);
            close(cl.cfd);
            return NULL;
        }else{
            broadcast(buf,cl);
        }
    }
}

int main(int argc, char const *argv[]){
    if (argc != 3){
        //stderr se usa para registrar mensajes de error standar
        //al compilar si no introduces los valores adecuados saldra este error 
        fprintf(stderr, "use: %s <ip> [port]\n",argv[0]);
        return -1; 
    }
    //guardamos la ip del segundo argumento 
    const char *ip = argv[1]; 
    //Guardamos el puerto del tercer argumento atoi convierte una cadena 
    //a su valor numerico 
    unsigned short int port = atoi(argv[2]); 

    int sfd = socket(AF_INET,SOCK_STREAM,0); 
    if (sfd == -1){
        perror(" Error de socket");
        return -1; 
    }

    //rellenamos la estructura addr
    struct sockaddr_in addr; 
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    //s_addr es una variable que contiene la información sobre la dirección que aceptamos.
    addr.sin_addr.s_addr = inet_addr(ip); 
    //socket_t dato de tipo entero unsigned de 32 bits  
    socklen_t addrlen = sizeof(addr); 
    
    //Con bind() asociamos un socket con un puerto 
    int ret = bind(sfd,(struct sockaddr*)(&addr),addrlen); 
    if (ret == -1){
        perror("Error al hacer el bind()");
        return -1; 
    }
    if (listen(sfd,10) == -1){
        perror("Error al hacer el listen()"); 
        return -1; 
    }

    //Una vez que arrancas el servidor se queda a la espera de la conexion 
    //de algun cliente
    while (1){
        struct sockaddr_in caddr; 
        socklen_t len = sizeof(caddr); 
        printf("Esperando la conexion del cliente ...\n"); 
        //aceptamos una solicitud de conexion de un cliente 
        int cfd = accept(sfd,(struct sockaddr*)(&caddr),&len);
        if (cfd == -1){
            perror("Error en funcion acept()");
            return -1; 
        }
        char buf[100] = {}; 
        //recibira un mensaje de un conector 
        recv(cfd,&client[cnt].name,40,0);
        client[cnt].cfd = cfd; 
        pthread_t id; 
        strcpy(buf,"Tu amigo");
        strcat(buf,client[cnt].name);
        strcat(buf,"Ir en línea");
        broadcast(buf,client[cnt]); 
        ret = pthread_create(&id,NULL,pthread_run,(void*)(&client[cnt]));
        cnt++; 
        if (ret != 0){
            printf("pthread_create: %s\n",strerror(ret));
            //continue hace que se vuelva inmediatamente a la condicion de 
            //bucle while 
            continue; 
        }
        printf("Un cliente se conecto correctamente: ip<%s> puerto [%hu]\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));

    }
    
    return 0;
}
//FIN DEL MAIN*************

