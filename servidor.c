#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "tp_socket.h"

#define AMOSTRAS 1
#define TAMANHO_CABECALHO 31

int timeout = 0;

//void myalarm(int seg);
//void timer_handler(int signum);
//void settimer(void);
void intParaChar(int inteiro, char* vetor, int inicio, int termino);
int somaDeVerificacao(const char* buffer);
//int comparaSomas(const char* buffer);
int enviaPacote(int somaDeVerificacao, int tamanhoDados, int numero_de_sequencia, int ACK, int flag, char* buffer, int socket_des, so_addr* destino);

int main (int argc, char *argv[]){
	tp_init();

	int PORTA_SERVIDOR = atoi(argv[1]); // porta da conexão
	int LENGTH = atoi(argv[2]); // tamanho do buffer
	
	char *buffer = malloc(LENGTH*sizeof(char));
	char nomeArq[20];
	int i = 0;
	int bytes_lidos, bytes_sendto;
	int bytes_enviados = 0;
	int numero_de_sequencia = 0;
	int socket_des; // descritor do socket
	float media = 0;
	
	struct timeval inicio, fim;
	so_addr cliente;

	//Criando socket
	socket_des = tp_socket(PORTA_SERVIDOR);
	if (socket_des == -1){
		perror("socket ");
		exit(1);
	}else if(socket_des == -2){
		perror("build addr");
		exit(1);
	}else if (socket_des == -3){
		perror("bind");
		exit(1);
	}
	printf("Socket criado com sucesso\n");


	//Recebe nome do arquivo
	memset(buffer, 0x0, LENGTH);
	memset(nomeArq, 0x0, 20);
	unsigned int sock_len = sizeof(struct sockaddr_in);
	do {
		tp_recvfrom(socket_des,buffer,sizeof(char), &cliente);
		nomeArq[i] = buffer[0];
		i++;
	} while(buffer[0]!='0');
	nomeArq[i - 1] = '\0';
	printf("Nome do arquivo: %s\n", nomeArq);

	
	// Envia arquivo
	
	//==========IMPLEMENTAÇÃO ANTIGA===================
	/*FILE* fp = fopen((const char*) nomeArq, "r");
	memset(buffer, 0x0, LENGTH);
	while((bytes_lidos=fread(buffer,sizeof(char),LENGTH, fp)) > 0){
		bytes_sendto = tp_sendto(socket_des, buffer, bytes_lidos, &cliente);
		bytes_enviados+=bytes_sendto;
		memset(buffer, 0x0, LENGTH);
	};*/

	//1º Envia pacote com o máximo de dados permitido pelo buffer.
	FILE* fp = fopen((const char*) nomeArq, "r");
	memset(buffer, 0x0, LENGTH);
	bytes_lidos = fread(&buffer[TAMANHO_CABECALHO], sizeof(char), LENGTH-TAMANHO_CABECALHO, fp);
	bytes_sendto = enviaPacote(somaDeVerificacao(buffer), bytes_lidos, numero_de_sequencia, 0, 0, buffer, socket_des, &cliente);
	numero_de_sequencia += bytes_lidos;
	bytes_enviados += bytes_sendto;
	//2º Espera recebimento do ACK ou timeout
	//3º Recebendo pacote, verifica o ACK e envia o pacote correspondente
	//4º Com timeout, reenvia pacote
	//5º Faz isso até o final do arquivo, enviando flag 1 no final do arquivo.
	//6º Espera resposta com flag 1 e fecha tudo.

	
	//Encerra e limpa a memória
	printf("Conexão encerrada\n");
	fclose(fp);
	close(socket_des);
	free(buffer);
	return 0;
}

/*void myalarm(int seg){
	alarm(1);
}*/

/*void timer_handler(int signum){
	printf("Error: Timeout\n");
	timeout = 1;
}*/

/*void settimer(void){
		signal(SIGALRM,timer_handler);
		myalarm(1);
}*/

void intParaChar(int inteiro, char* vetor, int inicio, int termino){
    for(int i = termino; i >= inicio; i--){
    int aux = inteiro%10;
    inteiro = inteiro/10;
        switch (aux){
            case 0:
                vetor[i] = '0';
                break;
            case 1:
                vetor[i] = '1';
                break;
            case 2:
                vetor[i] = '2';
                break;
            case 3:
                vetor[i] = '3';
                break;
            case 4:
                vetor[i] = '4';
                break;
            case 5:
                vetor[i] = '5';
                break;
            case 6:
                vetor[i] = '6';
                break;
            case 7:
                vetor[i] = '7';
                break;
            case 8:
                vetor[i] = '8';
                break;
            case 9:
                vetor[i] = '9';
                break;
        }
    }
}

int somaDeVerificacao(const char* buffer){
    int soma_buffer = 0;

    for (int i = TAMANHO_CABECALHO; i < strlen(buffer); i++){
        soma_buffer += (int) buffer[i];
        
        //131071(base 10) = 1111.1111.1111.1111(base 2)
        if (soma_buffer > 131071) soma_buffer -= 131071;
    
    }
    
    return soma_buffer;
}

/*int comparaSomas(const char* buffer){
	char* soma_verificacao = malloc(sizeof(char)*7);
	soma_verificacao = somaDeVerificacao(buffer);

    char* soma_cabecalho = malloc(sizeof(char)*7);
	strncpy(soma_cabecalho, buffer, 6);

    if (!strcmp(soma_verificacao, soma_cabecalho)){
        free(soma_verificacao);
		free(soma_cabecalho);
        return 1;
    }
    else{
		free(soma_verificacao);
        free(soma_cabecalho);
        return 0;
    }
}*/

int enviaPacote(int somaDeVerificacao, int tamanhoDados, int numero_de_sequencia, int ACK, int flag, char* buffer, int socket_des, so_addr* destino){
	char cabecalho[TAMANHO_CABECALHO];
	/*	cabecalho[0-5] -> Soma de verificação;
	  	cabecalho[6-9] -> Tamanho dados;
		cabecalho[10-19] -> Número de sequência;
		cabecalho[20-29] -> ACK.
		cabecalho[30] -> flag.
	*/
	intParaChar(somaDeVerificacao, cabecalho, 0, 5);
	intParaChar(tamanhoDados, cabecalho, 6, 9);
	intParaChar(numero_de_sequencia, cabecalho, 10, 19);
	intParaChar(ACK, cabecalho, 20, 29);
	intParaChar(flag, cabecalho, 30, 30);
	
	memcpy(buffer, cabecalho, TAMANHO_CABECALHO);

	return tp_sendto(socket_des, buffer, strlen(buffer), destino);
}