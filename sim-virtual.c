#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tpAlgoritmo.h"

typedef struct pagina{
	int r;
	int m;
	time_t t;
	
} Pagina;


int main (int argc, char * argv[]){
	char *arq;
	int debug = 0;
	int tamPag, tamMem;
	TpAlgoritmo tpalgoritmo;
	arq = argv[2];
	printf ("1 %s, 2 %s, 3 %s, 4 %s,5 %s" , argv[1],argv[2],argv[3],argv[4],argv[5]);
	
	if (strcmp(argv[1],"LRU") == 0)  tpalgoritmo = LRU;
	else if (strcmp(argv[1],"NRU") == 0)  tpalgoritmo = NRU;
	else exit(1);
	
	tamPag = atoi(argv[3]);
	tamMem = atoi(argv[4]);
	
	if (argc == 6){
		printf ("Modo debug\n");	
		debug = 1;
	}
	else printf ("\n");
		
	printf ("debug %d, tamPag %d, tamMem %d, arq %s tpalg %d\n" , debug, tamPag, tamMem, arq, tpalgoritmo);
	return 0;
}
