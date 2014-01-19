#ifndef XML_IO_H
#define XML_IO_H

extern void open_xmlrpc();
extern void close_xmlrpc();

extern void send_new_modem(std::string modem);
extern void send_report(string report);

extern std::string get_rx_data();
extern std::string get_trx_state();
extern std::string get_tx_duration();
extern std::string get_char_rates(void);
extern std::string get_rsid_state(void);
extern std::string get_tx_timing(std::string data);
extern std::string get_tx_char_n_timing(int character, int count);
extern std::string get_char_timing(int character);
extern void send_clear_rx(void);
extern void send_clear_tx(void);
extern void send_tx(void);
extern void send_rx(void);
extern void send_rsid(void);
extern void send_abort(void);
extern void send_tune(void);
extern void set_rsid(void);

extern void *xmlrpc_loop(void *d);

extern void set_xmlrpc_timeout(double value);
extern void set_xmlrpc_timeout_default(void);

#endif
