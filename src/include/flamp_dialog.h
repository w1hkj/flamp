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

extern Fl_Double_Window *main_window;

extern Fl_Output* txt_rx_filename;
extern Fl_Output* txt_rx_datetime;
extern Fl_Output* txt_rx_descrip;
extern Fl_Output* txt_rx_callinfo;
extern Fl_Output* txt_rx_filesize;
extern Fl_Output* txt_rx_numblocks;
extern Fl_Output* txt_rx_blocksize;
extern Fl_Output* txt_rx_missing_blocks;

extern Fl_BlockMap* rx_progress;

extern Fl_Hold_Browser* rx_queue;

extern FTextView* txt_rx_output;

extern Fl_Simple_Counter * cnt_blocksize;
extern Fl_Simple_Counter * cnt_repeat_nbr;
extern Fl_Simple_Counter * cnt_repeat_header;

extern Fl_Input2* txt_tx_mycall;
extern Fl_Input2* txt_tx_myinfo;
extern Fl_Input2* txt_tx_send_to;

extern Fl_ComboBox* encoders;
extern Fl_Output* txt_tx_filename;
extern Fl_Input2* txt_tx_descrip;
extern Fl_Input2* txt_tx_selected_blocks;
extern Fl_Output* txt_tx_numblocks;
extern Fl_Input*  drop_file;

extern Fl_Check_Button* btn_repeat_at_times;
extern Fl_ComboBox*     cbo_repeat_every;
extern Fl_Input2*       txt_repeat_times;
extern Fl_Check_Button* btn_repeat_forever;
extern Fl_Light_Button* do_events;
extern Fl_Output* outTimeValue;

extern void cb_do_events(Fl_Light_Button *b, void*);

extern Fl_Check_Button* btn_use_compression;
extern Fl_ComboBox* encoders;
extern Fl_ComboBox* cbo_modes;
extern Fl_Output*   txt_transfer_size_time;
extern Fl_Output*   txt_transfer_time;

extern Fl_Hold_Browser* tx_queue;

extern Fl_Double_Window* flamp_dialog();

extern bool rx_remove;

extern bool valid_mode_check(std::string &md);

struct st_modes {std::string s_mode; float f_cps;};
extern struct st_modes s_basic_modes[];
extern struct st_modes s_modes[];

extern void update_cbo_modes(std::string &fldigi_modes);

#endif
