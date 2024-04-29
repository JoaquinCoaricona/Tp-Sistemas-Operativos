#include "protocol.h"
#include "utils.h"

// MARK: CLIENT
t_buffer *create_buffer()
{
    t_buffer *buffer = malloc(sizeof(t_buffer));

    buffer->size = 0; //! Implementar stream
    buffer->stream = NULL;

    return buffer;
}

t_packet *create_packet(op_code code, t_buffer *buffer)
{
    t_packet *packet = malloc(sizeof(t_packet));

    packet->code = code;
    packet->buffer = buffer;

    return packet;
}

void *serialize_packet(t_packet *packet, int buffer_size)
{
    void *serialized_packet = malloc(buffer_size);
    int offset = 0;

    memcpy(serialized_packet + offset, &(packet->code), sizeof(int));
    offset += sizeof(int);

    memcpy(serialized_packet + offset, &(packet->buffer->size), sizeof(int));
    offset += sizeof(int);

    memcpy(serialized_packet + offset, packet->buffer->stream, packet->buffer->size);
    offset += packet->buffer->size;

    return serialized_packet;
}

void add_to_packet(t_packet *packet, void *stream, int size)
{
    packet->buffer->stream = realloc(packet->buffer->stream, packet->buffer->size + size + sizeof(int));

    memcpy(packet->buffer->stream + packet->buffer->size, &size, sizeof(int));
    memcpy(packet->buffer->stream + packet->buffer->size + sizeof(int), stream, size);

    packet->buffer->size += size + sizeof(int);
}

void send_packet(t_packet *packet, int client_socket)
{
    int buffer_size = packet->buffer->size + 2 * sizeof(int);
    void *to_send = serialize_packet(packet, buffer_size);

    send(client_socket, to_send, buffer_size, 0);

    free(to_send);
}

void destroy_packet(t_packet *packet)
{
    free(packet->buffer->stream);
    free(packet->buffer);
    free(packet);
}

// MARK: SERVER
void *fetch_buffer(int *size, int client_socket)
{
    void *buffer;

    recv(client_socket, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);

    recv(client_socket, buffer, *size, MSG_WAITALL);

    return buffer;
}

t_list *fetch_packet(int client_socket)
{
    int total_size;
    int offset = 0;
    int buffer_size;
    void *buffer;

    t_list *values = list_create();

    buffer = fetch_buffer(&total_size, client_socket);

    while (offset < total_size)
    {
        memcpy(&buffer_size, buffer + offset, sizeof(int));
        offset += sizeof(int);
        char *value = malloc(buffer_size);

        memcpy(value, buffer + offset, buffer_size);
        offset += buffer_size;

        list_add(values, value);
    }

    free(buffer);

    return values;
}

int fetch_codop(int client_socket)
{
    int cod_op;

    if (recv(client_socket, &cod_op, sizeof(int), MSG_WAITALL) > 0)
    {
        return cod_op;
    }

    close(client_socket);

    return -1;
}

void *fetch_PCB(int client_socket,t_pcb *PCBrec)
{
    int total_size;
    int offset = 0;
   
    void *buffer2;
   
    int tama; //Solo para recibir el size que esta al principio del buffer
    
    buffer2 = fetch_buffer(&total_size, client_socket);

    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO
    offset += sizeof(int);
    

    memcpy(&(PCBrec->pid),buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(PCBrec->program_counter), buffer2 + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);
    
    memcpy(&(PCBrec->quantum), buffer2 + offset, sizeof(int)); //RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(&(PCBrec->state), buffer2 + offset, sizeof(t_process_state)); //RECIBO EL PROCESS STATE
    offset += sizeof(t_process_state);

    memcpy(&(PCBrec->registers.PC), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.AX), buffer2 + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.BX), buffer2 + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.CX), buffer2 + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.DX), buffer2 + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.EAX), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.EBX), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.ECX), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.EDX), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.SI), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.DI), buffer2 + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);


    // memcpy(&tiempollega, buffer2 + offset, sizeof(int64_t)); //RECIBO EL TIEMPO LLEGADA
    // offset += sizeof(int64_t);
    
    // cpureg = malloc(tamaPuntero); // Allocate memory for CPU_REGISTERS
    // memcpy(cpureg, buffer2 + offset, tamaPuntero);
    // offset += tamaPuntero;

    // memcpy(state, buffer2 + offset, size_ps); 
    // offset += size_ps;
    
    // instruction = malloc(tamaInstruccion); // Allocate memory for instruction
    // memcpy(instruction, buffer2 + offset, tamaInstruccion); // Copy the instruction
    // offset += tamaInstruccion;
    
    // memcpy(&prueba, buffer2 + offset, sizeof(int)); //RECIBO EL PRUEBA
    // offset += sizeof(int);

    free(buffer2);
}

void *fetch_pathYpid(int client_socket,t_instrucciones *instruccionREC)
{
    int total_size;
    int offset = 0;
   
    void *buffer2;
   
    int tama; //Solo para recibir el size que esta al principio del buffer
    int lpath;


    buffer2 = fetch_buffer(&total_size, client_socket);

    memcpy(&(instruccionREC->pid),buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL PID(UN INT)
    offset += sizeof(int);
    
    memcpy(&(instruccionREC->pid),buffer2 + offset, sizeof(int)); //RECIBO EL PID REAL
    offset += sizeof(int);

    memcpy(&(lpath), buffer2 + offset, sizeof(int)); // RECIBO EL LENGTH DEL PATH
    offset += sizeof(int);
    instruccionREC->path = malloc(lpath);

    memcpy(instruccionREC->path,buffer2 + offset, lpath);
    
    printf("\n");
    printf("%s\n",instruccionREC->path);
  
    free(buffer2);

}




