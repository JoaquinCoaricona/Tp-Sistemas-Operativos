#include "protocol.h"

t_buffer *create_buffer(void *stream)
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = 0; //! Implementar stream
    buffer->stream = stream;
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

void serialize_packet(t_packet *packet, int buffer_size)
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
