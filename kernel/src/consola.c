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
			//aca antes tenia esto de abajo pero comando al hacerle el strtok a "hola joa" comando se queda con hola
			//pero linea tambien se modifica y queda con hola porque el espacio se remplaza por /0
			//y entonces parametro tambien quedaba con hola. El null lo que hace seguir desde donde se quedo
			//y usa la misma cadena y se habia quedado en joa y busca de nuevo el " " y como no lo encunetra devuelve joa
			//comando = strtok(linea," ");
			//parametro = linea

		}else{
			printf("ERROR AL INGRESAR COMANDO");
		}
		// FIN SEPARAR CASOS, AHORA YA TENGO EL COMANDO EN EL CHAR COMANDO


		//COMPARACIONES 

		if (strcmp(comando,"EJECUTAR_SCRIPT") == 0) {
			ejecutar_script(parametro);
		} else if (strcmp(comando,"INICIAR_PROCESO") == 0) {
			create_process(parametro); //Incluye parametro path
			//enviar_path_a_memoria(parametro);
		} else if (strcmp(comando, "FINALIZAR_PROCESO") == 0) {
			//hay que hacer un atoi
			finalizar_proceso(parametro);  //incluye el pid

		} else if (strcmp(comando, "DETENER_PLANIFICACION") == 0) {
			detener_planificacion();

		} else if (strcmp(comando, "INICIAR_PLANIFICACION") == 0) {
			iniciar_planificacion();

		} else if (strcmp(comando, "MULTIPROGRAMACION") == 0) {
			//hay que hacer un atoi
			multiprogramacion(parametro); //Incluye parametro VALOR multiprogramacion
			
		} else if(strcmp(comando, "PROCESO_ESTADO") == 0){
			listarProcesos();
		} else {
			log_error(logger,"Comando desconocido campeon, leete la documentacion de nuevo :p");
		}

		free(linea);
	}
}

