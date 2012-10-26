#ifndef XML_IO_H
#define XML_IO_H

extern void open_xmlrpc();
extern void close_xmlrpc();

extern void send_new_modem();
extern void send_report(string report);
extern string get_rx_data();
extern string get_trx_state();

extern void *xmlrpc_loop(void *d);

#endif
