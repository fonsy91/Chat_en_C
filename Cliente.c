#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[]){
    
    if (argc != 3){
        fprintf(stderr,"use: %s <ip> [port]\n",argv[0]);
        return -1; 
    }
    const char *ip = argv[1]; 
    unsigned short int port = atoi(argv[2]); 
    
    int sfd = socket(AF_INET,SOCK_STREAM,0);
    if (sfd == -1){
        perror("Error en creacion del socket del cliente");
        return -1; 
    }

    struct sockaddr_in addr; 
    addr.sin_family = AF_INET; 
    //htons convierte el entero que se le pasa al orden de bytes de la red
    addr.sin_port = htons(port); 
    //inet_addr(): convierte la cadena a la que apunta ip en una notacion
    //decimal estandar IPv4 en un entero adecuado para su uso como 
    //direccion de internet 
    addr.sin_addr.s_addr = inet_addr(ip); 
    socklen_t addrlen = sizeof(addr); 

    int ret = connect(sfd,(const struct sockaddr*)(&addr),addrlen); 
    if (ret == -1){
        perror("Error al realizar connect en cliente");
        return -1; 
    }
    printf("¡Conexion existosa del servidor!\n");
    printf("Ingrese su nombre de Chat: "); 
    char name[100]; 
    //gets(name); 
    fgets(name,100,stdin);
    send(sfd,name,strlen(name)+1,0);
    pid_t pid = fork(); 
    if (pid == -1){
        perror("Error al crear el fork en cliente"); 
        return -1; 
    }
    if (pid == 0){
        while (1){
            char buf[1024] = {}; 
            fgets(buf,1024,stdin);
            if (send(sfd,buf,strlen(buf)+1,0) <= 0){
                break; 
            }
        }
        
    }else{
        while (1){
            char buf[1024] = {}; 
            if (recv(sfd,buf,1024,0) <= 0){
                break; 
            }
            time_t timep; 
            time(&timep); 
            printf("%s\n",ctime(&timep)); 
            printf("%s\n",buf);
        }
        
    }

    return 0;
}
