//======================================================================
//	script_parsing.h  (FLAMP)
//
//  Author(s):
//
//	Robert Stiles, KK5VD, Copyright (C) 2014
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// =====================================================================

#ifndef __script_parsing__
#define __script_parsing__

#include <string>
#include <stdio.h>

#define MAX_CMD_PARAMETERS 5
#define MAX_COMMAND_LENGTH 128
#define MAX_PARAMETER_LENGTH FILENAME_MAX
#define MAX_READLINE_LENGTH (FILENAME_MAX+FILENAME_MAX)
#define MAX_SUB_SCRIPTS 5

#define SCRIPT_FILE_TAG ((char *)"FLAMP_CONFIG")
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define RESET_ALL 0x01
#define RESET_PARTIAL 0X02

#define CMD_AUTO_LOAD_QUEUE "AUTO LOAD QUEUE"
#define CMD_BASE            "BASE"
#define CMD_BLOCKS          "BLOCKS"
#define CMD_CALLFROM        "CALLFROM"
#define CMD_CALLTO          "CALLTO"
#define CMD_CLEAR_MISSING   "CLEAR MISSING"
#define CMD_CLEAR_RXQ       "CLEAR RXQ"
#define CMD_CLEAR_TXQ       "CLEAR TXQ"
#define CMD_COMP            "COMP"
#define CMD_EVENT           "EVENT"
#define CMD_EVENT_FOREVER   "EVENT FOREVER"
#define CMD_EVENT_TIMES     "EVENT TIMES"
#define CMD_EVENT_TIMED     "EVENT TIMED"
#define CMD_EVENT_TYPE      "EVENT TYPE"
#define CMD_FILE            "FILE"
#define CMD_HAMCAST         "HAMCAST"
#define CMD_HAMCAST_MODEM   "HAMCAST MODEM"
#define CMD_HDR_REPEAT      "HDR REPEAT"
#define CMD_HEADER          "HEADER"
#define CMD_HEADER_MODEM    "HEADER MODEM"
#define CMD_INFO            "INFO"
#define CMD_INHIBIT_HEADER  "INHIBIT HEADER"
#define CMD_INTERVAL        "INTERVAL"
#define CMD_LOAD_QUEUE      "LOAD QUEUE"
#define CMD_LOAD_TXDIR      "LOAD TXDIR"
#define CMD_MODEM           "MODEM"
#define CMD_PATH            "PATH"
#define CMD_PROTO           "PROTO"
#define CMD_QUEUE_FILEPATH  "QUEUE FILEPATH"
#define CMD_RESET           "RESET"
#define CMD_RX_INTERVAL     "RX INTERVAL"
#define CMD_SYNC_WITH       "SYNC WITH"
#define CMD_TX_INTERVAL     "TX INTERVAL"
#define CMD_TX_REPORT       "TX REPORT"
#define CMD_UNPROTO_MARKERS "UNPROTO MARKERS"
#define CMD_WARN_USER       "WARN USER"
#define CMD_XMIT_REPEAT     "XMIT REPEAT"

//! @enum _event_types
//! Event types.

//! @typedef EVENT_TYPES
//! @see _event_types

typedef enum _event_types {
	et_5_min = 0,
	et_15_min,
	et_30_min,
	et_hourly,
	et_even_hours,
	et_odd_hours,
	et_repeat_at,
	et_one_time_at,
	et_continious_at,
} EVENT_TYPES;

//! @enum script_codes
//! Error codes.

//! @typedef SCRIPT_CODES
//! @see script_codes

typedef enum script_codes {
	script_command_not_found = -100,     //!< Script command not found.
	script_file_not_found,               //!< Script file not found.
	script_path_not_found,               //!< Script directory path not found.
	script_non_script_file,              //!< Opened file not a Script file.
	script_max_sub_script_reached,       //!< Maximum open subscripts reached.
	script_subscript_exec_fail,          //!< Subscript execution fail (internal).
	script_incorrectly_assigned_value,   //!< Incorrect parameter type.
	script_invalid_parameter,            //!< Parameter is not valid.
	script_parameter_error,              //!< Script parameter invalid.
	script_function_parameter_error,     //!< Function parameter error (internal, non script error).
	script_mismatched_quotes,            //!< Opening or closing quotes missing prior to reaching end if line.
	script_command_seperator_missing,    //!< Command missing ':'
	script_args_eol,                     //!< Reached end of args list sooner then expected
	script_param_check_eol,              //!< Reached end of parameter check list sooner then expected
	script_paramter_exceeds_length,      //!< Data length exceeds expectations
	script_memory_allocation_error,      //!< Memory Allocation Error (Non-Script internal Error).
	script_general_error = -1,           //!< General purpose error (undefined erros).
	script_no_errors,                    //!< No Errors
	script_char_match_not_found,         //!< Search char not found (Warning)
	script_end_of_line_reached           //!< End of line reached (Warning)
} SCRIPT_CODES;

class ScriptParsing;
typedef  SCRIPT_CODES (ScriptParsing::*calling_func)(struct script_cmds *);

//! @enum paramter_types
//! Parameter type flags. Used to validate the informarion.

//! @typedef PARAM_TYPES
//! @see paramter_types

typedef enum paramter_types {
	p_null = 0,
	p_char,
	p_int,
	p_unsigned_int,
	p_long,
	p_unsigned_long,
	p_float,
	p_double,
	p_string,
	p_path,
	p_filename,
} PARAM_TYPES;

#define QUEUE_COMMAND  0x0001
#define SCRIPT_COMMAND 0x0002

//! @struct script_cmds
//! Vector table of script command string and executing function member.

//! @typedef SCRIPT_COMMANDS
//! @see script_cmds
typedef struct script_cmds {
	char command[MAX_COMMAND_LENGTH];                      //!< Command matching string.
	int  flags;                                            //!< General purpose flags
	size_t command_length;                                 //!< Number of chacters in the command string.
	int  argc;                                             //!< Number of required prarmeters for the command
	char *args[MAX_CMD_PARAMETERS+1];                      //!< String vector table of parameters
	enum paramter_types param_check[MAX_CMD_PARAMETERS+1]; //!< Flags to determine type of parameter.
	const char **valid_values;                                   //!< A list of valid paramters.
	int valid_value_count;                                 //!< The number of valid paramters.
	calling_func func;                                     //!< The function (member) to execute on positive match.
	int (*cb)(ScriptParsing *sp, struct script_cmds *sd);  //!< Call back function for script command

} SCRIPT_COMMANDS;

//! @class script_parsing_class
class ScriptParsing {

public:

private:
	pthread_mutex_t ac;            //<! Thread safe data access

	bool _auto_load_queue;          //!< Auto load the queue on event flag.
	bool _clear_missing;            //!< Clear missing block after transmit fills flag
	bool _clear_rxq;                //!< Clear receive queue flag.
	bool _clear_txq;                //!< Clear transmit queue flag.
	bool _comp;                     //!< Use compression on transmitted file flag.
	bool _event;                    //!< Enable/Disable event flag.
	bool _event_forever;            //!< Enable/Disable forever events.
	bool _event_timed;              //!< Enable/Event based on time.
	bool _hamcast;                  //!< Enable/Disable Hamcast flag.
	bool _hamcast_modem_1_enable;   //!< Enable/Disable flag for Hamcast modem position 1
	bool _hamcast_modem_2_enable;   //!< Enable/Disable flag for Hamcast modem position 2
	bool _hamcast_modem_3_enable;   //!< Enable/Disable flag for Hamcast modem position 3
	bool _hamcast_modem_4_enable;   //!< Enable/Disable flag for Hamcast modem position 4
	bool _header_modem_enable;      //!< Enable/Disable flag for Header Modem
	bool _inhibit_header;           //!< Inhibit header modem on fills.
	bool _interval;                 //!< Enable/Disable interval timer.
	bool _load_txdir;               //!< Enable/Disable loading of the queue from the tx directory.
	bool _proto;                    //!< Enable/Disable AMP protocol
	bool _sync_with_flamp;          //!< Enable/Disable Sync fldigi with flamp
	bool _sync_with_fldigi;         //!< Enable/Disable Sync flamp with fldigi
	bool _sync_with_prior;          //!< Enable/Disable Modem set prior to transmit.
	bool _tx_report;                //!< Enable/Disable tramsmit reports from FLAMP.
	bool _unproto_markers;          //!< Enable/Disable unproto makers during transmition of unproto data.
	bool _warn_user;                //!< Enable/Disable Warning message when removing files from the transmit queue.

	int _base;                      //!< Storage for the base encoding type
	int _blocks;                    //!< Storage for the transmit block count.
	int _event_type;                //!< Storage for type of event
	int _file_type;                 //!< File type true=script false=queue
	int _hdr_repeat;                //!< Number of time to repeat headr transmistion.
	int _reset;                     //!< Reset type flag.
	int _rx_interval;               //!< Rx interval duration (in seconds)
	int _sub_script_count;          //!< Number of sub class created and called from upper level class
	int _tx_interval;               //!< Interval tramit time (in minutes)
	int _xmit_repeat;               //!< Number of times non-header data repeats.

	std::string _call_from;         //!< Storage for the Call From (callsign)
	std::string _call_to;           //!< Storage for the Call To (callsign)
	std::string _envent_times;      //!< Storage fir the assigne vent times.
	std::string _desc;              //!< Storage for description of queued file.
	std::string _file;              //!< Secondary script file to process.
	std::string _header_modem;      //!< Header modem
	std::string _hamcast_modem_1;   //!< Hamcast modem name for position 1 in the rotation
	std::string _hamcast_modem_2;   //!< Hamcast modem name for position 2 in the rotation
	std::string _hamcast_modem_3;   //!< Hamcast modem name for position 3 in the rotation
	std::string _hamcast_modem_4;   //!< Hamcast modem name for position 4 in the rotation
	std::string _info;              //!< Information field in the config panel
	std::string _modem;             //!< Tramit modem.
	std::string _path;              //!< Storage for path when running script files.
	std::string _queue_filename;    //!< Storage for queue filename.
	std::string _queue_path;        //!< Storage for queue path.
	std::string _queue_filepath;    //!< Storage for queue script file name and path

	SCRIPT_COMMANDS *_script_command_table;    //!< Table of commands and vector functions @sees cript_cmds
	size_t _script_command_table_count;        //!< Number of assigned positions in the table.
	size_t _script_command_table_total_count;  //!< Number of positions in the table

	ScriptParsing *_parent;       //!< Calling ScriptParsing pointer. Primarly used for the reset command on the local 'this' pointer.

	char line_buffer[MAX_READLINE_LENGTH + 1]; //!< Line buffer for script read operations.
	char error_cmd_buffer[MAX_COMMAND_LENGTH];
	char error_string[MAX_COMMAND_LENGTH];
	char error_buffer[MAX_COMMAND_LENGTH + MAX_COMMAND_LENGTH + 1];

public:

	SCRIPT_CODES parse_commands(char *file_name_path); //!< The calling function to parse a script file.

	bool auto_load_queue(void) { return _auto_load_queue; }
	void auto_load_queue(bool value) { _auto_load_queue = value; }

	int base(void) { return _base; }
	void base(int value) { _base = value; }

	int blocks(void) { return _blocks; }
	void blocks(int value) { _blocks = value; }

	std::string call_from(void) { return _call_from; }
	void call_from(std::string value) { _call_from = value; }

	std::string call_to(void) { return _call_to; }
	void call_to(std::string value) { _call_to = value; }

	bool clear_missing(void) { return _clear_missing; }
	void clear_missing(bool value) { _clear_missing = value; }

	bool clear_rxq(void) { return _clear_rxq; }
	void clear_rxq(bool value) { _clear_rxq = value; }

	bool clear_txq(void) { return _clear_txq; }
	void clear_txq(bool value) { _clear_txq = value; }

	bool comp(void) { return _comp; }
	void comp(bool value) { _comp = value; }

	int event_type(void) { return _event_type; }
	void event_type(int value) { _event_type = value; }

	std::string event_times(void) { return _envent_times; }
	void event_times(std::string value) { _envent_times = value; }

	bool event_timed(void) { return _event_timed; }
	void event_timed(bool value) { _event_timed = value; }

	bool event(void) { return _event; }
	void event(bool value) { _event = value; }

	bool event_forever(void) { return _event_forever; }
	void event_forever(bool value) { _event_forever = value; }

	std::string desc(void) { return _desc; }
	void desc(std::string value) { _desc = value; }

	std::string file(void) { return _file; }
	void file(std::string value) { _file = value; }

	int file_type(void) { return _file_type; }
	void file_type(int value) { _file_type = value; }

	std::string hamcast_modem_1(void) { return _hamcast_modem_1; }
	void hamcast_modem_1(std::string value) { _hamcast_modem_1 = value; }

	std::string hamcast_modem_2(void) { return _hamcast_modem_2; }
	void hamcast_modem_2(std::string value) { _hamcast_modem_2 = value; }

	std::string hamcast_modem_3(void) { return _hamcast_modem_3; }
	void hamcast_modem_3(std::string value) { _hamcast_modem_3 = value; }

	std::string hamcast_modem_4(void) { return _hamcast_modem_4; }
	void hamcast_modem_4(std::string value) { _hamcast_modem_4 = value; }

	bool hamcast_modem_1_enable(void) { return _hamcast_modem_1_enable; }
	void hamcast_modem_1_enable(bool value) { _hamcast_modem_1_enable = value; }

	bool hamcast_modem_2_enable(void) { return _hamcast_modem_2_enable; }
	void hamcast_modem_2_enable(bool value) { _hamcast_modem_2_enable = value; }

	bool hamcast_modem_3_enable(void) { return _hamcast_modem_3_enable; }
	void hamcast_modem_3_enable(bool value) { _hamcast_modem_3_enable = value; }

	bool hamcast_modem_4_enable(void) { return _hamcast_modem_4_enable; }
	void hamcast_modem_4_enable(bool value) { _hamcast_modem_4_enable = value; }

	bool hamcast(void) { return _event; }
	void hamcast(bool value) { _event = value; }

	std::string header_modem(void) { return _header_modem; }
	void header_modem(std::string value) { _header_modem = value; }

	bool header_modem_enable(void) { return _header_modem_enable; }
	void header_modem_enable(bool value) { _header_modem_enable = value; }

	int hdr_repeat(void) { return _hdr_repeat; }
	void hdr_repeat(int value) { _hdr_repeat = value; }

	std::string info(void) { return _info; }
	void info(std::string value) { _info = value; }

	bool inhibit_header(void) { return _inhibit_header; }
	void inhibit_header(bool value) { _inhibit_header = value; }

	bool interval(void) { return _interval; }
	void interval(bool value) { _interval = value; }

	bool load_txdir(void) { return _load_txdir; }
	void load_txdir(bool value) { _load_txdir = value; }

	std::string modem(void) { return _modem; }
	void modem(std::string value) { _modem = value; }

	std::string path(void) { return _path; }
	void path(std::string value) { _path = value; }

	bool proto(void) { return _proto; }
	void proto(bool value) { _proto = value; }

	std::string queue_filename(void) { return _queue_path; }
	void queue_filename(std::string value) { _queue_path = value; }

	std::string queue_path(void) { return _queue_filename; }
	void queue_path(std::string value) { _queue_filename = value; }

	std::string queue_filepath(void) { return _queue_filepath; }
	void queue_filepath(std::string value) { _queue_filepath = value; }

	int reset(void) { return _reset; }
	void reset(int value) { _reset = value; }

	int rx_interval(void) { return _rx_interval; }
	void rx_interval(int value) { _rx_interval = value; }

	bool sync_with_flamp(void) { return _sync_with_flamp; }
	void sync_with_flamp(bool value) { _sync_with_flamp = value; }

	bool sync_with_fldigi(void) { return _sync_with_fldigi; }
	void sync_with_fldigi(bool value) { _sync_with_fldigi = value; }

	bool sync_with_prior(void) { return _sync_with_prior; }
	void sync_with_prior(bool value) { _sync_with_prior = value; }

	int tx_interval(void) { return _tx_interval; }
	void tx_interval(int value) { _tx_interval = value; }

	bool tx_report(void) { return _tx_report; }
	void tx_report(bool value) { _tx_report = value; }

	bool unproto_markers(void) { return _unproto_markers; }
	void unproto_markers(bool value) { _unproto_markers = value; }

	bool warn_user(void) { return _warn_user; }
	void warn_user(bool value) { _warn_user = value; }

	int xmit_repeat(void) { return _xmit_repeat; }
	void xmit_repeat(int value) { _xmit_repeat = value; }

	ScriptParsing *parent(void) { return _parent; }
	void parent(ScriptParsing *value) { _parent = value; }

	int sub_script_count(void) { return _sub_script_count; }
	void sub_script_count(int value) { _sub_script_count = value; }

	size_t script_command_table_total_count(void) { return _script_command_table_total_count; }
	void script_command_table_total_count(size_t value) { _script_command_table_total_count = value; }

	size_t script_command_table_count(void) { return _script_command_table_count; }
	void script_command_table_count(size_t value) { _script_command_table_count = value; }

	SCRIPT_COMMANDS * script_command_table(void) { return _script_command_table; }
	void script_command_table(SCRIPT_COMMANDS * value) { _script_command_table = value; }

	int assign_callback(const char *scriptCommand, int (*cb)(ScriptParsing *sp, SCRIPT_COMMANDS *sc));
	int assign_valid_parameters(const char *command, const char **array, const int array_count);

	void defaults(bool all);
	ScriptParsing();
	~ScriptParsing();

private:

	char * script_error_string(SCRIPT_CODES error_no, int line_number, char *cmd);
	char * skip_alpha_numbers(char * data, char *limit, SCRIPT_CODES &error);
	char * skip_characters(char * data, char *limit, SCRIPT_CODES &error);
	char * skip_numbers(char * data, char *limit, SCRIPT_CODES &error);
	char * skip_spaces(char * data, char *limit, SCRIPT_CODES &error);
	char * skip_to_character(char c, char * data, char *limit, SCRIPT_CODES &error);
	char * skip_white_spaces(char * data, char *limit, SCRIPT_CODES &error);

	inline SCRIPT_CODES test_on_off_state(bool &state, char *string, char *true_state);

	int call_callback(SCRIPT_COMMANDS *cb_data);
	int check_parameters_from_list(SCRIPT_COMMANDS *sc);
	int CopyScriptParsingEnv(ScriptParsing *src);

	SCRIPT_CODES copy_command(char *buffer, char *cPtr, char *ePtr, size_t limit);
	SCRIPT_CODES parse_parameters(char *s, char *d, SCRIPT_COMMANDS *matching_command);
	SCRIPT_CODES parse_single_command(char *data, size_t buffer_count);
	SCRIPT_CODES remove_crlf_comments(char *data, char *limit, size_t &count);
	SCRIPT_CODES sc_auto_load_queue(struct script_cmds *);
	SCRIPT_CODES sc_base_encode(struct script_cmds *);
	SCRIPT_CODES sc_block_count(struct script_cmds *);
	SCRIPT_CODES sc_call_from(struct script_cmds *);
	SCRIPT_CODES sc_call_to(struct script_cmds *);
	SCRIPT_CODES sc_clear_missing(struct script_cmds *);
	SCRIPT_CODES sc_clear_rxq(struct script_cmds *);
	SCRIPT_CODES sc_clear_txq(struct script_cmds *);
	SCRIPT_CODES sc_compression(struct script_cmds *);
	SCRIPT_CODES sc_dummy(struct script_cmds *);
	SCRIPT_CODES sc_event_forever(struct script_cmds *);
	SCRIPT_CODES sc_event_times(struct script_cmds *);
	SCRIPT_CODES sc_event_timed(struct script_cmds *);
	SCRIPT_CODES sc_event_type(struct script_cmds *);
	SCRIPT_CODES sc_event(struct script_cmds *);
	SCRIPT_CODES sc_file(struct script_cmds *);
	SCRIPT_CODES sc_hamcast_modem(struct script_cmds *);
	SCRIPT_CODES sc_hamcast(struct script_cmds *);
	SCRIPT_CODES sc_hdr_repeat(struct script_cmds *);
	SCRIPT_CODES sc_header_modem(struct script_cmds *cmd);
	SCRIPT_CODES sc_header(struct script_cmds *cmd);
	SCRIPT_CODES sc_info(struct script_cmds *);
	SCRIPT_CODES sc_inhibit_header(struct script_cmds *);
	SCRIPT_CODES sc_interval(struct script_cmds *);
	SCRIPT_CODES sc_load_queue(struct script_cmds *);
	SCRIPT_CODES sc_load_txdir(struct script_cmds *);
	SCRIPT_CODES sc_modem(struct script_cmds *);
	SCRIPT_CODES sc_path(struct script_cmds *);
	SCRIPT_CODES sc_proto(struct script_cmds *);
	SCRIPT_CODES sc_queue_filepath(struct script_cmds *);
	SCRIPT_CODES sc_reset(struct script_cmds *);
	SCRIPT_CODES sc_rx_interval(struct script_cmds *);
	SCRIPT_CODES sc_sync_with(struct script_cmds *);
	SCRIPT_CODES sc_tx_interval(struct script_cmds *);
	SCRIPT_CODES sc_tx_report(struct script_cmds *);
	SCRIPT_CODES sc_unproto_makers(struct script_cmds *);
	SCRIPT_CODES sc_warn_user(struct script_cmds *);
	SCRIPT_CODES sc_xmit_repeat(struct script_cmds *);
	SCRIPT_CODES sc_xmit_times(struct script_cmds *);

	SCRIPT_CODES check_parameters(struct script_cmds *);
	SCRIPT_CODES check_filename(char *filename, char *full_name_path, size_t limit, int path_flag);
	SCRIPT_CODES check_path(const char *);
	SCRIPT_CODES check_numbers(char *, paramter_types p);

	SCRIPT_COMMANDS * search_command(const char *command);
	
	void assign_func(size_t pos, calling_func func, size_t limit);
	void to_uppercase(char *str, int limit);
	void to_uppercase(std::string &str);
	void trim(char *buffer, size_t size);
	void initialize_function_members(void);
	int str_cnt(char * str, int count_limit);
};

int callback_dummy(ScriptParsing *sp, struct script_cmds *sc);

#endif /* defined(__script_parsing__) */
