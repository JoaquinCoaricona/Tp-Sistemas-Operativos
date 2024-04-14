#include "protocol.h"

t_buffer *create_buffer()
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = 0; //! Implementar stream
    buffer->stream = NULL;
    return buffer;
}

t_packet *create_packet(op_code code, t_buffer *buffer)
{
    // t_buffer *buffer = create_buffer(stream);
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

void destroy_packet(t_packet *packet)
{
    free(packet->buffer->stream);
    free(packet->buffer);
    free(packet);
}

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
    void *buffer;
    t_list *values = list_create();
    int buffer_size;

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

void send_packet(t_packet *packet, int client_socket)
{
    int buffer_size = packet->buffer->size + 2 * sizeof(int);
    void *to_send = serialize_packet(packet, buffer_size);

    send(client_socket, to_send, buffer_size, 0);

    free(to_send);
}