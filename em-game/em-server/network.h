void init_network();
void init_network_sig();
void kill_network();
void network_alarm();

struct string_t *get_text_addr(void *conn);


void net_emit_uint32(uint32_t temp_conn, uint32_t val);
void net_emit_int(uint32_t temp_conn, int val);
void net_emit_float(uint32_t temp_conn, float val);
void net_emit_uint16(uint32_t temp_conn, uint16_t val);
void net_emit_uint8(uint32_t temp_conn, uint8_t val);
void net_emit_char(uint32_t temp_conn, char val);
void net_emit_string(uint32_t temp_conn, char *string);
void net_emit_buf(uint32_t temp_conn, void *buf, int size);
void net_emit_end_of_stream(uint32_t temp_conn);


void disconnect(uint32_t conn);
void udp_data();

extern int udp_socket;
