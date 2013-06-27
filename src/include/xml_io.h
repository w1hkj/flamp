#ifndef XML_IO_H
#define XML_IO_H

extern void open_xmlrpc();
extern void close_xmlrpc();

extern void send_new_modem(std::string modem);
extern void send_report(string report);

extern string get_rx_data();
extern string get_trx_state();

extern void send_clear_rx(void);
extern void send_clear_tx(void);
extern void send_tx(void);
extern void send_rx(void);
extern void send_rsid(void);
extern void send_abort(void);
extern void send_tune(void);
extern void set_rsid(void);
extern string get_rsid_state(void);
extern void *xmlrpc_loop(void *d);

#endif
