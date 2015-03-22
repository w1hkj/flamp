/** **************************************************************
 \page script_parsing Script Parsing Class

 \par script_parsing.cxx (FLAMP)

 \par Author(s):
 Robert Stiles, KK5VD, Copyright &copy; 2014
 <br>
 <br>
 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. This software is distributed in
 the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more details. You
 should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 <br>
 <br>

 \par USE:
 Create a structure of data contained within the SCRIPT_COMMANDS
 structure \(see \ref script_cmds\).  Each element are as follows.<br>

 \verbatim
 { SCRIPT_COMMAND, 0, "AUTO LOAD QUEUE",  1, {0}, { p_string }, 0, 0, 0, 0 }
 \endverbatim

 \par "int flags <SCRIPT_COMMAND and/or QUEUE_COMMAND>"
 Indicates if the script command is a global assignment (SCRIPT_COMMAND)
 or local for each of the queued files (QUEUE_COMMAND).

 \par "size_t command_length"
 Programmer assignment not required. Internally Assigned.  This is the
 length of the script command in bytes.

 \par "char command[MAX_COMMAND_LENGTH]"
 The script command used to match within the content of the script
 file. MAX_COMMAND_LENGTH is the maximum length of the supplied buffer.

 \par "int  argc"
 The number of argument as required for the script command. The
 parsing engine will limit the number of scanned parameters even of more
 has been supplied.

 \par "char *args[MAX_CMD_PARAMETERS]"
 An array of char string pointers to each parameter scanned for
 the specific command. MAX_CMD_PARAMETERS is the maximum number
 of positions available in the parameter buffer table.

 \par "enum paramter_types param_check[MAX_CMD_PARAMETERS]"
 A list of validation flags for each parameters required.
 MAX_CMD_PARAMETERS is the maximum number of positions available
 in the param_check buffer table. See \ref paramter_types for a list
 of valid values.<br>

 \par "calling_func func"
 When a command string is matched in the script file, this StringParsing
 Class member is executed for further processing of the command
 paramaters. This can include any validation checks if required. The member
 functions are assigned during the creation of the class instance. See the
 constructor member ScriptParsing::ScriptParsing() for details.

 \par "const char **valid_values"
 List of valid paramters. Use function
 int assign_valid_parameters(const char *command, const char **array, const int array_count);
 to asssign the values to a specific script command.

 \par "int valid_value_count"
 Number of valid paramters.

 \par "int (*cb)(ScriptParsing *sp, struct script_cmds *sd)"
 This is assigned using the StringParsing Member:<br>
 <br>
 int assign_callback(const char *scriptCommand, int (*cb)(ScriptParsing *sp, SCRIPT_COMMANDS *sc));<br>
 <br>
 The function which is supplied by the programmer after the creation of the
 Class instance is called when the command is matched. This allows the
 programmer access to a multitude of information for further processing
 outside of the ScriptParsing Class instance.<br>
 <br>
 Example:<br>
 \verbatim
 #include "script_parsing.h"

 static const char *modems[] = {
 "BPSK31",
 "MFSK32",
 "MT63-500",
 "MFSK64"
 };

 int callback(ScriptParsing *sp, struct script_cmds *sc)
 {
 // do something
 return 0;
 }

 int main(int argc, const char * argv[])
 {
 ScriptParsing *sp = new ScriptParsing;

 if(sp) {
 sp->assign_valid_parameters("MODEM", modems, sizeof(modems)/sizeof(char *));
 sp->assign_callback("FILE", callback);
 sp->parse_commands((char *)"/fldigi-dev/test_parse_commands/running_test.txt");
 }

 return 0;
 }
 \endverbatim
 <br>
 See \ref script_parsing_class and \ref script_cmds for details about
 what data is provided by the ScriptParsing *sp and SCRIPT_COMMANDS *sc
 pointers. The passed SCRIPT_COMMANDS *sc pointer is a copy of the
 original data. Modification of this information does not alter the
 internal data.<br><br>
 <b>Note:</b> The member and function pointers within the SCRIPT_COMMANDS
 *sc pointer are set to dummy functions which return back to the caller
 if executed.
 *******************************************************************/

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

//#define EXTERNAL_TESTING
#undef EXTERNAL_TESTING
#ifdef EXTERNAL_TESTING
#define TESTING 1
#define LOG_INFO printf
#else
#include "debug.h"
#endif

#ifdef __WIN32__
#define PATH_SEPERATOR "\\"
#define PATH_CHAR_SEPERATOR '\\'
#include <direct.h>
#define get_current_dir _getcwd
#else
#define PATH_SEPERATOR "/"
#define PATH_CHAR_SEPERATOR '/'
#include <unistd.h>
#define get_current_dir getcwd
#endif

#include "script_parsing.h"

// This table (by reference) is not used. Copy to another memory location.
// Do not change the order of this without changing the order of
// void ScriptParsing::initialize_function_members(void) to match.

static const SCRIPT_COMMANDS default_script_command_table[] = {
	{ CMD_AUTO_LOAD_QUEUE, SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_BASE,            SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 },
	{ CMD_BLOCKS,          SCRIPT_COMMAND,                 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 },
	{ CMD_CALLFROM,        SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_CALLTO,          SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_CLEAR_MISSING,   SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_CLEAR_RXQ,       SCRIPT_COMMAND,                 0,  0, {0}, { p_null },                   0, 0, 0, 0 },
	{ CMD_CLEAR_TXQ,       SCRIPT_COMMAND | QUEUE_COMMAND, 0,  0, {0}, { p_null },                   0, 0, 0, 0 },
	{ CMD_COMP,            SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_EVENT_FOREVER,   SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_EVENT_TIMED,     SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_EVENT_TIMES,     SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_EVENT_TYPE,      SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_EVENT,           SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_FILE,            SCRIPT_COMMAND | QUEUE_COMMAND, 0,  2, {0}, { p_filename, p_string },     0, 0, 0, 0 },
	{ CMD_HAMCAST_MODEM,   SCRIPT_COMMAND,                 0,  2, {0}, { p_unsigned_int, p_string }, 0, 0, 0, 0 },
	{ CMD_HAMCAST,         SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_HDR_REPEAT,      SCRIPT_COMMAND,                 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 },
	{ CMD_HEADER_MODEM,    SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_HEADER,          SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_INFO,            SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_INHIBIT_HEADER,  SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_INTERVAL,        SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_LOAD_QUEUE,      SCRIPT_COMMAND | QUEUE_COMMAND, 0,  2, {0}, { p_string, p_string },       0, 0, 0, 0 },
	{ CMD_LOAD_TXDIR,      SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_string, p_string },       0, 0, 0, 0 },
	{ CMD_MODEM,           SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_PATH,            SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_path },                   0, 0, 0, 0 },
	{ CMD_PROTO,           SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_QUEUE_FILEPATH,  SCRIPT_COMMAND | QUEUE_COMMAND, 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_RESET,           SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_RX_INTERVAL,     SCRIPT_COMMAND,                 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 },
	{ CMD_SYNC_WITH,       SCRIPT_COMMAND,                 0,  2, {0}, { p_string, p_string },       0, 0, 0, 0 },
	{ CMD_TX_INTERVAL,     SCRIPT_COMMAND,                 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 },
	{ CMD_TX_REPORT,       SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_UNPROTO_MARKERS, SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_WARN_USER,       SCRIPT_COMMAND,                 0,  1, {0}, { p_string },                 0, 0, 0, 0 },
	{ CMD_XMIT_REPEAT,     SCRIPT_COMMAND,                 0,  1, {0}, { p_unsigned_int },           0, 0, 0, 0 }
};

/** **************************************************************
 * \brief Assign a list of valid parameters for verification checks.
 * \param array An Array of pointers to each element.
 * \param array_count Number of entries in the array.
 * \return the array count or '0' if error.
 * \par Note:
 * This array is limited to the first parameter of the command
 * used in it's comparison.
 *****************************************************************/
int ScriptParsing::assign_valid_parameters(const char *command, const char **array, const int array_count)
{
	if(!array || array_count < 1 || !command) return 0;

	int index = 0;
	int count = 0;

	SCRIPT_COMMANDS * cmd_sc = search_command(command);

	if(!cmd_sc) {
		return 0;
	}

	for(index = 0; index < array_count; index++) {
		if(*array[index]) count++;
	}

	if(count != array_count) return 0;

	cmd_sc->valid_values = array;
	cmd_sc->valid_value_count = array_count;

	return array_count;
}

/** **************************************************************
 * \brief Return true state if string is matched.
 * \param state Referenced value to assign results to.
 * \param string Pointer to the data string.
 * \param true_state Pointer to the data to match with.
 * \return SCRIPT_CODES error code.
 *****************************************************************/
inline SCRIPT_CODES ScriptParsing::test_on_off_state(bool &state, char *string, char *true_state=(char *)"ON")
{
	if(!string || !true_state) {
		return script_function_parameter_error;
	}

	bool flag = false;

	if(strncmp(string, true_state, MAX_PARAMETER_LENGTH) == 0)
		flag = true;

	state = flag;

	return script_no_errors;
}

/** **************************************************************
 * \brief Validate if file is located in the specified location.
 * \param filename Pointer to a series of charcters
 * \return SCRIPT_CODES error code.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::check_filename(char *filename, char *full_name_path=0, size_t limit=0, int path_flag=SCRIPT_COMMAND)
{
	char *filename_path = (char *)0;
	char *path = (char *)0;
	SCRIPT_CODES error = script_no_errors;
	std::string user_path = "";

	if(!filename) {
		return script_function_parameter_error;
	}

	filename_path = new char[FILENAME_MAX + 1];

	if(!filename_path) {
		return script_memory_allocation_error;
	}

	memset(filename_path, 0, (FILENAME_MAX + 1));

	path = new char[FILENAME_MAX + 1];

	if(!path) {
		delete [] filename_path;
		return script_memory_allocation_error;
	}

	memset(path, 0, (FILENAME_MAX + 1));

	if(path_flag == QUEUE_COMMAND) {
		user_path.assign(this->queue_path());
	} else {
		user_path.assign(this->path());
	}

	if(user_path.empty()) {
		get_current_dir(path, FILENAME_MAX);
	} else {
		strncpy(path, user_path.c_str(), FILENAME_MAX);
	}

	size_t size = strnlen(path, FILENAME_MAX);

	if(size > 1) {
		if(path[size - 1] != PATH_CHAR_SEPERATOR) {
			strncat(path, PATH_SEPERATOR, FILENAME_MAX);
		}
	}

	strncpy(filename_path, path, FILENAME_MAX);
	strncat(filename_path, filename, FILENAME_MAX);

#ifdef TESTING
	printf("     filename = %s\n", filename);
	printf("         path = %s\n", path);
	printf("filename_path = %s\n", filename_path);
#endif

	FILE *fd = (FILE *)0;

	fd = fopen(filename_path, "r");

	if(!fd) {
		error = script_file_not_found;
	} else {
		fclose(fd);

		if(full_name_path && limit > 0)
			strncpy(full_name_path, filename_path, limit);
	}

	delete [] filename_path;
	delete [] path;

	return error;
}

/** **************************************************************
 * \brief Validate if path is present.
 * \param path The path to verify.
 * \return SCRIPT_CODES error code.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::check_path(const char *path)
{
	if(!path) {
		return script_function_parameter_error;
	}

	struct stat st;
	memset(&st, 0, sizeof(struct stat));

	if(stat(path, &st) == 0) {
		if(st.st_mode & S_IFDIR)
			return script_no_errors;
	}

	return script_path_not_found;
}

/** **************************************************************
 * \brief Validate if the parameter is a value.
 * \param value The string in question.
 * \param p format verification.
 * \return SCRIPT_CODES error code.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::check_numbers(char *value, paramter_types p)
{
	SCRIPT_CODES error = script_no_errors;
	size_t length = 0;
	size_t index = 0;
	int data_count = 0;
	int signed_value = 0;
	int decimal_point = 0;

	if(!value)
		return script_function_parameter_error;

	length = strnlen(value, MAX_PARAMETER_LENGTH);

	if(length < 1)
		return script_parameter_error;

	// Skip any leading white spaces.
	for(index = 0; index < length; index++) {
		if(value[index] > ' ')
			break;
	}

	if((index >= length))
		return script_parameter_error;

	switch(p) {
		case p_int:
		case p_long:

			if(value[0] == '-' || value[0] == '+') {
				index++;
				signed_value++;
			}

		case p_unsigned_int:
		case p_unsigned_long:

			for(; index< length; index++) {
				if(isdigit(value[index]))
					data_count++;
				else
					break;
			}
			break;

			if(data_count)
				return script_no_errors;

		case p_float:
		case p_double:
			if(value[0] == '-' || value[0] == '+') {
				index++;
				signed_value++;
			}

			for(; index< length; index++) {
				if(isdigit(value[index]))
					data_count++;

				if(value[index] == '.')
					decimal_point++;

				if(decimal_point > 1)
					return script_parameter_error;
			}

			if(data_count)
				return script_no_errors;

			break;

		default:;

	}

	return error;
}

/** **************************************************************
 * \brief Validate the script parameter(s) are of the expected format.
 * \param cmd Matching command data structure.
 * \param p A table of expected parameters types (null terminated).
 * \param p_count the number of 'p[]' items in the table (includes null termination).
 * \return SCRIPT_CODES error code.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::check_parameters(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_no_errors;
	int count   = 0;
	int index   = 0;
	size_t size = 0;

	if(!cmd)
		return script_function_parameter_error;

	count = cmd->argc;

	if(count < 1)
		return script_no_errors;

	for(index = 0; index < count; index++) {

		if(!cmd->args[index]) {
			return script_args_eol;
		}

		if(cmd->param_check[index] == p_null) {
			size = 0;
		} else {
			size = strnlen(cmd->args[index], MAX_COMMAND_LENGTH);
		}

		switch(cmd->param_check[index]) {
			case p_null:
				error = script_param_check_eol;
				break;

			case p_char:
				if(size > 1)
					error = script_paramter_exceeds_length;
				break;

			case p_int:
			case p_long:
			case p_unsigned_int:
			case p_unsigned_long:
			case p_float:
			case p_double:
				error = check_numbers(cmd->args[index], cmd->param_check[index]);
				break;

			case p_string:
				if(size < 1)
					error = script_parameter_error;
				break;

			case p_path:
				error = check_path(cmd->args[index]);
				break;

			case p_filename:
				error = check_filename(cmd->args[index]);
				break;
		}

		if(error != script_no_errors)
			break;
	}

	return error;
}

/** **************************************************************
 * \brief Search the content of SCRIPT_COMMANDS structure table
 * for the specified command.
 * \param command The command to search for.
 * \return Pointer to the matching SCRIPT_COMMANDS entry. Null if
 * not found.
 *****************************************************************/
SCRIPT_COMMANDS * ScriptParsing::search_command(const char *command)
{
	char *cmd_buffer = (char *)0;
	int diff = 0;
	SCRIPT_COMMANDS * found = (SCRIPT_COMMANDS *) 0;
	size_t count = _script_command_table_count;
	size_t index = 0;

	if(!command) return found;

	cmd_buffer = new char [MAX_COMMAND_LENGTH];

	if(!cmd_buffer) {
		LOG_INFO("cmd_buffer allocation error near line %d", __LINE__);
		return found;
	}

	memset(cmd_buffer, 0, MAX_COMMAND_LENGTH);
	strncpy(cmd_buffer, command, MAX_COMMAND_LENGTH-1);

	to_uppercase(cmd_buffer, (int) MAX_COMMAND_LENGTH);
	trim(cmd_buffer, (int) MAX_COMMAND_LENGTH);

	for(index = 0; index < count; index++) {
		diff = strncmp(cmd_buffer, _script_command_table[index].command, MAX_COMMAND_LENGTH);
		if(diff == 0) {
			found = &_script_command_table[index];
			break;
		}
	}

	cmd_buffer[0] = 0;
	delete [] cmd_buffer;

	return found;
}

/** **************************************************************
 * \brief Convert string to uppercase characters.<br>
 * \par str Pointer to data.
 * \par limit data buffer size
 * \return void
 *****************************************************************/
void ScriptParsing::to_uppercase(char *str, int limit)
{
	if(!str || limit < 1) return;

	int character = 0;
	int count     = 0;
	int index     = 0;

	count = (int) strnlen(str, limit);

	for(index = 0; index < count; index++) {
		character = str[index];
		if(character == 0) break;
		character = (char) toupper(character);
		str[index] = character;
	}
}

/** **************************************************************
 * \brief Convert string to uppercase characters.<br>
 * \par str String storage passed by reference.
 * \return void
 *****************************************************************/
void ScriptParsing::to_uppercase(std::string &str)
{
	int character = 0;
	int count     = 0;
	int index     = 0;

	count = (int) str.length();

	for(index = 0; index < count; index++) {
		character = str[index];
		if(character == 0) break;
		character = (char) toupper(character);
		str[index] = character;
	}
}

/** **************************************************************
 * \brief Assign Call back function to a given script command.<br>
 * \param scriptCommand Script command string<br>
 * \param cb Pointer to call back function. int (*cb)(ScriptParsing *sp, SCRIPT_COMMANDS *sc)
 *****************************************************************/
int ScriptParsing::assign_callback(const char *scriptCommand, int (*cb)(ScriptParsing *sp, SCRIPT_COMMANDS *sc))
{
	char *cmd_buffer = (char *)0;
	int diff = 0;
	size_t count = _script_command_table_count;
	size_t index = 0;

	if(!scriptCommand || !cb) return 0;

	cmd_buffer = new char[MAX_COMMAND_LENGTH];

	if(!cmd_buffer) {
		LOG_INFO("cmd_buffer allocation error near line %d", __LINE__);
		return 0;
	}

	memset(cmd_buffer, 0, MAX_COMMAND_LENGTH);
	strncpy(cmd_buffer, scriptCommand, MAX_COMMAND_LENGTH-1);

	to_uppercase(cmd_buffer, (int) MAX_COMMAND_LENGTH);
	trim(cmd_buffer, (int) MAX_COMMAND_LENGTH);

	for(index = 0; index < count; index++) {
		diff = strncmp(cmd_buffer, _script_command_table[index].command, MAX_COMMAND_LENGTH);
		if(diff == 0) {
			if(_script_command_table[index].cb)
				LOG_INFO("Over writing call back funcion for \"%s\"", cmd_buffer);
			_script_command_table[index].cb = cb;
			break;
		}
	}

	cmd_buffer[0] = 0;
	delete [] cmd_buffer;

	return 0;
}

/** **************************************************************
 * \brief Assign func to func array checking array bounds.
 * \param pos Position in the indexed array
 * \param func The function (member) to assign
 * \param limit Array count limit
 * \return void (nothing)
 *****************************************************************/
void ScriptParsing::assign_func(size_t pos, calling_func func, size_t limit)
{
	if(pos < limit) {
		_script_command_table[pos].func = func;
		_script_command_table[pos].command_length = strnlen(_script_command_table[pos].command, MAX_COMMAND_LENGTH);
	}
}

/** **************************************************************
 * \brief Initialize callable members.
 * \return void (nothing)
 *****************************************************************/
void ScriptParsing::defaults(bool all)
{
	if(all) {
		_call_from        = "";
		_call_to          = "";
	}

	_clear_missing          = false;
	_event                  = false;
	_event_forever          = false;
	_hamcast                = false;
	_hamcast_modem_1_enable = false;
	_hamcast_modem_2_enable = false;
	_hamcast_modem_3_enable = false;
	_hamcast_modem_4_enable = false;
	_inhibit_header         = false;
	_interval               = false;
	_proto                  = true;
	_sync_with_flamp        = false;
	_sync_with_fldigi       = false;
	_sync_with_prior        = false;
	_tx_report              = false;
	_warn_user              = false;
}

/** **************************************************************
 * \brief Initialize callable members.
 * \return void (nothing)
 *****************************************************************/
void ScriptParsing::initialize_function_members(void)
{
	// Ensure this is in the same sequence as the structure it's assigned to.
	int index = 0;
	assign_func(index++, &ScriptParsing::sc_auto_load_queue,     _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_base_encode,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_block_count,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_call_from,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_call_to,             _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_clear_missing,       _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_clear_rxq,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_clear_txq,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_compression,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_event_forever,       _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_event_timed,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_event_times,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_event_type,          _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_event,               _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_file,                _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_hamcast_modem,       _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_hamcast,             _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_hdr_repeat,          _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_header_modem,        _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_header,              _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_info,                _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_inhibit_header,      _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_interval,            _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_load_queue,          _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_load_txdir,          _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_modem,               _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_path,                _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_proto,               _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_queue_filepath,      _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_reset,               _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_rx_interval,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_sync_with,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_tx_interval,         _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_tx_report,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_unproto_makers,      _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_warn_user,           _script_command_table_total_count);
	assign_func(index++, &ScriptParsing::sc_xmit_repeat,         _script_command_table_total_count);
}

/** **************************************************************
 * \brief Constructor: Copy and initialize function arrays.<br>
 *****************************************************************/
ScriptParsing::ScriptParsing()
{
	size_t count = 0;

	// Initialize class variables.

	_auto_load_queue  = false;
	_base             = 0;
	_blocks           = 0;
	_call_from        = "";
	_call_to          = "";
	_clear_missing    = false;
	_clear_rxq        = false;
	_clear_txq        = false;
	_comp             = false;
	_desc             = "";
	_envent_times     = "";
	_event            = false;
	_event_forever    = false;
	_event_type       = 0;
	_file             = "";
	_file_type        = SCRIPT_COMMAND;
	_hamcast          = false;
	_hamcast_modem_1  = "";
	_hamcast_modem_1_enable = false;
	_hamcast_modem_2  = "";
	_hamcast_modem_2_enable = false;
	_hamcast_modem_3  = "";
	_hamcast_modem_3_enable = false;
	_hamcast_modem_4  = "";
	_hamcast_modem_4_enable = false;
	_hdr_repeat       = 0;
	_info             = "";
	_inhibit_header   = false;
	_interval         = false;
	_modem            = "";
	_path             = "";
	_proto            = true;
	_queue_filename   = "";
	_queue_path       = "";
	_reset            = 0;
	_rx_interval      = 0;
	_sync_with_flamp  = false;
	_sync_with_fldigi = false;
	_sync_with_prior  = false;
	_tx_interval      = 0;
	_tx_report        = false;
	_warn_user        = false;
	_xmit_repeat      = false;

	_sub_script_count = 0;
	_parent = (ScriptParsing *)0;

	_script_command_table = (SCRIPT_COMMANDS *)0;
	_script_command_table_count = 0;
	_script_command_table_total_count = 0;

	count = sizeof(default_script_command_table)/sizeof(SCRIPT_COMMANDS);

	_script_command_table  = new SCRIPT_COMMANDS[count + 1];

	if(!_script_command_table) {
		return;
	}

	_script_command_table_count = count;
	_script_command_table_total_count = count + 1;

	memset(_script_command_table, 0, sizeof(SCRIPT_COMMANDS) * _script_command_table_total_count);
	memcpy(_script_command_table, default_script_command_table, sizeof(default_script_command_table));

	memset(line_buffer, 0, sizeof(line_buffer));

	initialize_function_members();

}

/** **************************************************************
 * \brief Copy environment to the sub ScriptParsing class
 * \param src Source Class pointer to copy from.
 *****************************************************************/
int ScriptParsing::CopyScriptParsingEnv(ScriptParsing *src)
{
	if(!src || (src == this)) return -1;

#if 0 // Do not copy these variables.
	desc(src->desc());
	file(src->file());
	load_queue(src->load_queue());
#endif // 0

	auto_load_queue(src->auto_load_queue());
	base(src->base());
	blocks(src->blocks());
	call_from(src->call_from());
	call_to(src->call_to());
	clear_missing(src->clear_missing());
	clear_rxq(src->clear_rxq());
	clear_txq(src->clear_txq());
	comp(src->comp());
	event_forever(src->event_forever());
	event_times(src->event_times());
	event_type(src->event_type());
	event(src->event());
	hamcast_modem_1_enable(src->hamcast_modem_1_enable());
	hamcast_modem_1(src->hamcast_modem_1());
	hamcast_modem_2_enable(src->hamcast_modem_2_enable());
	hamcast_modem_2(src->hamcast_modem_2());
	hamcast_modem_3_enable(src->hamcast_modem_3_enable());
	hamcast_modem_3(src->hamcast_modem_3());
	hamcast_modem_4_enable(src->hamcast_modem_4_enable());
	hamcast_modem_4(src->hamcast_modem_4());
	hamcast(src->hamcast());
	hdr_repeat(src->hdr_repeat());
	info(src->info());
	inhibit_header(src->inhibit_header());
	interval(src->interval());
	modem(src->modem());
	path(src->path());
	proto(src->proto());
	reset(src->reset());
	rx_interval(src->rx_interval());
	sync_with_flamp(src->sync_with_flamp());
	sync_with_fldigi(src->sync_with_fldigi());
	sync_with_prior(src->sync_with_prior());
	tx_interval(src->tx_interval());
	tx_report(src->tx_report());
	warn_user(src->warn_user());
	xmit_repeat(src->xmit_repeat());

	parent(src);
	script_command_table_count(src->script_command_table_count());
	script_command_table_total_count(src->script_command_table_total_count());
	sub_script_count(src->sub_script_count() + 1);

	SCRIPT_COMMANDS * dst_table = script_command_table();
	SCRIPT_COMMANDS * src_table = src->script_command_table();

	size_t index = 0;
	size_t count = script_command_table_count();

	for(index = 0; index < count; index++) {
		dst_table[index].cb                =  src_table[index].cb;
		dst_table[index].valid_value_count =  src_table[index].valid_value_count;
		dst_table[index].valid_values      =  src_table[index].valid_values;
	}

	initialize_function_members();

	return 0;
}

/** **************************************************************
 * \brief Enable/Disable Auto load of TX queue during timed event.<br>
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"AUTO LOAD QUEUE:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_auto_load_queue(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->auto_load_queue(state);
	}

	return error;
}

/** **************************************************************
 * \brief Set the base encoding type.<br>
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"BASE:\<64|128|256\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>64 = Base64</tt><br>
 * <tt>128 = Base128</tt><br>
 * <tt>256 = Base256</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_base_encode(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;

	if(cmd->argc && cmd->args[0]) {

		value = atoi(cmd->args[0]);

		switch(value) {
			case  64:
			case 128:
			case 256:
				break;

			default:
				LOG_INFO("%s Valid Parameters: 64, 128, or 256.", cmd->command);
				return script_invalid_parameter;
		}

		this->base(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Set the block count.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"Blocks:\<value\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>value = 16, 32, 48, ..., 2048</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_block_count(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;
	int modulus = 0;

	if(cmd->argc && cmd->args[0]) {
		value = atoi(cmd->args[0]);

		modulus = value % 16;

		if(modulus == 0) {
			this->blocks(value);
		}
		else {
			LOG_INFO("%s Parameters are modulus 16 values (16, 32,..., 2048).", cmd->command);
			return script_invalid_parameter;
		}
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Set the transmitting stations callsign.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"CALLFROM:\<string\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>string = ham radio operator call sign</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_call_from(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		std::string value = "";

		value.assign(cmd->args[0]);

		if(!value.empty())
			this->call_from(value);
		else
			return script_invalid_parameter;
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Set the receiving station(s) callsign.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"CALLTO:\<string\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>string = ham radio operator call sign</tt>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_call_to(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		std::string value = "";

		value.assign(cmd->args[0]);

		if(!value.empty())
			this->call_to(value);
		else
			return script_invalid_parameter;
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Set flag to clear received missing block on a fill
 * retransmission.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"CLEAR MISSING:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_clear_missing(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->clear_missing(state);
	}

	return error;
}

/** **************************************************************
 * \brief Remove all files in the receive queue.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"CLEAR RXQ:\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>None</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_clear_rxq(struct script_cmds *cmd)
{
	return script_no_errors;
}

/** **************************************************************
 * \brief Remove all files in the transmit queue.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"CLEAR TXQ:\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>None</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_clear_txq(struct script_cmds *cmd)
{
	return script_no_errors;
}

/** **************************************************************
 * \brief Enable/Disable file compression on transmitted file.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"COMP:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_compression(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->comp(state);
	}

	return error;
}

/** **************************************************************
 * \brief Set the event type used when events are enabled.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"EVENT TYPE:\<string\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>\"5 min\" = Transmit every 5 minutes</tt><br>
 * <tt>\"15 min\" = Transmit every 15 minutes</tt><br>
 * <tt>\"30 min\" = Transmit every 30 minutes</tt><br>
 * <tt>\"Hourly\" = Transmit every hour</tt><br>
 * <tt>\"Even hours\" = Transmit every even hour</tt><br>
 * <tt>\"Odd hours\" = Transmit every odd hour</tt><br>
 * <tt>\"Repeated at\" = Transmit at specific time intervals repeatedly</tt><br>
 * <tt>\"One time at\" = Transmit at specific time intervals once</tt><br>
 * <tt>\"Continuous at\" = Transmit between specific time intervals repeatedly</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_event_type(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	static const char *valid_values[] = {
		(char *) "5 MIN",
		(char *) "15 MIN",
		(char *) "30 MIN",
		(char *) "HOURLY",
		(char *) "EVEN HOURS",
		(char *) "ODD HOURS",
		(char *) "REPEATED AT",
		(char *) "ONE TIME AT",
		(char *) "CONTINUOUS AT"
	};

	static const int event_ref[] = {
		et_5_min,
		et_15_min,
		et_30_min,
		et_hourly,
		et_even_hours,
		et_odd_hours,
		et_repeat_at,
		et_one_time_at,
		et_continious_at
	};

	int match     = -1;
	SCRIPT_CODES error = script_invalid_parameter;
	size_t count  = (sizeof(valid_values)/sizeof(char *));
	size_t index  = 0;
	std::string value = "";

	if(cmd->argc && cmd->args[0]) {
		value.assign(cmd->args[0]);
		if(value.empty())
			return script_invalid_parameter;

		to_uppercase(value);

		for(index = 0; index < count; index++) {
			match = strncmp(value.c_str(), valid_values[index], MAX_PARAMETER_LENGTH);
			if(match == 0) {
				this->event_type(event_ref[index]);
				return script_no_errors;
			}
		}
	}

	return error;
}

/** **************************************************************
 * \brief Assign event times
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command
 * <tt>XMIT TIMES:<start_time-end_time> (Continuous at)</tt> or
 * <tt>XMIT TIMES:<single_event>,<next_single_event>,... (all other event types)</tt>
 * All times are in 0000 through 2359 Hr (zulu/utc) format
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_event_times(struct script_cmds *cmd)
{
	if(!cmd) return script_function_parameter_error;

	char *cPtr   = (char *)0;
	size_t count = cmd->argc;
	size_t hits  = 0;
	size_t index = 0;
	size_t j     = 0;
	size_t size  = 0;
	size_t valid_data = 0;
	std::string valid_string = "";

	if(!count) return script_parameter_error;

	valid_string.clear();

	for(index = 0; index < count; index++) {
		cPtr = cmd->args[index];
		if(!cPtr) break;

		size = strnlen(cPtr, MAX_PARAMETER_LENGTH);
		hits = 0;

		for(j = 0; j < size; j++) {
			if(!*cPtr) break;
			if(isdigit(*cPtr) || *cPtr == '-' || *cPtr == ' ') hits++;
			cPtr++;
		}

		if(hits == size) {
			valid_data++;
			valid_string.append(cmd->args[index]).append(" ");
		}
	}

	if(valid_data != count)
		return script_incorrectly_assigned_value;

	event_times(valid_string);

	return script_no_errors;
}

/** **************************************************************
 * \brief Enable timed event to occur.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see @ref script_codes
 * \par Script Command:<br>
 * <tt>\"EVENT TIMED:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_event_timed(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->event_timed(state);
	}

	return error;
}

/** **************************************************************
 * \brief Enable event transmission.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see @ref script_codes
 * \par Script Command:<br>
 * <tt>\"EVENT:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_event(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->event(state);
	}

	return error;
}

/** **************************************************************
 * \brief Enable forever timed event to occur.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see @ref script_codes
 * \par Script Command:<br>
 * <tt>\"EVENT FOREVER:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 * \par Note:
 * Requires event flag to be enabled. See ScriptParsing::sc_event()
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_event_forever(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->event_forever(state);
	}

	return error;
}

/** **************************************************************
 * \brief Varify and set file name / description parameters.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"FILE:<file_name.ext>\"</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_file(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	char *buffer = (char *)0;
	SCRIPT_CODES error = script_no_errors;
	std::string value = "";

	if(cmd->argc > 1) {
		buffer = new char[FILENAME_MAX];

		if(!buffer)
			return script_memory_allocation_error;

		memset(buffer, 0, FILENAME_MAX);

		error = check_filename(cmd->args[0], buffer, FILENAME_MAX-1);

		if(error != script_no_errors)
			return error;

		value.assign(buffer);

		delete [] buffer;

		if(value.empty())
			return script_parameter_error;

		this->file(value);

		value.assign(cmd->args[1]);

		if(value.size())
			this->desc(value);
		else
			this->desc("");

	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Assign modem type to hamcast position x
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"HAMCAST MODEM:<pos>,<modem_id_string>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt> pos = A value of 1, 2, 3, or 4</tt>
 * <tt> modem_id_string = MFSK32 (for example)</tt>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_hamcast_modem(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_no_errors;

	if(!cmd)
		return script_function_parameter_error;

	bool flag    = false;
	int diff     = 0;
	int index    = 0;
	int off_diff = 0;
	int on_diff  = 0;
	int pos      = 0;

	size_t size  = 0;
	std::string value = "";

	if(cmd->argc > 1) {
		if(cmd->args[0])
			pos = atoi(cmd->args[0]);

		if(pos < 1 || pos > 4) {
			LOG_INFO("Parameter 1 out of range. (1, 2, 3, or 4)");
			return script_invalid_parameter;
		}

		if(cmd->args[1]) {

			value.assign(cmd->args[1]);

			if(value.empty())
				return script_invalid_parameter;

			to_uppercase(value);

			on_diff  = strncmp(value.c_str(), "ON",  MAX_PARAMETER_LENGTH);
			off_diff = strncmp(value.c_str(), "OFF", MAX_PARAMETER_LENGTH);

			if(on_diff == 0 || off_diff == 0) {
				if(on_diff == 0) flag = true;
				else flag = false;

				switch(pos) {
					case 1:
						this->hamcast_modem_1_enable(flag);
						break;
					case 2:
						this->hamcast_modem_2_enable(flag);
						break;
					case 3:
						this->hamcast_modem_3_enable(flag);
						break;
					case 4:
						this->hamcast_modem_4_enable(flag);
						break;
				}

				return script_no_errors;
			}

			if(cmd->valid_values && cmd->valid_value_count) {
				for(index = 0; index < cmd->valid_value_count; index++) {
					diff = strncmp(cmd->args[1], cmd->valid_values[index], MAX_PARAMETER_LENGTH);
					if(diff == 0) {
						switch(pos) {
							case 1:
								this->hamcast_modem_1(value);
								break;
							case 2:
								this->hamcast_modem_2(value);
								break;
							case 3:
								this->hamcast_modem_3(value);
								break;
							case 4:
								this->hamcast_modem_4(value);
								break;
						}

						return script_no_errors;
					}
				}
				LOG_INFO("Non-matching/unavailable modem ID string (%s).", value.c_str());
				return script_invalid_parameter;
			}

			size = (int) strnlen(cmd->args[1], MAX_PARAMETER_LENGTH);

			if(size < 4) {
				error = script_invalid_parameter;
			} else {
				switch(pos) {
					case 1:
						this->hamcast_modem_1(value);
						break;
					case 2:
						this->hamcast_modem_2(value);
						break;
					case 3:
						this->hamcast_modem_3(value);
						break;
					case 4:
						this->hamcast_modem_4(value);
						break;
				}
			}
		}
	}

	return error;
}

/** **************************************************************
 * \brief Enable hamcast events
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"HAMCAST:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_hamcast(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->hamcast(state);
	}

	return error;
}

/** **************************************************************
 * \brief Enable/Disable Header modem use.<br>
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"HEADER:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_header(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->auto_load_queue(state);
	}

	return error;
}

/** **************************************************************
 * \brief Process and test modem parameters for validity.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par "Script Command:"<br>
 * <tt>\"HEADER MODEM:\<modem_id_string\>\"<br>
 * Example:\"HEADER MODEM:BPSK250\"</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_header_modem(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_no_errors;

	if(!cmd)
		return script_function_parameter_error;

	int diff  = 0;
	int index = 0;
	std::string value = "";

	if(cmd->argc > 0 && cmd->args[0]) {

		value.assign(cmd->args[0]);

		if(value.empty())
			return script_invalid_parameter;

		to_uppercase(value);

		if(strncmp(value.c_str(), "ON",  MAX_PARAMETER_LENGTH) == 0) {
			header_modem_enable(true);
			return script_no_errors;
		}

		if(strncmp(value.c_str(), "OFF",  MAX_PARAMETER_LENGTH) == 0) {
			header_modem_enable(false);
			return script_no_errors;
		}

		if(cmd->valid_values && cmd->valid_value_count) {
			for(index = 0; index < cmd->valid_value_count; index++) {
				diff = strncmp(cmd->args[0], cmd->valid_values[index], MAX_PARAMETER_LENGTH);
				if(diff == 0) {
					header_modem(cmd->args[0]);
					return script_no_errors;
				}
			}
			LOG_INFO("Non-matching/unavailable modem ID string (%s).", value.c_str());
			return script_invalid_parameter;
		}
	}

	return error;
}

/** **************************************************************
 * \brief Set the header repeat count
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"HDR REPEAT:<number>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>number = 1,2,3,...,10</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_hdr_repeat(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;

	if(cmd->argc && cmd->args[0]) {
		value = atoi(cmd->args[0]);

		if(value < 1) value = 1;
		if(value > 10) value = 10;

		this->hdr_repeat(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Assign INFO field
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"INFO:<string>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>string = Byte value 32 to byte value 255</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_info(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		std::string value = "";
		value.assign(cmd->args[0]);

		if(value.empty())
			return script_incorrectly_assigned_value;

		this->info(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Enable/Disable Header modem use.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"INHIBIT HEADER:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_inhibit_header(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->inhibit_header(state);
	}

	return error;
}

/** **************************************************************
 * \brief Enable/Disable interval timer.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"INTERVAL:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_interval(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->interval(state);
	}

	return error;
}

/** **************************************************************
 * \brief Load Queue script execution.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Commands:
 * LOAD QUEUE:FILE,<file_name><br>
 * LOAD QUEUE:PATH,<directory_path><br>
 * \par Parameters
 * FILE = Indicates next prarmeter is a file name of the queue load script.<br>
 * PATH = Indicates next prarmeter is a path to the file name.<br>
 * \par Note(s):
 * This command is recursive for a maximum of MAX_SUB_SCRIPTS times.
 * Queue scripts use a subset of availble commands see default_script_command_table[]
 * int flags field for details.  The parent ScriptParsing enviroment is
 * duplicated to the newly created subscript. Any modification to the subscript
 * environment will not effect the parent.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_load_queue(struct script_cmds *cmd)
{
	char *buffer = (char *)0;
	SCRIPT_CODES error = script_no_errors;
	std::string value = "";

	if(sub_script_count() >= MAX_SUB_SCRIPTS)
		return script_max_sub_script_reached;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc > 1 && cmd->args[0] && cmd->args[1]) {
		value.assign(cmd->args[0]);

		if(value.empty())
			return script_invalid_parameter;

		to_uppercase(value);

		if(strncmp(value.c_str(), "PATH", 4) == 0) {

			value.assign(cmd->args[1]);
			if(value.empty())
				return script_invalid_parameter;

			error = check_path(cmd->args[1]);

			if(error != script_no_errors)
				return error;

			this->queue_path(value);

			return script_no_errors;
		}

		if(strncmp(value.c_str(), "FILE", 4) == 0) {
			value.assign(cmd->args[1]);
			if(value.empty())
				return script_invalid_parameter;

			buffer = new char[FILENAME_MAX];
			if(!buffer)
				return script_memory_allocation_error;

			memset(buffer, 0, FILENAME_MAX);

			error = check_filename(cmd->args[1], buffer, FILENAME_MAX-1, QUEUE_COMMAND);

			if(error != script_no_errors)
				return error;

			this->queue_filename(value);

			ScriptParsing *sp = new ScriptParsing;

			if(!sp) {
				delete [] buffer;
				return script_subscript_exec_fail;
			}

			if(sp->CopyScriptParsingEnv(this)) {
				delete [] buffer;
				return script_subscript_exec_fail;
			}

			sp->file_type(QUEUE_COMMAND);

			error = sp->parse_commands(buffer);

			delete [] buffer;

#ifdef TESTING
			printf("Modem:%s\n", this->modem().c_str());
#endif // TESTING

			return error;
		}
	}

	return script_invalid_parameter;
}

/** **************************************************************
 * \brief Flag the queue loader to load from the tx directory
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"LOAD TXDIR:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_load_txdir(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->load_txdir(state);
	}

	return error;

}

/** **************************************************************
 * \brief Process and test modem parameters for validity.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par "Script Command:"<br>
 * <tt>\"MODEM:\<modem_id_string\>\" Example:\"MODEM:BPSK250\"</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_modem(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	std::string value = "";

	if(cmd->argc) {
		value.assign(cmd->args[0]);

		if(value.empty())
			return script_parameter_error;

		if(cmd->valid_values && cmd->valid_value_count) {
			int index = 0;
			int diff = 0;
			for(index = 0; index < cmd->valid_value_count; index++) {
				diff = strncmp(cmd->args[0], cmd->valid_values[index], MAX_PARAMETER_LENGTH);
				if(diff == 0) {
					this->modem(value);
					return script_no_errors;
				}
			}
			LOG_INFO("Non-matching/available modem ID string used (%s).", value.c_str());
			return script_invalid_parameter;
		}

		this->modem(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Base path for file access that follows.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par "Script Command:"<br>
 * <tt>\"PATH:\<directory_path\>\" Example:\"PATH:/usr/local\"</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_path(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	SCRIPT_CODES error = script_invalid_parameter;
	std::string value = "";

	if(cmd->argc) {
		value.assign(cmd->args[0]);

		if(value.empty())
			return script_parameter_error;

		error = check_path(value.c_str());

		if(error == script_no_errors) {
			this->path(value);
		}
	}

	return error;
}

/** **************************************************************
 * \brief Enable/Disable AMP2 protocol in the transmitted data.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"PROTO:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_proto(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->proto(state);
	}

	return error;
}
/** **************************************************************
 * \brief Enable/Disable unproto makers.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"UNPROTO MARKERS:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_unproto_makers(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->unproto_markers(state);
	}

	return error;
}


/** **************************************************************
 * \brief Event queue loading path (full path and file name)
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"QUEUE FILEPATH:\</path/filename.txt>\"</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_queue_filepath(struct script_cmds *cmd)
{
	if((cmd->argc > 0) && !cmd->args[0])
		return script_invalid_parameter;

	FILE *fd = fopen(cmd->args[0], "r");

	if(fd) {
		fclose(fd);
		queue_filepath(cmd->args[0]);
		return script_no_errors;
	}

	return script_file_not_found;
}

/** **************************************************************
 * \brief Reset program parameters
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"RESET:\<ALL|PARTIAL\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ALL     = Complete reset of all variables</tt><br>
 * <tt>PARTIAL = Parital reset of variables.</tt><br>
 * \par NOTE:
 * The resting of the data is handled by the callback function.
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_reset(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		int diff = 0;
		std::string value;

		value.assign(cmd->args[0]);
		if(value.empty())
			return script_invalid_parameter;

		to_uppercase(value);

		diff = strncmp(value.c_str(), "PARTIAL", MAX_PARAMETER_LENGTH);

		if(diff == 0) {
			this->reset(RESET_PARTIAL);
			return script_no_errors;
		}

		diff = strncmp(value.c_str(), "ALL", MAX_PARAMETER_LENGTH);

		if(diff == 0) {
			this->reset(RESET_ALL);
			return script_no_errors;
		}
	}

	return error;
}

/** **************************************************************
 * \brief Set the receive interval time (seconds)
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"RX INTERVAL:\<time_in_seconds\>\"</tt><br>
 * <tt>Value range 1-120</tt>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_rx_interval(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;

	if(cmd->argc && cmd->args[0]) {
		value = atoi(cmd->args[0]);

		if(value < 1) value = 1;
		if(value > 120) value = 120;

		this->rx_interval(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Modem sync method between FLAMP and FLDIGI
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"SYNC WITH:\<mode\>,\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>FLAMP  = FLDIGI Sync's with FLAMP</tt><br>
 * <tt>FLDIGI = FLAMP Sync's with FLDIGI</tt><br>
 * <tt>PRIOR  = Set FLDIGI to FLAMP's modem prior to transmitting data</tt>
 * <tt>ON     = Enable</tt>
 * <tt>OFF    = Disable</tt>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_sync_with(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	char *valid_values[] = {
		(char *) "FLAMP",
		(char *) "FLDIGI",
		(char *) "PRIOR"
	};

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		int diff = 0;
		std::string value;

		value.assign(cmd->args[0]);
		if(value.empty())
			return script_invalid_parameter;

		to_uppercase(value);

		for(size_t index = 0; index < sizeof(valid_values)/sizeof(char *); index++) {
			diff = strncmp(value.c_str(), valid_values[index], MAX_PARAMETER_LENGTH);

			if(diff == 0) {
				error = test_on_off_state(state, cmd->args[1]);

				if(error == script_no_errors) {
					switch(index) {
						case 0:
							this->sync_with_flamp(state);
							break;

						case 1:
							this->sync_with_fldigi(state);
							break;

						case 2:
							this->sync_with_prior(state);
							break;
					}
				}

				break;
			}
		}
	}

	return error;
}

/** **************************************************************
 * \brief Set the transmit interval in minutes
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"TX INTERVAL:\<time_in_minutes\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>Value range 1 to 8</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_tx_interval(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;

	if(cmd->argc && cmd->args[0]) {
		value = atoi(cmd->args[0]);

		if(value < 1) value = 1;
		if(value > 8) value = 8;

		this->tx_interval(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Enable/Diable transmit on report.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"TX REPORT:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_tx_report(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->tx_report(state);
	}

	return error;
}

/** **************************************************************
 * \brief
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"WARN USER:\<ON|OFF\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>ON  = Enable</tt><br>
 * <tt>OFF = Disable</tt><br>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_warn_user(struct script_cmds *cmd)
{
	SCRIPT_CODES error = script_invalid_parameter;
	bool state = false;

	if(!cmd)
		return script_function_parameter_error;

	if(cmd->argc && cmd->args[0]) {
		error = test_on_off_state(state, cmd->args[0]);

		if(error == script_no_errors)
			this->warn_user(state);
	}

	return error;
}

/** **************************************************************
 * \brief Set the number of times transmited data is repeated.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 * \par Script Command:<br>
 * <tt>\"XMIT REPEAT:\<count\>\"</tt><br>
 * \par Script Parameters:<br>
 * <tt>count = number of transmit repeats</tt><br>
 * <tt>Value range 1 - 100</tt>
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_xmit_repeat(struct script_cmds *cmd)
{
	if(!cmd)
		return script_function_parameter_error;

	int value = 0;

	if(cmd->argc && cmd->args[0]) {
		value = atoi(cmd->args[0]);

		if(value < 1) value = 1;
		if(value > 100) value = 100;

		this->xmit_repeat(value);
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Used for initialization of the function vector table.
 * \param cmd Pointer to matching struct script_cmds script
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
SCRIPT_CODES ScriptParsing::sc_dummy(struct script_cmds *cmd)
{
	return script_no_errors;
}

/** **************************************************************
 * \brief Convert error numbers into human readable form.
 * \param error_no Error number to convert.
 * \param line_number The offending line number in the script file.
 * \param cmd The script command is question.
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
char * ScriptParsing::script_error_string(SCRIPT_CODES error_no, int line_number, char *cmd)
{
	char *es = (char *) "";

	memset(error_buffer,     0, sizeof(error_buffer));
	memset(error_cmd_buffer, 0, sizeof(error_cmd_buffer));
	memset(error_string,     0, sizeof(error_string));

	if(cmd) {
		strncpy(error_cmd_buffer, cmd, sizeof(error_cmd_buffer)-1);
	}

	switch(error_no) {
		case script_command_not_found:
			es =  (char *) "Command Not Found";
			break;

		case script_non_script_file:
			es = (char *) "Not a script file/tag not found";
			break;

		case script_parameter_error:
			es = (char *) "Invalid parameter";
			break;

		case script_function_parameter_error:
			es = (char *) "Invalid function parameter (internal non-script error)";
			break;

		case script_mismatched_quotes:
			es = (char *) "Missing paired quotes (\")";
			break;

		case script_general_error:
			es = (char *) "General Error";
			break;

		case script_no_errors:
			es = (char *) "No Errors";
			break;

		case script_char_match_not_found:
			es = (char *) "Character searched not found";
			break;

		case script_end_of_line_reached:
			es = (char *) "End of line reached";
			break;

		case script_file_not_found:
			es = (char *) "File not found";
			break;

		case script_path_not_found:
			es = (char *) "Directory path not found";
			break;

		case script_args_eol:
			es = (char *) "Unexpected end of parameter (args[]) list found";
			break;

		case script_param_check_eol:
			es = (char *) "Unexpected end of parameter check list found";
			break;

		case script_paramter_exceeds_length:
			es = (char *) "Character count in args[] parameter exceeds expectations";
			break;

		case script_memory_allocation_error:
			es = (char *) "Memory Allocation Error (internal non-script error)";
			break;

		case script_incorrectly_assigned_value:
			es = (char *) "Passed parameter is not of the expected type.";
			break;

		case script_invalid_parameter:
			es = (char *) "Parameter is not valid.";
			break;

		case script_command_seperator_missing:
			es = (char *) "Command missing ':'.";
			break;

		case script_max_sub_script_reached:
			es = (char *) "Maximum open subscripts reached.";
			break;

		case script_subscript_exec_fail:
			es = (char *) "Subscript execution fail (internal).";
			break;

		default:
			es = (char *) "Undefined error";
	}

	snprintf(error_buffer, sizeof(error_buffer)-1, "Line: %d Error:%d %s (%s)",
			 line_number, error_no, es, error_cmd_buffer);

	return error_buffer;
}

/** **************************************************************
 * \brief Search for first occurrence of a non white space
 * \param data Data pointer to search.
 * \param limit Number of bytes in the data buffer.
 * \param error returned error code.
 * \return Pointer to character if found. Otherwise, return null
 * \par Note:<br>
 * The searched condition is ignored if the expected content is
 * encapsulated in quotes \(\"\"\).
 *****************************************************************/
char * ScriptParsing::skip_white_spaces(char * data, char * limit, SCRIPT_CODES &error)
{
	char *cPtr      = (char *) 0;

	if(!data || !limit) {
		error = script_function_parameter_error;
		return (char *)0;
	}

	for(cPtr = data; cPtr < limit; cPtr++) {
		if(*cPtr > ' ') {
			error = script_no_errors;
			return cPtr;
		}
	}

	error = script_end_of_line_reached;


	return (char *)0;        // End of line reached.
}

/** **************************************************************
 * \brief Search for the first occurrence on a non number.
 * \param data Data pointer to search.
 * \param limit Number of bytes in the data buffer.
 * \param error returned error code.
 * \return Pointer to character if found. Otherwise, return null
 * \par Note:<br>
 * The searched condition is ignored if the expected content is
 * encapsulated in quotes \(\"\"\).
 *****************************************************************/
char * ScriptParsing::skip_numbers(char * data, char * limit, SCRIPT_CODES &error)
{
	char *cPtr  = (char *) 0;
	int  q_flag = 0;

	if(!data || !limit) {
		error = script_function_parameter_error;
		return (char *)0;
	}

	for(cPtr = data; cPtr < limit; cPtr++) {
		if(*cPtr == '"')     // Check for encapsulated strings ("")
			q_flag++;

		if((q_flag & 0x1))   // Continue if string is encapsulated
			continue;

		if(!isdigit(*cPtr)) {
			error = script_no_errors;
			return cPtr;
		}
	}

	if(q_flag & 0x1) {
		error = script_mismatched_quotes;
	} else {
		error = script_end_of_line_reached;
	}

	return (char *)0;        // End of line reached.
}

/** **************************************************************
 * \brief Skip characters until either a number or white space is
 * found.
 * \param data Data pointer to search.
 * \param limit Number of bytes in the data buffer.
 * \param error returned error code.
 * \return Pointer to character if found. Otherwise, return null
 * \par Note:<br>
 * The searched condition is ignored if the expected content is
 * encapsulated in quotes \(\"\"\).
 *****************************************************************/
char * ScriptParsing::skip_characters(char * data, char * limit, SCRIPT_CODES &error)
{
	char *cPtr  = (char *) 0;
	int  q_flag = 0;

	if(!data || !limit) {
		error = script_function_parameter_error;
		return (char *)0;
	}

	for(cPtr = data; cPtr < limit; cPtr++) {
		if(*cPtr == '"')     // Check for encapsulated strings ("")
			q_flag++;

		if((q_flag & 0x1))   // Continue if string is encapsulated
			continue;

		if(isdigit(*cPtr) || *cPtr <= ' ') {
			error = script_no_errors;
			return cPtr;
		}
	}

	if(q_flag & 0x1) {
		error = script_mismatched_quotes;
	} else {
		error = script_end_of_line_reached;
	}

	return (char *)0;        // End of line reached.
}

/** **************************************************************
 * \brief Search for the first occurrence of a white space.
 * \param data Data pointer to search.
 * \param limit Number of bytes in the data buffer.
 * \param error returned error code.
 * \return Pointer to character if found. Otherwise, return null
 * \par Note:<br>
 * The searched condition is ignored if the expected content is
 * encapsulated in quotes \(\"\"\).
 *****************************************************************/
char * ScriptParsing::skip_alpha_numbers(char * data, char * limit, SCRIPT_CODES &error)
{
	char *cPtr  = (char *) 0;
	int  q_flag = 0;

	if(!data || !limit) {
		error = script_function_parameter_error;
		return (char *)0;
	}

	for(cPtr = data; cPtr < limit; cPtr++) {

		if(*cPtr == '"')     // Check for encapsulated strings ("")
			q_flag++;

		if((q_flag & 0x1))   // Continue if string is encapsulated
			continue;

		if(*cPtr <= ' ') {
			error = script_no_errors;
			return cPtr;
		}
	}

	if(q_flag & 0x1) {
		error = script_mismatched_quotes;
	} else {
		error = script_end_of_line_reached;
	}

	return (char *)0;        // End of line reached.
}

/** **************************************************************
 * \brief Search for first occurrence of 'character'
 * \param c Character to search for
 * \param data Pointer to Data to search for character in.
 * \param limit Number of bytes in the data buffer.
 * \param error returned error code.
 * \return Pointer to character if found. Otherwise, return null
 * \par Note:<br>
 * The searched condition is ignored if the expected content is
 * encapsulated in quotes \(\"\"\).
 *****************************************************************/
char * ScriptParsing::skip_to_character(char c, char * data, char * limit, SCRIPT_CODES &error)
{
	char *cPtr  = (char *) 0;
	int  q_flag = 0;

	if(!data || !limit) {
		error = script_function_parameter_error;
		return (char *)0;
	}

	for(cPtr = data; cPtr < limit; cPtr++) {
		if(*cPtr == '"')     // Check for encapsulated strings ("")
			q_flag++;

		if((q_flag & 0x1))   // Continue if string is encapsulated
			continue;

		if(*cPtr == c)   {    // Match found. Return pointer to it's location
			error = script_no_errors;
			return cPtr;
		}
	}

	if(q_flag & 0x1) {
		error = script_mismatched_quotes;
	} else {
		error = script_end_of_line_reached;
	}

	return (char *)0;        // End of line reached.
}

/** **************************************************************
 * \brief Replace CR, LF, and '#' with '0' (by value)
 * \param data Search data pointer
 * \param limit data buffer size
 * \return void (none)
 * \par Note:<br>
 * The searched condition is ignored if the remark character \(#\)
 * is encapsulated in quotes \(\"\"\).
 *****************************************************************/
SCRIPT_CODES ScriptParsing::remove_crlf_comments(char *data, char *limit, size_t &count)
{
	char *cPtr  = (char *) 0;
	int  q_flag = 0;

	SCRIPT_CODES error = script_no_errors;

	if(!data || !limit)
		return script_function_parameter_error;

	count = 0;

	for(cPtr = data; cPtr < limit; cPtr++) {
		if(*cPtr == '\r' || *cPtr == '\n') {
			*cPtr = 0;
			return script_no_errors;
		}

		if(*cPtr == '"')
			q_flag++;

		if((q_flag & 0x1))
			continue;

		if(*cPtr == '#') {
			*cPtr = 0;
			break;
		}

		if(*cPtr > ' ')
			count++;

	}

	// Remove trailing white spaces.
	while(cPtr >= data) {
		if(*cPtr <= ' ') *cPtr = 0;
		else break;
		cPtr--;
	}

	if(q_flag & 0x1) {
		error = script_mismatched_quotes;
	} else {
		error = script_end_of_line_reached;
	}

	return error;
}

/** **************************************************************
 * \brief Copy memory from address to address to the source buffer.
 * \param buffer Destination buffer
 * \param sPtr Start of the copy Address
 * \param ePtr End of the copy Address
 * \param limit Destination buffer size
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
SCRIPT_CODES ScriptParsing::copy_command(char *buffer, char *sPtr, char *ePtr, size_t limit)
{
	if(!buffer || !sPtr || !ePtr || limit < 1) {
		return script_function_parameter_error;
	}

	char *dPtr   = buffer;
	size_t index = 0;

	for(index = 0; index < limit; index++) {
		*dPtr++ = toupper(*sPtr++);
		if(sPtr >= ePtr) break;
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Remove leading/trailing white spaces and quotes.
 * \param buffer Destination buffer
 * \param limit passed buffer size
 * \return void
 *****************************************************************/
void ScriptParsing::trim(char *buffer, size_t limit)
{
	char *s      = (char *)0;
	char *e      = (char *)0;
	char *dst    = (char *)0;
	size_t count = 0;

	if(!buffer || limit < 1) {
		return;
	}

	for(count = 0; count < limit; count++)
		if(buffer[count] == 0) break;

	if(count < 1) return;

	s = buffer;
	e = &buffer[count-1];

	for(size_t i = 0; i < count; i++) {
		if((*s <= ' ') || (*s == '"')) s++;
		else break;
	}

	while(e > s) {
		if((*e <= ' ') || (*e == '"'))
			*e = 0;
		else
			break;
		e--;
	}

	dst = buffer;
	for(; s <= e; s++) {
		*dst++ = *s;
	}
	*dst = 0;
}

/** **************************************************************
 * \brief Parse the parameters and seperate into individual components.
 * \param s char pointer to the start of the string.
 * \param e char pointer to the end of the string.
 * \param matching_command pointer to the data strucure of the matching
 * command. See \ref SCRIPT_COMMANDS
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
SCRIPT_CODES ScriptParsing::parse_parameters(char *s, char *e, SCRIPT_COMMANDS *matching_command)
{
	char *c   = s;
	char *d   = (char *)0;
	int index = 0;
	int parameter_count = matching_command->argc;
	int count = 0;
	long tmp  = 0;

	SCRIPT_CODES error = script_no_errors;

	// Clear the old pointers.
	for(index = 0; index < MAX_CMD_PARAMETERS; index++) {
		matching_command->args[index] = (char *)0;
	}

	if(parameter_count > 0) {
		count = parameter_count - 1;
		for(index = 0; index < count; index++) {
			c = skip_white_spaces(c, e, error);

			if(error != script_no_errors)
				return script_parameter_error;

			d = skip_to_character(',', c, e, error);

			if(error != script_no_errors)
				return script_parameter_error;

			*d = 0;
			tmp = (long) (d - c);
			if(tmp > 0)
				trim(c, (size_t)(tmp));
			matching_command->args[index] = c;
			c = d + 1;
		}

		c = skip_white_spaces(c, e, error);
		if(error) return error;

		d = skip_alpha_numbers(c, e, error);
		if(error) return error;

		*d = 0;
		tmp = (long) (d - c);
		if(tmp > 0)
			trim(c, (size_t)(tmp));

		matching_command->args[index] = c;
	}

#ifdef TESTING
	for(int i = 0; i < parameter_count;  i++)
		if(matching_command->args[i])
			printf("parameters %d (%s)\n", i, matching_command->args[i]);
#endif

	error = check_parameters(matching_command);

	if(error != script_no_errors)
		return error;

	if(matching_command->func)
		error = (this->*matching_command->func)(matching_command);
	if(error) return error;


	return script_no_errors;
}

/** **************************************************************
 * \brief Execute callback function.
 * \param cb_data Pointer for making a copy of the data to prevent
 * exterior alteration of source information.
 * \return 0 = No error<br> \< 0 = Error<br>
 *****************************************************************/
int ScriptParsing::call_callback(SCRIPT_COMMANDS *cb_data)
{
	int argc     = 0;
	int error    = 0;
	int index    = 0;
	SCRIPT_COMMANDS *tmp = (SCRIPT_COMMANDS *)0;
	size_t count = 0;

	if(!cb_data || !cb_data->cb) return -1;

	argc = cb_data->argc;

	tmp = new SCRIPT_COMMANDS;

	if(!tmp) return -1;

	memset(tmp, 0, sizeof(SCRIPT_COMMANDS));

	for(index = 0; index < argc; index++) {
		if(cb_data->args[index]) {
			count = strnlen(cb_data->args[index], MAX_PARAMETER_LENGTH-1);
			tmp->args[index] = new char[count+1];
			if(tmp->args[index]) {
				memset(tmp->args[index], 0, count+1);
				strncpy(tmp->args[index], cb_data->args[index], count);
			} else {
				error = -1;
				break;
			}
		} else break;
	}

	if(error > -1) {
		// Fill SCRIPT_COMMANDS (tmp) struct with useful data.
		tmp->flags          = cb_data->flags;
		tmp->command_length = cb_data->command_length;
		tmp->argc           = cb_data->argc;
		strncpy(tmp->command, cb_data->command, MAX_COMMAND_LENGTH);

		// Initialize with do nothing functions
		tmp->func = &ScriptParsing::sc_dummy;
		tmp->cb   = callback_dummy;

		error = (*cb_data->cb)(this, tmp);
	}

	if(tmp) {
		for(index = 0; index < argc; index++) {
			if(tmp->args[index]) {
				delete [] tmp->args[index];
			}
		}

		delete tmp;
	}

	return error;
}

/** **************************************************************
 * \brief Parse a single line of data from the script file being read.
 * \param data Pointer the the script scring in question
 * \param buffer_size buffer size of the data pointer.
 * command.
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
SCRIPT_CODES ScriptParsing::parse_single_command(char *data, size_t buffer_size)
{
	char *buffer = (char *)0;
	char *cPtr   = (char *)0;
	char *endPtr = (char *)0;
	char *ePtr   = (char *)0;
	int allocated_buffer_size = 128;
	int callback_error = 0;
	size_t cmd_size    = 0;
	size_t cmp_results = 0;
	size_t index       = 0;
	size_t size        = 0;

	SCRIPT_CODES error = script_no_errors;

	cPtr = data;
	endPtr = &data[buffer_size];

	cPtr = skip_white_spaces(cPtr, endPtr, error);
	if(error != script_no_errors) return error;

	ePtr = skip_to_character(':', cPtr, endPtr, error);
	if(error != script_no_errors) return script_command_seperator_missing;

	buffer = new char [allocated_buffer_size];
	if(!buffer) {
		LOG_INFO("Buffer allocation Error near File: %s Line %d", __FILE__, __LINE__);
		return script_memory_allocation_error;
	}

	memset(buffer, 0, allocated_buffer_size);
	error = copy_command(buffer, cPtr, ePtr, allocated_buffer_size-1);
	if(error != script_no_errors) {
		buffer[0] = 0;
		delete [] buffer;
		return error;
	}

	int str_count = str_cnt(buffer, allocated_buffer_size);
	trim(buffer, str_count);

	for(index = 0; index < _script_command_table_count; index++) {
		size = strnlen(_script_command_table[index].command, MAX_COMMAND_LENGTH);
		cmd_size = strnlen(buffer, MAX_COMMAND_LENGTH);
		cmp_results = memcmp(buffer, _script_command_table[index].command, size);

		if(cmp_results == 0 && (cmd_size == size)) {
			if(file_type() & _script_command_table[index].flags) {
				error = parse_parameters(++ePtr, endPtr, &_script_command_table[index]);
				if(error)  {
					buffer[0] = 0;
					delete [] buffer;
					return error;
				}

				if(_script_command_table[index].cb) {
					callback_error = call_callback(&_script_command_table[index]);
					if(callback_error < 0)
						LOG_INFO("Call back for script command %s reported an Error", _script_command_table[index].command);
				}
			} else {
				LOG_INFO("Command %s ignored, not supported in current file type:", buffer);
			}
			break;
		}
	}

	buffer[0] = 0;
	delete [] buffer;

	return script_no_errors;
}

/** **************************************************************
 * \brief Script entry point for parsing the script file.
 * \param file_name_path path and file name for the script to parse.
 * \return SCRIPT_CODES error see \ref script_codes
 *****************************************************************/
SCRIPT_CODES ScriptParsing::parse_commands(char *file_name_path)
{
	bool log_error  = false;
	char *cPtr      = (char *)0;
	FILE *fd        = (FILE *)0;
	int line_number = 0;
	SCRIPT_CODES error_code = script_no_errors;
	size_t count    = 0;
	size_t tmp      = 0;

	if(!file_name_path) {
		LOG_INFO("Invalid function parameter 'char *file_name_path' (null)");
		return script_general_error;
	}

	fd = fopen(file_name_path, "r");

	if(!fd) {
		LOG_INFO("Unable to open file %s", file_name_path);
		return script_general_error;
	}

	memset(line_buffer, 0, sizeof(line_buffer));

	line_number++;
	char *retval = fgets(line_buffer, sizeof(line_buffer) - 1, fd);

	tmp = strlen(SCRIPT_FILE_TAG);
	line_buffer[tmp] = 0;
	tmp = strncmp(SCRIPT_FILE_TAG, line_buffer, tmp);

	if(!retval || tmp) {
		cPtr = script_error_string(script_non_script_file, line_number, line_buffer);
		LOG_INFO("%s", cPtr);
		fclose(fd);
		return script_non_script_file;
	}

	while(1) {
		if(ferror(fd) || feof(fd)) break;

		memset(line_buffer, 0, sizeof(line_buffer));
		if(fgets(line_buffer, sizeof(line_buffer) - 1, fd))
			line_number++;

#ifdef TESTING
		printf("Reading: %s", line_buffer);
#endif

		error_code = remove_crlf_comments(line_buffer, &line_buffer[sizeof(line_buffer)], count);

		if(count < 1) {
			continue;
		}

#ifdef TESTING
		printf("remove_crlf_comments(%s)\n", line_buffer);
#endif

		if(error_code >= script_no_errors)
			error_code = parse_single_command(line_buffer, sizeof(line_buffer) - 1);

		if(error_code != script_no_errors) {
			LOG_INFO("%s", script_error_string(error_code, line_number, line_buffer));
			log_error = true;
		}
	}

	fclose(fd);

	if(log_error) {
		return script_general_error;
	}

	return script_no_errors;
}

/** **************************************************************
 * \brief Destructors
 *****************************************************************/
ScriptParsing::~ScriptParsing()
{
	if(_script_command_table)
		delete [] _script_command_table;
}

/** **************************************************************
 * \brief Dummy callback function for initialization of
 * function pointers.
 * \param sp The calling ScriptParsing Class
 * \param sc Command data structure pointer to the matching script
 * command.
 * \return 0 = No error<br> \< 0 = Error<br>
 *****************************************************************/
int callback_dummy(ScriptParsing *sp, struct script_cmds *sc)
{
	return 0;
}

/** ********************************************************
 * \brief Determine the length of the string with a count
 * limitation.
 * \return signed integer. The number of characters in the
 * array not to excede count limit.
 ***********************************************************/
int ScriptParsing::str_cnt(char * str, int count_limit)
{
	if(!str || (count_limit < 1))
		return 0;

	int value = 0;

	for(int index = 0; index < count_limit; index++) {
		if(str[index] == 0) break;
		value++;
	}

	return value;
}




