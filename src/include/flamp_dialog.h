#ifndef FLAMP_DIALOG_H
#define FLAMP_DIALOG_H

#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Progress.H>

#include "FTextView.h"
#include "flinput2.h"
#include "combo.h"
#include "calendar.h"
#include "Fl_BlockMap.h"

#define CNT_BLOCK_SIZE_STEP_RATE	16
#define CNT_BLOCK_SIZE_MINIMUM		16
#define CNT_BLOCK_SIZE_MAXIMUM		2048

#define XMT_LABEL "Xmit"
#define CANX_LABEL "Cancel"
#define RELAY_LABEL "Relay"

extern Fl_Tabs  * tabs;
extern Fl_Group * Config_tab;

extern Fl_Double_Window *main_window;

extern Fl_Output * txt_rx_blocksize;
extern Fl_Output * txt_rx_callinfo;
extern Fl_Output * txt_rx_datetime;
extern Fl_Output * txt_rx_descrip;
extern Fl_Output * txt_rx_filename;
extern Fl_Output * txt_rx_filesize;
extern Fl_Output * txt_rx_missing_blocks;
extern Fl_Output * txt_rx_numblocks;

extern Fl_BlockMap * rx_progress;

extern Fl_Hold_Browser * rx_queue;

extern FTextView * txt_rx_output;

extern Fl_Check_Button * btn_disable_header_modem_on_block_fills;
extern Fl_Check_Button * btn_enable_header_modem;
extern Fl_Check_Button * btn_enable_tx_unproto;
extern Fl_Check_Button * btn_enable_txrx_interval;
extern Fl_Check_Button * btn_enable_unproto_markers;

extern Fl_Check_Button * btn_fldigi_xmt_mode_change;
extern Fl_Check_Button * btn_load_from_tx_folder;
extern Fl_Check_Button * btn_sync_mode_flamp_fldigi;
extern Fl_Check_Button * btn_sync_mode_fldigi_flamp;

extern Fl_ComboBox       * cbo_header_modes;
extern Fl_Simple_Counter * cnt_rx_internval_secs;
extern Fl_Simple_Counter * cnt_tx_internval_mins;

extern Fl_Simple_Counter * cnt_blocksize;
extern Fl_Simple_Counter * cnt_repeat_header;
extern Fl_Simple_Counter * cnt_repeat_nbr;

extern Fl_Input2 * txt_tx_mycall;
extern Fl_Input2 * txt_tx_myinfo;
extern Fl_Input2 * txt_tx_send_to;

extern Fl_ComboBox * encoders;
extern Fl_Input    * drop_file;
extern Fl_Input2   * txt_tx_descrip;
extern Fl_Input2   * txt_tx_selected_blocks;
extern Fl_Output   * txt_tx_filename;
extern Fl_Output   * txt_tx_numblocks;

extern Fl_Button       * btn_send_file;
extern Fl_Button       * btn_send_queue;
extern Fl_Check_Button * btn_repeat_at_times;
extern Fl_Check_Button * btn_repeat_forever;
extern Fl_ComboBox     * cbo_repeat_every;
extern Fl_Input2       * txt_repeat_times;
extern Fl_Light_Button * do_events;
extern Fl_Output       * outTimeValue;

extern Fl_Check_Button * btn_auto_load_queue;
extern Fl_Input2       * txt_auto_load_queue_path;

extern void cb_do_events(Fl_Light_Button *b, void*);

extern Fl_Check_Button * btn_use_compression;
extern Fl_ComboBox     * cbo_modes;
extern Fl_ComboBox     * encoders;
extern Fl_Output       * txt_transfer_size_time;
extern Fl_Output       * txt_transfer_time;

extern Fl_Check_Button * btn_clear_tosend_on_tx_blocks;
extern Fl_Check_Button * btn_enable_delete_warning;
extern Fl_Check_Button * btn_enable_tx_on_report;

extern Fl_Hold_Browser * tx_queue;

extern Fl_Double_Window * flamp_dialog();

extern bool valid_mode_check(std::string &md);

extern const char * s_basic_modes[];
extern char * s_modes[];

extern void update_cbo_modes(std::string &fldigi_modes);

extern Fl_Check_Button * btn_enable_header_modem;
extern void cb_enable_header_modem(Fl_Check_Button *a, void *b);
extern void unproto_widgets(class cAmp *amp);

// Hamcasting panel

extern Fl_Check_Button * btn_hamcast_mode_cycle;

extern Fl_Check_Button * btn_hamcast_mode_enable_1;
extern Fl_ComboBox     * cbo_hamcast_mode_selection_1;
extern Fl_Output       * txt_hamcast_select_1_time;

extern Fl_Check_Button * btn_hamcast_mode_enable_2;
extern Fl_ComboBox     * cbo_hamcast_mode_selection_2;
extern Fl_Output       * txt_hamcast_select_2_time;

extern Fl_Check_Button * btn_hamcast_mode_enable_3;
extern Fl_ComboBox     * cbo_hamcast_mode_selection_3;
extern Fl_Output       * txt_hamcast_select_3_time;

extern Fl_Check_Button * btn_hamcast_mode_enable_4;
extern Fl_ComboBox     * cbo_hamcast_mode_selection_4;
extern Fl_Output       * txt_hamcast_select_4_time;

extern Fl_Output * txt_hamcast_select_total_time;

extern bool assign_bc_modem_list(void);
extern void estimate_bc(void);

extern std::string g_header_modem;
extern std::string g_modem;
extern std::string selected_encoder_string;

// end Hamcasting panel

// Receive pane
extern Fl_Button * btn_parse_relay_blocks;
extern Fl_Button * btn_send_relay;
extern Fl_Input2 * txt_relay_selected_blocks;
// end Receive pane

#endif
