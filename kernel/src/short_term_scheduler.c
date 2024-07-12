#include "short_term_scheduler.h"
int id_counter = 1;
//Para VRR
t_temporal *timer;
int ms_transcurridos;


//Planificador de Corto Plazo General
void *planificadorCortoPlazo(void *arg){

	while(1){
		if(string_equals_ignore_case(algoritmo_planificacion, "FIFO")){
			planificador_corto_plazo_FIFO();
		}else if(string_equals_ignore_case(algoritmo_planificacion, "RR")){
			planificador_corto_plazo_RoundRobin();
		}else if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            planificador_corto_plazo_Virtual_RoundRobin();
        }else{
			//planificar_corto_plazo_round_robbin();
            printf("ERROR");
		}
	}

	return NULL;
}


//FIFO
void planificador_corto_plazo_FIFO() {      
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    sem_wait(&sem_ready); 

    pthread_mutex_lock(&m_planificador_corto_plazo);
    
    /*aca lo pongo en este orden porque antes estaba al reves, entonces lo que pasaba es que
    como la funcion planificador de corto plazo es un while(1) entonces apenas se enviaba el proceso
    desde esta funcion FIFO, volvia a ejecutarse la de planificador de corto plazo que 
    esta arriba de la de fifo y como es un while 1 directamente hacia el wait a el sem
    ready y entonces se quedaba esperando al signal de shorttercmscheduler pero si por
    ejemplo eliminaas un proceso lo que pasa es que se le hace un wait al sem ready
    que es un contador de cuantos hya en ready: en el caso que tenas dos procesos unno este ejecutando 
    y el otro en ready y entonces vos eliminabas el que estaba en ready, al volver el otro
    despertaba aal planiicador de cort plazo par que envie otro, y cuando vos eliminstas
    hiciste el wait del semaforocontador semready pero como el sem ready  ya habia sido
    pasado por lo que explique al principio solamente con el signal al depsrttar
    planifiacodr corto plazo ya pedias que manden otro a ejecutara pero no habia mas en ready
    por eso lo di vuelta, ahora primero espera desportar y despues controla que haya en ready
    para que el semaforo de ready cuando le toque decidir este actualizado y no pase lo del ejemplo
    que puse, que por mas que le haga el wait a sem ready no servia porque ya habia sido superado
    y estaban esperando solo el signal a despetar planificador de corto plazo. Esto
    sse usa en FIFO RR y VRR
    */   
    
    //ACLARACION IMPORTANTE SOBRE ESTE IF Y EL ORDEN DEL MUTEX Y WAITS ESTA EN VRR
    if (queue_size(queue_ready) == 0) {
		sem_post(&short_term_scheduler_semaphore);
        log_info(logger,"La cola ready esta vacia");
		pthread_mutex_unlock(&m_planificador_corto_plazo);
		return;
	}
    
    pthread_mutex_lock(&mutex_state_ready);
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready);

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %d\n", proceso->pid,proceso->state);
    log_info(logOficialKernel,"Cambio de Estado: PID: <%i> - Estado Anterior: READY - Estado Actual: EXEC ",proceso->pid);
    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso ->pid;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    
    pcbEJECUTANDO = proceso;
    enviar_proceso_cpu(proceso);
    
    pthread_mutex_unlock(&m_planificador_corto_plazo);

 }


void manejoHiloQuantum(void *pcb){

    t_quantum *proceso = (t_quantum *)pcb;
    
    usleep(quantumGlobal * 1000);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    if(procesoEjectuandoActualmente == proceso->pid){
        log_info(logger,"Envio Interupcion %i",procesoEjectuandoActualmente);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
        char *motivo = "Fin de Quantum";
        enviarInterrupcion(motivo,proceso->pid);
    }else{
        log_info(logger, "El Proceso %i termino antes del Quantum",proceso->pid);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

    }
    //EL UNLOCK TIENE QUE ESTAR EN LOS DOS CASOS, PORQUE SINO QUEDA BLOQUEADO
    
    free(pcb);
    pthread_cancel(pthread_self());
   
}

//Round Robin
void planificador_corto_plazo_RoundRobin() {
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    sem_wait(&sem_ready); //aclaracion esto estaba dado vuelta antes, aclaracion hecha en fifo
    
    pthread_mutex_lock(&m_planificador_corto_plazo);
    
    //ACLARACION IMPORTANTE SOBRE ESTE IF Y EL ORDEN DEL MUTEX Y WAITS ESTA EN VRR

    if (queue_size(queue_ready) == 0) {
		sem_post(&short_term_scheduler_semaphore);
        log_info(logger,"La cola ready esta vacia");
		pthread_mutex_unlock(&m_planificador_corto_plazo);
		return;
	}

    pthread_mutex_lock(&mutex_state_ready);
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready);

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %i\n", proceso->pid,proceso->state);
    log_info(logOficialKernel,"Cambio de Estado: PID: <%i> - Estado Anterior: READY - Estado Actual: EXEC ",proceso->pid);
    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso->pid;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    pcbEJECUTANDO = proceso; // aca guardo en el puntero global el PCB que se va enviar
    enviar_proceso_cpu(proceso); //envio el proceso
        
    pthread_t hiloQuantum;
    t_quantum *quatnumHilo = malloc(sizeof(t_quantum));
    quatnumHilo->pid = proceso->pid;
    //solo le paso el pid, no le paso el puntero al pcb
    //el puntero al pcb esta en pcbEJECUTANDO
    //Aca no es necesario pasarle un quantum porque hay un quantum Global

    pthread_create(&hiloQuantum,NULL,manejoHiloQuantum,quatnumHilo);
    pthread_detach(hiloQuantum);

    pthread_mutex_unlock(&m_planificador_corto_plazo);
}



void enviarInterrupcion(char *motivo, int pid){
   

    t_buffer *bufferINTERRUPCION;
    t_packet *packetINTERRUPCION;
    
    bufferINTERRUPCION = create_buffer();
    packetINTERRUPCION = create_packet(INTERRUPCION, bufferINTERRUPCION);
    add_to_packet(packetINTERRUPCION,motivo,(strlen(motivo)+1));
    add_to_packet(packetINTERRUPCION,&pid,sizeof(int));
    //si tengo una variable, le tengo que pasar la direccion a addpacket

    send_packet(packetINTERRUPCION, cpu_interrupt_socket);
    destroy_packet(packetINTERRUPCION);
}

void planificador_corto_plazo_Virtual_RoundRobin(){
    
    t_pcb *proceso = NULL;
    //log_info(logger,"Vuelvo a ejecutar");
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    log_info(logger,"Paso el semaforo DetenerPlanifiacion y esta en Semwait()");
    
    //Esto es para obtener el valor del seamforo y probar, pero se puede comentar
    int value;
    sem_getvalue(&sem_ready, &value); // Obtiene el valor actual del semáforo
    log_info(logger,"Valor actual del semáforo: %d", value);

    sem_wait(&sem_ready); //aclaracion esto estaba dado vuelta antes, aclaracion hecha en fifo
    //EL DADO VUELTA SE REFIERE A ESTE SEMAFORO Y EL DE SHORT TERM SCHEDULER
    
    pthread_mutex_lock(&m_planificador_corto_plazo);

    /*Aca el problema era este: Tenia los dos waits en el orden que los tengo ahora pero
    habia metido el mutex planificador corto plazo en el medio, para que pueda
    detener la planificacion y eliminar procesos, pero el problema estaba en que habia
    que pausar la planinficacion justo cuando estaba en cpu el proceso(porque los semaforos
    que utiliza para detenerplanificacion se bloquea aca el de corto plazo 
    mientras hace el envio). Si yo no hacia eso
    y lo dejaba como esta ahora habia un problema cuando tenia un solo proceso porque
    si el proceso volvia de cpu por fin de quantum y yo detenia la planificacion entonecs 
    el proceso quedaba en ready y aca quedaba trabado en el mutex de la mitad y no llegaba a
    ejecutarse el semwait semready y entonces como estaba frenado antes, yo podia eliminar el proceso
    y como no se ejecutaba este wait, no consumia el signal que le hago cuando despues
    de volver de fin de quantum lo guardo en ready. Ese signal lo usaba eliminar proceso
    que al encontrarlo en ready le hacia un wait para bajar el semaforo contador semready
    pero si bien esto funcionaba habia un problema porque ponele que el proceso se ejecutaba normalmente
    y yo no detenia la planificacion y el proceso llegaba a exit entonces como llego a exit, en dispatch
    le hacen un signal al semaforo shorttermscheduler y entra aca. Pero entonces como no detuve la planificacion
    supera el mutex lock y se queda trabado en semready y ahi estaba el problema, en que si queria detener
    la planificacion no podia, porque ya habia pasado ese lock aca y la consola quedaba bloqueada 
    infinitamente porque tampoco podia meter procesos entonces no podia hacer un signal a sem ready
    para que semready entre aca y llegue a hacer el unlock al mutex short term. Entonces ahi hay un problema
    despues tambien podia dejarlo como esta ahora y si podia detener la planificacion pero habia otro problema
    no funcionaba eliminar proceso de ready porque ponele que vuelve un pcb por fin de quantum
    y yo freno la planificacion para que quede trabado en el mutex pero ya habiendo superado
    el semready usando el signal que le hizo al ponerlo en ready por fin de Q 
    entonces una vez que quedo trabado en el mutex le hago eliminar el proceso
    por consola, pero en la funcion eliminar proceso habia un waitsem ready para mantener actualizado el 
    semaforo contrador de procesos en ready  y como voy a eliminar proceso le hago el wait, pero 
    ahi se quedaba colgada la consola porque ese 1 que tenia ya fue usado por este semaforo entones 
    en eliminar proceso ya estaba en 0 y se bloqueaba la consola ahi. Esto tambien pasaba si lo hacia
    en la cola prioitaria del VRR porque al volver de IO lo pone en ready y hace el signal pero lo consumia aca
    y si bien yo frenaba la planifiaccion para que quede trabado en la cola prioridad, ya habia superado
    el sem ready usando el signal que le dieron al volver de IO, entonces hacia finalizar proceso
    pero al hacer el wait ready (porque para entrar a cola prioridad uso el semaforo ready pero me fijo
    que no haya nada en cola prioridad y si hay ejecuto eso) se quedaba bloqueado el hilo consola de 
    finalizar proceso entonces era lo mismo.

    La solucion fue comentar ese wait que se hacia en finalizar proceso y que no se  haga, el problema
    era que si yo no hacia eso, al salir de finalizar proceso y reanudar la planificacion
    aca ya habia superado el sem ready y entonces al hacer el unlock del mutex directamente
    iba a hacer un pop de la cola ready o de la prioridad pero estaban vacias porque habia matado 
    al unico proceso (esto considerando que tenia solo un proceso, ahi fallaba casi siempre) entonces
    iba a hacer pop a una cola vacia y no iba a tener nada par enviar a cpu. Entonces por eso
    saque del otro TP resuelto esta parte que al principio me parecia que iba a desconfigurar
    el cuidaod que habia tenido con el semaforo contrado de procesos de ready pero esto lo que hace 
    es que despues de haber entrado y pasado ready se fija que si ready y prioridad estan vacias 
    entonces hace un return para que vuelva a la funcion loop de short term scheuer y vuelva a ingresar a VRR
    ademas le hago un post al short term schduler para que qude trabado en el sem ready y no en el
    short term schduler porque eso solo se hace desde dispatch y solo desde el largo plazo una unica vez al
    inicio de todo, entonces por eso se lo hago aca porque no tenia forma de recueprarlo ese. Pero ahi se
    queda bloqueado en semready (es como que lo vuelvo a hacer trabarse en sem ready) y ahi se queda
    esperando a que entren procesos nuevos a ready que desde el largo plazo le hagan el sempost a semready.

    De esta forma se arreglo el probelma y ahora funcion para casi todos los casos (los que probe por ahora)
    esta solucion es como que permite que el semaforo contador de procesos en ready este desactulizado y tenga 
    signals de mas pero chquea esto y es como que te limpia los que tenga de mas. Esto lo aplique
    para los otros algoritmos porque tambien servia solo que en esos me fijo solamente la cola de ready
    aca me fijo las dos porque si hay algo en prioridad tengo que entrar igual aunque ready este vacio
    Podria ser necesario un mutex antes de hacer el size de cada cola pero seria un lio entonces por las
    dudas no lo agregue y en este caso seria necesario dos mutex uno por cada cola.
    Esta parte es muy parecida a la del tp resuelto pero no hubiera pensado en modificar el semaforo
    contador de procesos en ready, porque siempre hice todo par tratar de mantenerlo actualizado
    pero esta solucion parcece que funciona.
    */
    
    //Puede ser que haya una condicion de carrera al entrar a las colas
	if (queue_size(queue_ready) == 0 && queue_size(queue_prioridad) == 0) {
		sem_post(&short_term_scheduler_semaphore);
        log_info(logger,"La cola ready esta vacia");
		pthread_mutex_unlock(&m_planificador_corto_plazo);
		return;
	}

    //Me fijo si hay algo en la cola prioridad
    log_info(logger,"Momento de Eleccion");
    pthread_mutex_lock(&mutex_state_prioridad);
        if(queue_size(queue_prioridad) != 0 ){
            log_info(logger,"Elijo Cola Prioridad");
            //Aca desbloqueo el semaoro de la cola prioridad
            pthread_mutex_unlock(&mutex_state_prioridad);
            proceso = obtenerSiguienteColaPrioridad();
            log_info(logger,"Quantum Restante: %i",proceso->quantum);
        }else{
            log_info(logger,"Elijo Cola Ready");
            //Aca desbloqueo el semaoro de la cola prioridad
            pthread_mutex_unlock(&mutex_state_prioridad);
            
            //No tengo funcion para esto, pero para la cola prioridad si
            pthread_mutex_lock(&mutex_state_ready);
            proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
            pthread_mutex_unlock(&mutex_state_ready);        
            // le cargo el QuantumGlobal porque esta viniendo de la cola de ready y no de prioridad
            proceso->quantum = quantumGlobal;
        }
    


    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %i\n", proceso->pid,proceso->state);
    log_info(logOficialKernel,"Cambio de Estado: PID: <%i> - Estado Anterior: READY - Estado Actual: EXEC ",proceso->pid);
    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso->pid;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    pcbEJECUTANDO = proceso; // aca guardo en el puntero global el PCB que se va enviar
    enviar_proceso_cpu(proceso); //envio el proceso
    //Este timer es una variable global, aca como recien envie el proceso
    //lo empiezo a correr pero la voy a frenar ...CONTINUAR
    timer = temporal_create();

    pthread_t hiloQuantum;
    t_quantum *quatnumHilo = malloc(sizeof(t_quantum));
    quatnumHilo->pid = proceso->pid;
    quatnumHilo->quantum = proceso->quantum;
    //solo le paso el pid, no le paso el puntero al pcb
    //el puntero al pcb esta en pcbEJECUTANDO
    //Aca no es necesario pasarle un quantum porque hay un quantum Global

    pthread_create(&hiloQuantum,NULL,manejoHiloQuantumVRR,quatnumHilo);
    pthread_detach(hiloQuantum);

    pthread_mutex_unlock(&m_planificador_corto_plazo);
}

void manejoHiloQuantumVRR(void *pcb){
//Este es Exclusivo para el VRR pero es casi igual a RR
//La unica diferencia es que aca en el caso que sea de cola prioritaria 
//solo va a ejecutar lo que le faltaba y si es de cola ready le cargo el quantum original
//podria haber usado el mismo pero lo puse para tratar de evitar tocar el codigo
//que funciona 
    t_quantum *proceso = (t_quantum *)pcb;
    
    usleep(proceso->quantum * 1000);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    if(procesoEjectuandoActualmente == proceso->pid){
        log_info(logger,"Envio Interupcion %i",procesoEjectuandoActualmente);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
        char *motivo = "Fin de Quantum";
        enviarInterrupcion(motivo,proceso->pid);
    }else{
        log_info(logger, "El Proceso %i termino antes del Quantum",proceso->pid);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

    }
    //EL UNLOCK TIENE QUE ESTAR EN LOS DOS CASOS, PORQUE SINO QUEDA BLOQUEADO
    
    free(pcb);
    pthread_cancel(pthread_self());
   
}

void obtenerDatosTemporal(){
    //Detengo el Temporal y guardo el tiempo transcurrido en la variable global
    temporal_stop(timer);
    ms_transcurridos = temporal_gettime(timer); //Antes se multiplicaba por mil abajo
    log_info(logger,"Tiempo Transcurrido: %i",ms_transcurridos); 
    temporal_destroy(timer);
}

void enviar_proceso_cpu(t_pcb *proceso){


    t_pcb PCBPRUEBA;
    int sizePCB = sizeof(PCBPRUEBA);

    t_buffer *bufferPCB;
    t_packet *packetPCB;
    
    bufferPCB = create_buffer();
    packetPCB = create_packet(PCB_REC, bufferPCB);
    add_to_packet(packetPCB,proceso, sizePCB);
    send_packet(packetPCB, cpu_dispatch_socket);
    destroy_packet(packetPCB);

}


t_pcb *initializePCB(int PID){
        t_pcb *pcb = malloc(sizeof(t_pcb));
        
        pcb->pid = PID;
        //id_counter++;  Aumenta el pid para que no haya 2 procesos iguales
        pcb->program_counter = 1;
        pcb->quantum = quantumGlobal;
        pcb->state = 0;
        //pcb->registers = malloc(sizeof( t_cpu_registers));
        //pcb->instruction = malloc(sizeof( t_instruction));
        // pcb->prueba=5;

        //Hago esto porque valgrind decia que estabamos mandando memoria sin inicializar
        //entonces mostraba un error. Por eso lo inicializo asi.
        //Igual dijeron que siempre le van a hacer set a un registro
        //antes de usarlo no deberia haber problemas
        pcb->registers.PC = 1;     // Program Counter
        pcb->registers.AX = 1;     // Registro AX
        pcb->registers.BX = 1;     // Registro BX
        pcb->registers.CX = 1;     // Registro CX
        pcb->registers.DX = 1;     // Registro DX
        pcb->registers.EAX = 1;    // Registro EAX
        pcb->registers.EBX = 1;    // Registro EBX
        pcb->registers.ECX = 1;    // Registro ECX
        pcb->registers.EDX = 1;    // Registro EDX
        pcb->registers.SI = 1;     // Source Index
        pcb->registers.DI = 1;   
        //Se arreglo el error    

        return pcb;
}




    
   
  
    

   
	
	
