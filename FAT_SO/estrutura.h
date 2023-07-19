#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*Valores das entradas na FAT de 16 bits:
0x0000 -> cluster livre
0x0001 - 0xfffc -> arquivo (ponteiro p/ proximo cluster)
0xfffd -> boot block
0xfffe -> FAT
0xffff -> fim do arquivo

Informacoes da estrutura das entradas de diretorio:
18 bytes -> nome do arquivo
1 byte -> atributo do arquivo
7 bytes -> reservado
2 bytes -> numero do primeiro cluster ocupado
4 bytes -> tamanho do arquivo
*/

#define SECTOR_SIZE	                512
#define CLUSTER_SIZE	            2*SECTOR_SIZE
#define ENTRY_BY_CLUSTER            CLUSTER_SIZE /sizeof(dir_entry_t)
#define NUM_CLUSTER	                4096
#define STRINGS_SIZE 200

typedef struct{

	uint8_t filename[18];
	uint8_t attributes;
	uint8_t reserved[7];
	uint16_t first_block;
	uint32_t size;

}dir_entry_t;

typedef union{

	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
	uint8_t data[CLUSTER_SIZE];

}data_cluster;


unsigned short  fat[NUM_CLUSTER];
unsigned char   boot_block[CLUSTER_SIZE];
dir_entry_t     root_dir[32];
data_cluster    clusters[4086];

void init();
int load();
void atualizar_fat();
void save_cluster(int index, data_cluster cluster);
data_cluster ler_cluster(int index);
int procurar_dir(char *diretorio, char *dir_atual, int procura);
int num_dir(char *caminho);
void separa(char *str1, char *str2, char *str3, char *separador);
data_cluster *qtd_cluster(char *string, int *numClusters);
void mkdir(char *diretorio);
void ls(char* diretorio);
void create(char *diretorio);
void write(char* parametros);
void unlink(char *diretorio);
void read(char* diretorio);
void append(char* parametros);
void help();
void command();