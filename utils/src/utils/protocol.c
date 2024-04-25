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

t_pcb *fetch_PCB(int client_socket)
{
    int total_size;
    int offset = 0;
   
    void *buffer2;
   
  
    t_pcb* PCBrec;
	t_instruction* instruction;
    t_cpu_registers* cpureg;
    
    int pid;
    int counter;
    int tama;
    int quantum;
    int length;
    char* estado; // pueden ser "NEW", "READY", "EXEC", "BLOCKED" y "EXIT"
	int64_t tiempollega;
    int tamaPuntero = sizeof(cpureg);
    int tamaInstruccion = sizeof(instruction);
    


    buffer2 = fetch_buffer(&total_size, client_socket);

    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO
    offset += sizeof(int);
    

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    memcpy(&counter, buffer2 + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);
    
    memcpy(&quantum, buffer2 + offset, sizeof(int)); //RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(cpureg, buffer2 + offset, tamaPuntero); //! RECIBO EL PUNTERO A CPU_REGISTERS a los que son punter no pongo el &
    offset += tamaPuntero;

    memcpy(&length,buffer2 + offset, sizeof(int)); //RECIBO EL LENGTH
    offset += sizeof(int);
    
    estado = malloc(length);
    memcpy(estado,buffer2 + offset, length-1); //RECIBO EL PROCESS_STATE aca no paso la direccion porque ya es un PUNTERO
    offset += length;
    
    memcpy(&tiempollega, buffer2 + offset, sizeof(int64_t)); //RECIBO EL TIEMPO LLEGADA
    offset += sizeof(int64_t);

    memcpy(instruction, buffer2 + offset,tamaInstruccion); //RECIBO la instruccion

    free(buffer2);


    printf("ESTADO: %s       ",estado);
    printf("PID: %i        ",pid);
    printf("QUANTUM: %i      ",quantum);

   return PCBrec;
}
