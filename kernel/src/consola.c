#include "consola.h"
#include "main.h"


void levantar_consola(t_log *logger) {


	printf("CONSOLA ACTIVA: \n");
	while (1) {
		char *linea = readline(">");
		char *comando = NULL;
		char *parametro = NULL;

		// //es para que guarde un historial de los comandos y poder usar las flechas
		// if(linea){
		// 	add_history(linea);
		// }

		//SEPARAR CASOS EN LOS QUE LLEGA CON PARAMETRO Y EN LOS QUE NO
		if(strchr(linea, ' ') == NULL){
			comando = linea;
		}else if(strchr(linea, ' ') != NULL){
			comando = strtok(linea," ");
			parametro = strtok(NULL, " ");

		}else{
			printf("ERROR AL INGRESAR COMANDO");
		}
		// FIN SEPARAR CASOS, AHORA YA TENGO EL COMANDO EN EL CHAR COMANDO


		//COMPARACIONES 

		if (strcmp(comando,"EJECUTAR_SCRIPT") == 0) {
			//ejecutar_script(parametro); //Incluye parametro path
			printf("%s\n",comando);
			printf("%s\n",parametro);
		} else if (strcmp(comando,"INICIAR_PROCESO") == 0) {
			//iniciar_proceso(parametro); //Incluye parametro path

		} else if (strcmp(comando, "FINALIZAR_PROCESO") == 0) {
			//hay que hacer un atoi
			//finalizar_proceso(parametro);  //incluye el pid

		} else if (strcmp(comando, "DETENER_PLANIFICACION") == 0) {
			//detener_planificacion();

		} else if (strcmp(comando, "INICIAR_PLANIFICACION") == 0) {
			//iniciar_planificacion();

		} else if (strcmp(comando, "MULTIPROGRAMACION") == 0) {
			//hay que hacer un atoi
			//multiprogramacion(parametro) //Incluye parametro VALOR multiprogramacion
			
		} else if(strcmp(comando, "PROCESO_ESTADO") == 0){
			//proceso_estado();
		} else {
			log_error(logger,"Comando desconocido campeon, leete la documentacion de nuevo :p");
			printf("ERROR COMANDO DESCONOCIDO");
		}

		free(linea);
	}
}
