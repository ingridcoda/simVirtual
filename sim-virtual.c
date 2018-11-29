#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pagina {
	int r; //referenciada
	int m; //modificada
	int t; //timer
	unsigned int endereco; //endereco da pagina	
} Pagina;

int contPageFaults = 0, contPagsSujas = 0, debug = 0;

int calculaShift(int tamPag){
	if(tamPag == 8){
		return 13;
	}
	else if(tamPag == 16){
		return 14;
	}
	else if(tamPag == 32){
		return 15;
	}
	return 0;
}

void imprimeVetor(Pagina ** tabelaPags, int tamVet){
	int i;

	for(i = 0; i < tamVet; i++){
		printf("Vetor posicao %d: Endereco %x R %d M %d T %d\n", i, tabelaPags[i]->endereco, tabelaPags[i]->r, tabelaPags[i]->m, tabelaPags[i]->t);
	}
}

Pagina ** substituiNRU(Pagina ** tabelaPags, int shift, int i, unsigned int tempEnd, char tempRW){
	tabelaPags[i]->endereco = tempEnd >> shift;
	tabelaPags[i]->r = 1;
	tabelaPags[i]->t = 0;

	if(tempRW == 'W') tabelaPags[i]->m = 1;
	else tabelaPags[i]->m = 0;

	if(debug) printf("NRU tempEnd %x endereco %x i %d\n", tempEnd, tabelaPags[i]->endereco, i);

	return tabelaPags;
}


Pagina ** trataNRU(Pagina ** tabelaPags, int shift, int tamVet, unsigned int tempEnd, char tempRW){
	int i, tipo = 5;

	if(debug) {
		printf("Vetor antes substituição NRU:\n");
		imprimeVetor(tabelaPags, tamVet);
	}

	for(i = 0; i < tamVet; i++){
		//Não referenciada, não modificada
		if(tabelaPags[i]->r == 0 && tabelaPags[i]->m == 0){
			if(tipo > 1) tipo = 1;
			break;
		}
		//Não referenciada, modificada
		else if(tabelaPags[i]->r == 0 && tabelaPags[i]->m){
			if(tipo > 2) tipo = 2;
			break;
		}
		//Referenciada, não modificada
		else if(tabelaPags[i]->r > 0 && tabelaPags[i]->m == 0){
			if(tipo > 3) tipo = 3;
			break;
		}
		//Referenciada, modificada
		else if(tabelaPags[i]->r > 0 && tabelaPags[i]->m){
			if(tipo > 4) tipo = 4;
			break;
		}
	}

	tabelaPags = substituiNRU(tabelaPags, shift, i, tempEnd, tempRW);

	if(tipo % 2 == 0) contPagsSujas++;

	if(debug) {
		printf("NRU: fim do tratamento\n");
		printf("Vetor depois substituição NRU:\n");
		imprimeVetor(tabelaPags, tamVet);
	}

	return tabelaPags;
}

Pagina ** trataLRU(Pagina ** tabelaPags, int shift, int tamVet, unsigned int tempEnd, char tempRW){
	int i, maior = 0;

	if(debug) {
		printf("Vetor antes substituição LRU:\n");
		imprimeVetor(tabelaPags, tamVet);
	}

	for(i = 0; i < tamVet; i++){
		if(tabelaPags[i]->t > tabelaPags[maior]->t){
			maior = i;
		}
	}

	if(tabelaPags[maior]->m == 1) contPagsSujas++;

	tabelaPags[maior]->endereco = tempEnd >> shift;
	tabelaPags[maior]->r = 1;
	tabelaPags[maior]->t = 0;

	if(tempRW == 'W') tabelaPags[maior]->m = 1;
	else tabelaPags[maior]->m = 0;

	if(debug) {
		printf("LRU tempEnd %x endereco %x maior %d\n", tempEnd, tabelaPags[maior]->endereco, maior);
		printf("LRU: fim do tratamento\n");
		printf("Vetor depois substituição LRU:\n");
		imprimeVetor(tabelaPags, tamVet);
	}
	return tabelaPags;	
}


int main (int argc, char * argv[]){
	char nomeArq[50]; 
	int i, tamPag, tamMem, tamVet, shift;
	char tpalgoritmo[3];
	Pagina ** tabelaPags;
	FILE * arq;
	unsigned int tempEnd;
	char tempRW;

	if (argc == 6) {
		printf ("Executando o simulador em modo debug...\n");	
		debug = 1;
	} 
	else printf ("Executando o simulador...\n");

	strcpy(tpalgoritmo, argv[1]);
	strcpy(nomeArq, argv[2]);	
	tamPag = atoi(argv[3]);
	tamMem = atoi(argv[4]);
	tamVet = tamMem / tamPag;
	shift = calculaShift(tamPag);

	tabelaPags = (Pagina **) malloc (tamVet * sizeof(Pagina*));
	if(tabelaPags == NULL) {
		printf("Erro ao tentar alocar memória para tabela de páginas.\n");
		exit(1);
	}

	for(i = 0; i < tamVet; i++) {
		tabelaPags[i] = (Pagina*) malloc (sizeof(Pagina));
		tabelaPags[i]->t = 0;
		tabelaPags[i]->endereco = 0;

		if(tabelaPags[i] == NULL) {
			printf("Erro ao tentar alocar memória para página %d.\n", i);
			exit(1);
		}
	}

	arq = fopen(nomeArq, "r");
	if(arq == NULL) {
		printf("Erro ao tentar abrir arquivo %s.\n", nomeArq);
		exit(1);
	}

	while(fscanf(arq, "%x %c\n", &tempEnd, &tempRW) != EOF) {
		if(debug) printf("Lido: %x %c\n", tempEnd, tempRW);
		for(i = 0; i < tamVet; i++){
			if(tabelaPags[i]->endereco != 0) (tabelaPags[i]->t)++;
			//Pagina ja existe na tabela, so referencia
			if(tabelaPags[i]->endereco != 0 && tabelaPags[i]->endereco == (tempEnd >> shift)) {
				if(debug) {
					printf("Vetor antes caso IF:\n");
					imprimeVetor(tabelaPags, tamVet);
					printf("IF tempEnd %x endereco %x\n", tempEnd, tabelaPags[i]->endereco);
				}
				(tabelaPags[i]->r)++;

				if(tempRW == 'W') tabelaPags[i]->m = 1;
				else tabelaPags[i]->m = 0;
				if(debug) {
					printf("Vetor depois caso IF:\n");
					imprimeVetor(tabelaPags, tamVet);
				}
				break;
			} 
			//Pagina nao existe na tabela, e tabela nao esta cheia, adiciona na tabela e referencia
			else if(tabelaPags[i]->endereco == 0) {
				if(debug) {
					printf("Vetor antes caso ELSE IF:\n");
					imprimeVetor(tabelaPags, tamVet);
				}
				tabelaPags[i]->endereco = tempEnd >> shift;
				(tabelaPags[i]->r)++;

				if(tempRW == 'W') tabelaPags[i]->m = 1;
				else tabelaPags[i]->m = 0;
				if(debug) {
					printf("ELSE IF tempEnd %x endereco %x\n", tempEnd, tabelaPags[i]->endereco);
					printf("Vetor depois caso ELSE IF:\n");
					imprimeVetor(tabelaPags, tamVet);
				}
				break;
			} 
			//Pagina nao existe na tabela, e tabela esta cheia, chama algoritmo de substituicao
			else if(tabelaPags[i]->endereco != 0 && tabelaPags[i]->endereco != (tempEnd >> shift) && i == (tamVet-1)){
				if(debug) printf("ELSE - PAGEFAULTS %d\n", contPageFaults);

				contPageFaults++;

				if(strcmp(tpalgoritmo, "LRU") == 0) {
					tabelaPags = trataLRU(tabelaPags, shift, tamVet, tempEnd, tempRW);
				} 
				else if(strcmp(tpalgoritmo, "NRU") == 0) {
					tabelaPags = trataNRU(tabelaPags, shift, tamVet, tempEnd, tempRW);
				} 
				else {
					printf("Erro: Algoritmo não identificado.\n");
					exit(1);
				}
				break;
			}
		}
	}

	printf("Arquivo de entrada: %s\n", nomeArq);
	printf("Tamanho da memoria fisica: %d\n", tamMem);
	printf("Tamanho das páginas: %d\n", tamPag);
	printf("Alg de substituição: %s\n", tpalgoritmo);
	printf("Numero de Faltas de Paginas: %d\n", contPageFaults);
	printf("Numero de Paginas escritas: %d\n", contPagsSujas);

	if(debug) {
		printf("Vetor final:\n");
		imprimeVetor(tabelaPags, tamVet);
	}

	return 0;
}
