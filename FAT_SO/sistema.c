#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "estrutura.h"

#define SECTOR_SIZE	                512
#define CLUSTER_SIZE	            2*SECTOR_SIZE
#define ENTRY_BY_CLUSTER            CLUSTER_SIZE /sizeof(dir_entry_t)
#define NUM_CLUSTER	                4096
#define STRINGS_SIZE 200

void init(){

	FILE* FILE = fopen("fat.part","wb");
	
	for (int i = 0; i < CLUSTER_SIZE; ++i) boot_block[i] = 0xbb;

	fwrite(&boot_block, sizeof(boot_block), 1,FILE);

    
	fat[0] = 0xfffd;
	for (int i = 1; i < 9; ++i){
		fat[i] = 0xfffe; 
	}

	fat[9] = 0xffff;
	for (int i = 10; i < NUM_CLUSTER; ++i){
		fat[i] = 0x0000;
	}

	fwrite(&fat, sizeof(fat), 1, FILE);
	fwrite(&root_dir, sizeof(root_dir), 1,FILE); 

	for (int i = 0; i < 4086; ++i){
		fwrite(&clusters, sizeof(data_cluster), 1, FILE);
	}

	fclose(FILE);
}

int load(){ 

	FILE* arq= fopen("fat.part", "rb");
	int i;
	if (arq == NULL){
		printf("ERRO NA LEITURA\n");
		return 0;
	}
	fseek(arq, sizeof(boot_block), SEEK_SET); 
	fread(arq, sizeof(fat), 1, arq);
	fclose(arq);

}

void atualizar_fat(){

	FILE *arq = fopen("fat.part", "rb+");
	if (arq == NULL){
	
		printf("ERRO\n");
		exit(1);
	}
	fseek(arq, sizeof(boot_block), SEEK_SET);
	fwrite(fat, sizeof(fat), 1, arq);
	fclose(arq);
}

void save_cluster(int index, data_cluster cluster){

	FILE *arq = fopen("fat.part", "rb+");
	if (arq == NULL){
		printf("ERRO\n");
		exit(1);
	}
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET); 
	fwrite(&cluster, CLUSTER_SIZE, 1, arq);	
	// fseek(arq, index*sizeof(data_cluster), SEEK_SET);
	// fwrite(cluster, sizeof(data_cluster), 1, arq);
	fclose(arq);
}

data_cluster ler_cluster(int index){

	FILE *arq = fopen("fat.part", "rb");
	data_cluster cluster;
	if(arq == NULL){
		printf("ERRO AO ABRIR\n");
		exit(1);
	}
	memset(&cluster, 0x00, sizeof(data_cluster));				
	fseek(arq, index *sizeof(data_cluster), SEEK_SET);
	fread(&cluster, sizeof(data_cluster), 1, arq);
	fclose(arq);
	return cluster;
}

int procurar_dir(char *diretorio, char *dir_atual, int procura){
	int index = 9, j = 0, k = 0;
	int indexPai = index;
	if (strcmp(diretorio, "/") == 0){							
		return index;
	}

	if(diretorio[0] != '/'){
		printf("PARAMETRO INVALIDO\n");
		return -1;
	}
	int qtd_dir = num_dir(diretorio);
	data_cluster data = ler_cluster(index);					 
	for(int i = 1; i <= strlen(diretorio); i++){																																					 
		if(diretorio[i] == '/' || diretorio[i] == '\0' || diretorio[i] == '\n'){																																						 
			dir_atual[j] = '\0';																										
			for(k = 0; k < 32; k++){
				if(strcmp(data.dir[k].filename, dir_atual) == 0 && data.dir[k].first_block != 0){
					if(data.dir[k].attributes == 1){																
						index = data.dir[k].first_block; 
						data = ler_cluster(index);				 
						break;
					}else{
						if(qtd_dir == 0 && procura == 3){ 
							return index;
						}
					}
				}
			}
			if(procura == 1){ 
				if(k == 32 || qtd_dir == 0){ 
					if(qtd_dir != 0){
						printf("DIRETORIO NAO ENCONTRADO\n");
						return -1;
					}
					if(k < 32){
						return indexPai;
					}
					return index;
				}
			}else if(procura == 2){
				if(k == 32 && qtd_dir == 0){
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;

				}else if(k < 32 && qtd_dir == 0){ 
					return index;
				}
			}
			qtd_dir--;	
			j = 0;					
			indexPai = index; 

		}else{
			dir_atual[j] = diretorio[i]; 
			j++;
		}
	}
	return -1;
}

int num_dir(char *caminho){										
	int qtd_dir = 0;
	int i = 0;						 
	do{ 
		if (caminho[i] == '/'){
			qtd_dir++;
		}
		i++;
	}while (caminho[i] != '\0' && caminho[i] != '\n');
	return qtd_dir - 1;
}

void separa(char *str1, char *str2, char *str3, char *separador){
    int tam = strlen(str1);
	char *aux;
	if (strcmp(str1, "") == 0){
		strcpy(str2, "");
		strcpy(str3, "");
		return;
	}	
	strcpy(str2, strtok(str1, separador));
	if (tam == strlen(str2)){
		strcpy(str3, "");
	}else{
		if (strcmp(separador, "/") == 0){
			strcpy(str3, "/");
			aux = strtok(NULL, "\n"); 
			if (aux == NULL){
				strcpy(str3, "");
			}else{
				strcat(str3, aux);
			}
		}else{
			aux = strtok(NULL, "\n");
			if (aux == NULL){
				strcpy(str3, "");
			}else{
				strcpy(str3, aux);
			}
		}
	}
}

data_cluster *qtd_cluster(char *string, int *numClusters){
	data_cluster *clusters;
	int tam_str, numClustersInteiros, numClustersFalta;
	tam_str = strlen(string);
	numClustersInteiros = tam_str / CLUSTER_SIZE;
	numClustersFalta = tam_str % CLUSTER_SIZE;

	if (numClustersInteiros != 0){
		int resto = 0, i = 0;
		if (numClustersFalta >= 0){
			resto = 1; 
		}
		clusters = (data_cluster *)malloc((numClustersInteiros + resto) * CLUSTER_SIZE);
		while(i < numClustersInteiros){																																				
			memset(&(clusters)[i], 0x00, CLUSTER_SIZE);													
			memcpy(&(clusters)[i].data, &string[(CLUSTER_SIZE)*i], CLUSTER_SIZE);
			((clusters)[i].data)[CLUSTER_SIZE] = '\0';	
            i++;				
		}
		if (resto == 1){																																							
			memset(&(clusters)[i], 0x00, CLUSTER_SIZE);	
			memcpy(&(clusters)[i].data, &string[i * (CLUSTER_SIZE)], numClustersFalta);
			(clusters)[i].data[numClustersFalta] = '\0';																
		}
		(*numClusters) = numClustersInteiros + resto;
		return clusters;
	}else{ 
		clusters = (data_cluster *)malloc(CLUSTER_SIZE);
		memset(clusters, 0x00, CLUSTER_SIZE);			 
		memcpy(&(clusters)[0], string, tam_str);
		(*numClusters) = 1;
		return clusters;
	}
}

void ls(char *diretorio){
	if (diretorio == NULL){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurar_dir(diretorio, dir_atual, 2);
	if (index != -1){
		data_cluster data = ler_cluster(index);
		printf("DIRETORIOS:\n");
		for (int i = 0; i < 32; i++){
			if (data.dir[i].first_block != 0 && data.dir[i].attributes == 1){
				printf("%s ", data.dir[i].filename);
			}
		}
		printf("\nARQUIVOS\n");
		for (int i = 0; i < 32; i++){
			if (data.dir[i].first_block != 0 && data.dir[i].attributes == 0){
				printf("%s ", data.dir[i].filename);
			}
		}
		printf("\n");
	}
	free(dir_atual);
}

void mkdir(char *diretorio){
	if (diretorio == NULL || strcmp(diretorio, "") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	if(strcmp(diretorio, "/") == 0){
		printf("NAO E POSSIVEL CRIAR O DIRETORIO RAIZ\n");
		return;
	}

	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurar_dir(diretorio, dir_atual, 1);
	if (index != -1){
		if (strcmp(dir_atual, "") != 0 && strcmp(dir_atual, "/") != 0 && strcmp(dir_atual, " ") != 0){ 
			data_cluster data = ler_cluster(index);
			data_cluster novoDiretorio;
			int index_bloco=10, j=0;
			memset(novoDiretorio.dir, 0x00, 32 * sizeof(dir_entry_t));
			while(j < 32){
				if (strcmp(data.dir[j].filename, dir_atual) == 0 && data.dir[j].attributes == 1){
					printf("DIRETORIO JA EXISTE\n");
					free(dir_atual);
					return;
				}
				if(data.dir[j].first_block == 0){
					break;
				}
                j++;
			}
			if (j == 32){
				printf("DIRETORIO CHEIO\n");
				free(dir_atual);
				return;
			}
			if (strlen(dir_atual) < 18){		
                strncpy(data.dir[j].filename, dir_atual, strlen(dir_atual));						
			}
			else{
				strncpy(data.dir[j].filename, dir_atual, 17);
			}
			data.dir[j].attributes = 1;
			while(index_bloco < 4096){
				if (fat[index_bloco] == 0x00){
					data.dir[j].first_block = index_bloco;
					fat[index_bloco] = 0xffff;
					atualizar_fat();														
					save_cluster(index, data);							
					save_cluster(index_bloco, novoDiretorio);
					free(dir_atual);
					return;
				}
                index_bloco++;
			}
		}else{
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dir_atual);
}

void create(char *diretorio){
	if (diretorio == NULL || strcmp(diretorio, "") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurar_dir(diretorio, dir_atual, 1);
	if (index != -1){
		if (strcmp(dir_atual, "") != 0 && strcmp(dir_atual, "/") != 0 && strcmp(dir_atual, " ") != 0){
			data_cluster data = ler_cluster(index);
			data_cluster novo_arq;
			int index_bloco=10, j=0;
			memset(novo_arq.data, 0x00, CLUSTER_SIZE);
			while (j < 32){
				if (strcmp(data.dir[j].filename, dir_atual) == 0 && data.dir[j].attributes == 0){ 
					printf("ARQUIVO JA EXISTE\n");
					free(dir_atual);
					return;
				}
				if (data.dir[j].first_block == 0){ 
					break;
				}
                j++;
			}
			if (j == 32){
				printf("DIRETORIO CHEIO\n");
				free(dir_atual);
				return;
			}
			if (strlen(dir_atual) < 18){																														 //se o nome do arquivo for maior que o limite 18 caracters é tratado no else
				strncpy(data.dir[j].filename, dir_atual, strlen(dir_atual)); 
			}else{
				strncpy(data.dir[j].filename, dir_atual, 17);
			}
			data.dir[j].attributes = 0; //0 - arquivo
			while(index_bloco < 4096){ 
				if (fat[index_bloco] == 0x00){
					data.dir[j].first_block = index_bloco;
					fat[index_bloco] = 0xffff;
					atualizar_fat();													
					save_cluster(index, data);							
					save_cluster(index_bloco, novo_arq);
					free(dir_atual);
					return;
				}
                index_bloco++;
			}
		}else{
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dir_atual);
}

void unlink(char *diretorio){
	if (diretorio == NULL || strcmp(diretorio, "") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	if (strcmp(diretorio, "/") == 0){
		printf("NAO FOI POSSIVEL A EXCLUSAO\n");
		return;
	}
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurar_dir(diretorio, dir_atual, 1);
	if(index != -1){
		if(strcmp(dir_atual, "") != 0 && strcmp(dir_atual, "/") != 0 && strcmp(dir_atual, " ") != 0){
			data_cluster aux;
			data_cluster data = ler_cluster(index); 
			dir_entry_t dirVazio;									 
			int j=0, i=0;
			memset(&dirVazio, 0x00, 32);
			while(i < 32){
				if (data.dir[i].first_block != 0 && strcmp(dir_atual, data.dir[i].filename) == 0){
					if (data.dir[i].attributes == 1){
						aux = ler_cluster(data.dir[i].first_block);
						while(j < 32){
							if (aux.dir[j].first_block != 0){ 
								break;
							}
                            j++;
						}
						if (j == 32){
							fat[data.dir[i].first_block] = 0x00;
							data.dir[i] = dirVazio;							 
							save_cluster(index, data);					 
							atualizar_fat();											 
							printf("DIRETORIO APAGADO\n");
							free(dir_atual);
							return;
						}else{						
	
							printf("ESVAZIE O DIRETORIO\n");
							free(dir_atual);
							return;
						}
					}
					if (data.dir[i].attributes == 0){					
						int indexaux = data.dir[i].first_block;
						//indexaux = fat[indexaux];
						while (fat[indexaux] != 0xffff){
							j = indexaux;
							indexaux = fat[indexaux];
							fat[j] = 0;
						}
						//data.dir[i].first_block
						fat[data.dir[i].first_block] = 0x00; 
						fat[indexaux] = 0x00;								 
						data.dir[i] = dirVazio;							 
						save_cluster(index, data);					 
						atualizar_fat();											 
						printf("ARQUIVO APAGADO\n");
						free(dir_atual);
						return;
					}
					printf("ERRO DE ATTRIBUTES\n");
				}
                i++;
			}
			if (i == 32){
				printf("DIRETORIO NAO ENCONTRADO\n");
			}
		}else{		
			printf("PARAMETRO INVALIDO\n");
		}
	}
	free(dir_atual);
}

void write(char *parametros){
	if (parametros == NULL || strcmp(parametros, "") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *diretorio = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *string = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	separa(parametros, string, diretorio, "/");
	if (strcmp(string, "") == 0){
		printf("STRING VAZIA\n");
		free(diretorio);
		free(string);
		free(dir_atual);
		return;
	}
	int index;
	index = procurar_dir(diretorio, dir_atual, 3);
	if (index != -1){
		data_cluster data = ler_cluster(index);
		int i;
		for (i = 0; i < 32; i++){
			if (strcmp(data.dir[i].filename, dir_atual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0){
				index = data.dir[i].first_block;
				break;
			}
		}
		if (i == 32){ 
			printf("ARQUIVO NAO ENCONTRADO\n");
			free(diretorio);
			free(string);
			free(dir_atual);
			return;
		}
		int numClusters;
		data_cluster *clusters;
		clusters = qtd_cluster(string, &numClusters);
		int auxindex = index, index_bloco;
		while (fat[auxindex] != 0xffff){
			index_bloco = auxindex;
			auxindex = fat[auxindex];
			fat[index_bloco] = 0x00;
		}
		fat[auxindex] = 0x00;
		fat[index] = 0xffff;
		save_cluster(index, clusters[0]);
		if (numClusters > 1){
			for (i = 1; i < numClusters; i++){
				for (index_bloco = 10; index_bloco < 4096; index_bloco++){
					if (fat[index_bloco] == 0x00)
					{
						break;
					}
				}
				fat[index] = index_bloco;
				index = index_bloco;
				fat[index] = 0xffff;
				save_cluster(index, clusters[i]); 
			}
		}
		atualizar_fat();
	}else{
		printf("ARQUIVO NAO ENCONTRADO\n");
	}
	free(diretorio);
	free(string);
	free(dir_atual);
}

void append(char *parametros){
	if (parametros == NULL || strcmp(parametros, "") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *diretorio = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *string = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	separa(parametros, string, diretorio, "/");
	if (strcmp(string, "") == 0){
		printf("STRING VAZIA\n");
		free(diretorio);
		free(string);
		free(dir_atual);
		return;
	}
	int index;
	index = procurar_dir(diretorio, dir_atual, 3); 
	if (index != -1){																				 
		data_cluster data = ler_cluster(index);
		int i, tam_arq;
		for (i = 0; i < 32; i++){
			if (strcmp(data.dir[i].filename, dir_atual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0){
				break;
			}
		}
		if (i == 32){
			printf("ARQUIVO NAO ENCONTRADO\n");
			free(diretorio);
			free(string);
			free(dir_atual);
			return;
		}
		while (fat[index] != 0xffff){
			index = fat[index];
		}
		data = ler_cluster(index);		
		tam_arq = strlen(data.data);
		if (1024 > tam_arq + strlen(string)){														 
			strcat(data.data, string);
			save_cluster(index, data);
		}else{
			data_cluster *clusters;
			int numClusters = 0, index_bloco, tam;
			tam = 1023 - tam_arq;
			strncat(data.data, string, tam);																			
			save_cluster(index, data);																							
			clusters = qtd_cluster(&string[1024 - tam_arq], &numClusters); 
			for (index_bloco = 10; index_bloco < 4096; index_bloco++){
				if (fat[index_bloco] == 0x00){ 
					break;
				}
			}
			fat[index] = index_bloco;
			fat[index_bloco] = 0xffff;
			save_cluster(index_bloco, clusters[0]);
			if (numClusters > 1){
				for (i = 1; i < numClusters; i++){ 
					int index_bloco=10;
					while(index_bloco < 4096){
						if (fat[index_bloco] == 0x00){
							break;
						}
                        index_bloco++;
					}
					fat[index] = index_bloco;
					index = index_bloco;
					fat[index] = 0xffff;
					save_cluster(index, clusters[i]);
				}
			}
			atualizar_fat();
		}
	}else{
		printf("ARQUIVO NAO ENCONTRADO\n");
	}
	free(diretorio);
	free(string);
	free(dir_atual);
}

void read(char *diretorio){
	if (diretorio == NULL || strcmp(diretorio, "") == 0 || strcmp(diretorio, "/") == 0){
		printf("CAMINHO INVALIDO\n");
		return;
	}
	char *dir_atual = (char *)malloc(sizeof(char) * STRINGS_SIZE);
	int index;
	index = procurar_dir(diretorio, dir_atual, 3);
	if (index != -1){
		data_cluster data = ler_cluster(index);
		int i=0, tam_arq;
		while(i < 32){
			if (strcmp(data.dir[i].filename, dir_atual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0){
				break;
			}
            i++;
		}
		if (i == 32){
			printf("ARQUIVO NAO ENCONTRADO\n");
			free(dir_atual);
			return;
		}
		data = ler_cluster(index);
		uint8_t arquivo[1024];										
		snprintf(arquivo, 1024, "%s", data.data);
		printf("%s", arquivo);
		while (fat[index] != 0xffff){
			index = fat[index];
			data = ler_cluster(index);
			snprintf(arquivo, 1024, "%s", data.data);
			printf("%s", arquivo);
		}
	}else{
		printf("ARQUIVO NAO ENCONTRADO");
	}
	free(dir_atual);
	printf("\n");
}

void help() {
	printf("-inicializar o sistema:\tinit\n");
	printf("-carregar o sistema:\tload\n");
	printf("-listar diretorio:\tls\n");
	printf("-criar diretorio:\tmkdir \n");
	printf("-criar arquivo:\tcreate\n");
	printf("-excluir arquivo ou diretorio:\tunlink\n");
	printf("-escrever em um arquivo:\twrite\n");
	printf("-anexar dados em um arquivo:\tappend\n");
	printf("-ler o conteudo de um arquivo:\tread\n");
	printf("-exibe a ajuda:\thelp\n");
	printf("-sair:\texit\n");
}

void command(){
   
  	char nome[4096] = { 0 };
    char nome2[4096] = { 0 };
	char nomecpy[4096] = { 0 };
	const char aux[2] = "/";
	char aux2[4096] = { 0 };

	char *token;
	int i;

	fgets(nome, 4096,stdin);

	strcpy(nomecpy,nome);

	token = strtok(nome,aux);

	if ( strcmp(token, "append ") == 0 && nomecpy[7] == '/'){
		for(i = 7; i < strlen(nomecpy)-1; ++i){
			aux2[i-7] = nomecpy[i];
		}
		printf("Digite o texto");
		fgets(nome2,4096,stdin);
		append(aux2);
	}else if ( strcmp(token, "create ") == 0 && nomecpy[7] == '/'){
		for(i = 7; i < strlen(nomecpy)-1; ++i){
			aux2[i-7] = nomecpy[i];
		}
		create(aux2);
	}else if ( strcmp(token, "init\n") == 0){
		init();
	}else if ( strcmp(token, "load\n") == 0){
		load();
	}else if ( strcmp(token, "ls ") == 0 && nomecpy[3] == '/'){
		for(i = 3; i < strlen(nomecpy)-1; ++i){
			aux2[i-3] = nomecpy[i];
		}
		ls(aux2);
	}else if( strcmp(token, "mkdir ") == 0 && nomecpy[6] == '/'){
		for(i = 6; i < strlen(nomecpy)-1; ++i){
			aux2[i-6] = nomecpy[i];
		}
		mkdir(aux2);
	}else if( strcmp(token, "read ") == 0 && nomecpy[5] == '/'){
		for(i = 5; i < strlen(nomecpy)-1; ++i){
			aux2[i-5] = nomecpy[i];
		}
		read(aux2);
	}else if( strcmp(token, "unlink ") == 0 && nomecpy[7] == '/'){
		for(i = 7; i < strlen(nomecpy)-1; ++i){
			aux2[i-7] = nomecpy[i];
		}
		unlink(aux2);
	}else if (strcmp(token, "write ") == 0 && nomecpy[6] == '/'){
		for(i = 6; i < strlen(nomecpy)-1; ++i){
			aux2[i-6] = nomecpy[i];
		}
		printf("Digite o texto");
		fgets(nome2,4096,stdin);
		write(aux2);
	}else if( strcmp(token, "help\n") == 0){
		help();
	}else if( strcmp(token, "exit\n") == 0){
		exit(0);
	}else printf("Comando nao encontrado\n");
}
