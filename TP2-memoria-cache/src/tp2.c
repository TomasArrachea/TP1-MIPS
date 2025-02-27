#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include "cache.h"

const char* VERSION_ACTUAL = "1.0";

void imprimir_ayuda();
void imprimir_version();
void leer_archivo(char* nombre);
void ejecutar_instrucciones(char* archivo_instrucciones, char* archivo_salida);
char parse_arguments(int argc, char** argv, int* ways, int* cache_size, int* block_size, char* archivo_salida, char* archivo_instrucciones);


int main(int argc, char* argv[]) {
	int ways = 4, cache_size = 4096, block_size = 256;
	char* archivo_salida = "cache_log.mem", *archivo_instrucciones;

	if (parse_arguments(argc, argv, &ways, &cache_size, &block_size, archivo_salida, archivo_instrucciones) != 0) {
		return 1;
	}

	init_memoria(block_size);
	init_cache(ways, cache_size);
    ejecutar_instrucciones(archivo_instrucciones, archivo_salida);
    delete_memoria();
    delete_cache();
	return 0;
}


char parse_arguments(int argc, char** argv, int* ways, int* cache_size, int* block_size, char* archivo_salida, char* archivo_instrucciones) {
	int c;
    static struct option long_options[] = {
        {"help",    no_argument,      0,  'h' },
        {"version",	no_argument,       0,  'V' },
        {"output",  required_argument, 0,  'o' },
        {"ways",	required_argument, 0,  'w' },
        {"cachesize", required_argument, 0,  'c' },
        {"blocksize", required_argument, 0,  'b' },
        {"cs", required_argument, 0,  'c' },
        {"bs", required_argument, 0,  'b' },
        {0,           0,                 0,  0   }
    };

	while ((c = getopt_long_only(argc, argv, "hVo:w:c:b:?", long_options, NULL)) != -1)
		switch (c) {
			case 'h':
				imprimir_ayuda();
			    return 1;
			case 'V':
				imprimir_version();
			    return 1;
			case 'o':
		    	strcpy(archivo_salida, optarg);
			    break;
			case 'w':
			    *ways = atoi(optarg);
			    break;
			case 'c':
			    *cache_size = atoi(optarg);
			    break;
			case 'b':
			    *block_size = atoi(optarg);
			    break;
			case '?':
			    return 1;
			default:
				fprintf (stderr, "opcion default.");
				return 1;
    }
    int index = optind;
    if (argc - index != 1) {	
		fprintf (stderr, "Error: se debe ingresar el nombre de un archivo con instrucciones. Ingrese -h para ver la ayuda.");
		return 1;
    }
	strcpy(archivo_instrucciones, argv[index++]);
	return 0;
}

void ejecutar_instrucciones(char* archivo_instrucciones, char* archivo_salida) {
	FILE* instrucciones = fopen(archivo_instrucciones, "r");
	if (!instrucciones){
		fprintf(stderr, "Error al abrir el archivo de instrucciones.");
		return;
	}
	FILE* salida = fopen(archivo_salida, "w");
	if (!salida){
		fprintf(stderr, "Error al abrir el archivo de resultados.");
		return;
	}

	char comando;
	while(!feof(instrucciones)) {
		fscanf(instrucciones, "%c", &comando);
		switch (comando) {
			case 'W' : {
				int address;
				char value;
				fscanf(instrucciones, "%i, %hhu\n", &address, &value);
				char hit = write_byte(address, value);
				if (hit == -1)
					fprintf(salida, "SEGFAULT trying to Write address %i.\n", address);
				else 
					fprintf(salida, "Write value %hhu in address %i: hit = %hhu\n", value, address, hit);
				break;
			}
			case 'R': {
				int address;
				fscanf(instrucciones, "%i\n", &address);
				char hit;
				char value = read_byte(address, &hit);
				if (hit == -1)
					fprintf(salida, "SEGFAULT trying to Read address %i.\n", address);
				else
					fprintf(salida, "Read value %hhu from address %i: hit = %hhu\n", value, address, hit);
				break;
			}
			case 'M':
				fscanf(instrucciones, "R\n");
				fprintf(salida, "Miss rate: %hhu%%.\n", get_miss_rate());
				break;
			case 'i':
				fscanf(instrucciones, "nit\n");
				init();
				fprintf(salida, "init\n");
				break;
			default:
				return;
		}
	}

	fclose(instrucciones);
	fclose(salida);
}

void imprimir_ayuda(){
	printf(
	"	Uso:\n"
	"		tp2 -h\n"
	"		tp2 -V\n"
	"		tp2 options archivo\n"
	"\n	Opciones:\n"
	"		-h	Imprime este mensaje.\n"
	"		-V 	Da la versión del programa.\n"
	"		-s 	Imprime por consola la salida del programa, en lugar de guardarlo en el archivo de salida.\n"
	"		-o 	Prefijo de los archivos de salida.\n"
	"		-w	Cantidad de vı́as.\n"
	"		-cs Tamaño del caché en kilobytes.\n"
	"		-bs Tamaño de bloque en bytes.\n"
	"\n	Ejemplos:\n"
	"	-> tp2 -w 4 -cs 8 -bs 16 prueba1.mem\n");
}

void imprimir_version(){
	printf("Versión actual: %s\n", VERSION_ACTUAL);
}