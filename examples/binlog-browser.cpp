/*
  Copyright (c) 2013, Oracle and/or its affiliates. All rights
  reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; version 2 of
  the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA
*/

#include "binlog.h"
#include <getopt.h>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <cstring>
#include <regex.h>
#include <algorithm>
#define MAX_BINLOG_SIZE 1073741824
#define MAX_BINLOG_POSITION MAX_BINLOG_SIZE/4


/*
  @file binlog-browser

  This program code opens a binary log and prints out what events are found.
  It allows user to specify command line options including start and stop
  positions, offset from the beginning of the binlog file, as well as database
  names. With --database option, event type and positions for statements
  executed on that particular database will only be printed. Other options
  include --event_type, which will print out only that event which is
  specified by the user.
*/

using namespace std;
using binary_log::Binary_log;
using binary_log::system::create_transport;
using binary_log::system::Binary_log_driver;
using binary_log::system::Binlog_file_driver;
string opt_database;
string opt_event_type;
long int opt_start_pos= 4;
long int opt_offset= 0;
//The flag is set if user specifies --event_type option in the argument
int opt_event_flag= 0;
//The flag is set if user specifies --database option in the argument
int opt_db_flag= 0;
//Default value for opt_stop_pos is the maximum allowed binlog size value.
long int opt_stop_pos= MAX_BINLOG_POSITION;
//The flag is set if user specifes --include option in the argument
int opt_include_flag= 0;
//The flag is set if user specifes --exclude option in the argument
int opt_exclude_flag= 0;
//The flag is set if user specifes --format option in the argument
int opt_format_flag= 0;
//The flag is set if user specifes --format=line option in the argument
int opt_line_flag= 1;  //Default output format is line format
//The flag is set if user specifes --format=long option in the argument
int opt_para_flag= 0;
//This flag will force reading unknown binlog events.
int opt_force_read= 0;

/* map used for --format option */

typedef std::pair<enum_field_types, std::string> Row_Fields_pair;
typedef std::vector<Row_Fields_pair> Row_Fields_map;
typedef std::vector<Row_Fields_map> Row_map;

map<string, int> opt_format;
map<int, string> tid2tname;
map<int, string>::iterator tb_it;
map<int, binary_log::Table_map_event *> tid2tevent;
map<int, binary_log::Table_map_event *>::iterator tbe_it;


string event_types[]= { "Unknown",
                        "Start_v3",
                        "Query",
                        "Stop",
                        "Rotate",
                        "Intvar",
                        "Load",
                        "Slave",
                        "Create_file",
                        "Append_block",
                        "Exec_load",
                        "Delete_file",
                        "New_load",
                        "RAND",
                        "User var",
                        "Format_desc",
                        "Xid",
                        "Begin_load_query",
                        "Execute_load_query",
                        "Table_map",
                        "Write_rows_event_old",
                        "Update_rows_event_old",
                        "Delete_rows_event_old",
                        "Write_rows_v1",
                        "Update_rows_v1",
                        "Delete_rows_v1",
                        "Incident",
                        "Heartbeat",
                        "Ignorable",
                        "Rows_query",
                        "Write_rows",
                        "Update_rows",
                        "Delete_rows",
                        "Gtid",
                        "Anonymous_Gtid",
                        "Previous_gtids",
                        "User Defined"};

int number_of_events= sizeof(event_types) / sizeof(event_types[0]);

/*
  The following structure contains description for the command line
  options permitted by getopt_long.
*/
struct my_option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
  string arg;
  string description;
};

static struct my_option my_long_options[]=
{
  {"start-position", required_argument, 0, 'a', "INT",
   "Start reading the binlog at position N.\n"
   "N should be the staring position for an event in the binlog"},
  {"stop-position", required_argument, 0, 'b', "INT",
   "Stop reading the binlog at position N."},
  {"database", required_argument, 0, 'd',"DB_NAME",
   "Browse events for just this database."},
  {"offset", required_argument, 0, 'o', "INT",
   "Skip the first N entries"},
  {"event-type", required_argument, 0, 'e', "EVENT-TYPE",
   "Browse only one particular event"},
  {"include", required_argument, 0, 'i', "RegEx",
   "Include row events occuring on dbname.tbname"},
  {"exclude", required_argument, 0, 'x', "RegEx",
   "Exclude row events occuring on dbname.tbname"},
  {"format", required_argument, 0, 'F', "FIELD-LIST"
   "Select fields of the event, mentioned in the FIELD-LIST"},
  {"info-format", required_argument, 0, 'n', "ARG",
   "User specified event information format"},
  {"force", no_argument, 0, 'f', "BOOL",
   "Force reading unknown binlog events."},
  {0, 0, 0, 0}
};

/* long_options structure used by getopt_long */
static struct option
 long_options[sizeof(my_long_options) / sizeof(my_long_options[0])];

/**
  This function is called if the user provides an incorrect input on the
  command line.

  @param options  Structure containing description of available command line
                  options

  @return NULL
*/
static void usage(const struct my_option *options , const char *arg);

/**
  This function is called internally from check_event_db function.
  It compares the database names obatained from the user
  and that got from the event in the binlog file.

  @param log_dbname  It is the name got from the event in the binlog file.

  @return status
    @retval true  The event on the database log_dbname are to be skipped.
    @retval false The events are to be printed.
*/
bool shall_skip_database(const string& log_dbname)
{
  if (!log_dbname.empty() && !opt_database.empty())
    if (opt_database == log_dbname)
      return false;
  return true;
}

void print_acceptable_event_type()
{
  cout << "Allowed values for (--event_type=EVENT-TYPE)" << endl;
  for (int i= 0; i < number_of_events; i++)
    cout << event_types[i] << endl;
}

/**
  The function takes in takes in event name given as command line option, and
  compares it to the acceptable event types.

  @param optarg     The string given as command line option to --event_type

  @return status
    @retval true    Event type matches any of the acceptable event types
    @retval false   Event type specified on the command line does not match any
                    of the acceptable event types.
*/
bool check_opt_event_type(string optarg)
{
  return binary_search(event_types, event_types + number_of_events, optarg);
}

bool check_event_type(Binary_log_event **event)
{
  string log_event_type= event_types[(*event)->get_event_type()];

  if (!opt_event_type.empty() && !log_event_type.empty())
    if (opt_event_type == log_event_type)
      return false;
  return true;
}


/**
  The function takes in events from the binlog file, gets
  the database on which the event occurred, and returns status
  whether the database is same as that  given by the user.

  @param  event     events read from the binlog file via get_next_event and
                    decode_event in the main function

  @return Status
    @retval true    Database name does not match the name given by the user and
                    the event must  be skipped
    @retval false   The database for the event is the same as that specified
                    by the user.
*/

static bool check_event_db(Binary_log_event **event)
{
  switch ((*event)->get_event_type())
  {
  case QUERY_EVENT:
    {
      Query_event *query_event= dynamic_cast<Query_event*>(*event);
      if (query_event == 0)
        return false;

      return shall_skip_database(query_event->db);
    }
  case TABLE_MAP_EVENT:
    {
      Table_map_event *table_map_event= dynamic_cast<Table_map_event*>(*event);
      if (table_map_event == 0)
        return false;

      if (shall_skip_database(table_map_event->m_dbnam))
      {

        tid2tname[table_map_event->get_table_id()]= table_map_event->m_dbnam;

        /*
          table_id is inserted in the map in case the event must not be skipped.
          It is used while checking databases for the row events.
        */
        return true;
      }
      return false;
    }
  case PRE_GA_WRITE_ROWS_EVENT:
  case PRE_GA_UPDATE_ROWS_EVENT:
  case PRE_GA_DELETE_ROWS_EVENT:
  case WRITE_ROWS_EVENT:
  case WRITE_ROWS_EVENT_V1:
  case UPDATE_ROWS_EVENT:
  case UPDATE_ROWS_EVENT_V1:
  case DELETE_ROWS_EVENT:
  case DELETE_ROWS_EVENT_V1:
    {
      Rows_event *row_event= dynamic_cast<Rows_event*>(*event);
      if (row_event == 0)
        return false;

      tb_it= tid2tname.find(row_event->get_table_id());
      if (tb_it != tid2tname.end())
      {
        if (row_event->get_flags() == Rows_event::STMT_END_F)
          tid2tname.erase(tb_it);
        return true;
      }
      return false;
    }
  default:
    return false;
  //TODO Add cases for CREATE_FILE_EVENT and EXECUTE_LOAD_QUERY_EVENT.
  /*
     Currently, the API has no classes for these events,
     and they go into the default case. Classes are to be added to the
     repository. Reference file: binlog_driver.h.
  */
  }//end switch
}

bool check_conversion(const char* optarg, long int *position)
{
  char *endp;

  errno= 0;
  *position= strtol(optarg, &endp, 10);
  if ((errno == ERANGE &&
     (*position == LONG_MAX || *position == LONG_MIN)) ||
     (errno == EINVAL) ||
     (errno != 0 && *position == 0) || *endp != '\0' )
  {
    return false;
  }
  else
  {
    return true;
  }
}

/*
  regular expressions defining the fully qualified
  table names with database names.
  (POSIX regular expressions)
*/
static regex_t include_regexp;
static regex_t exclude_regexp;

/* pattern string in the argument vector for regex function regcomp */
string include_pattern, exclude_pattern;

/**
  Function to match the fully qualified table name to the
  regex provided by the user

  @param database_dot_table  Fully qualified table name (db_name.tb_name)
  @param flag                Indicates whether the event is to be matched for
                             inclusion or exclusion
                             flag= 'i' => include
                             flag= 'e' => exclude

  @return Status
    @retval true             If the match regexp matched
    @retval false            If the match regexp is not matched
*/
bool filter_table_by_regex(string database_dot_table, char flag)
{
  int rcode;
  if (flag == 'i')
    rcode= regexec(&include_regexp, database_dot_table.c_str(), 0, NULL, 0);
  if (flag == 'e')
    rcode= regexec(&exclude_regexp, database_dot_table.c_str(), 0, NULL, 0);

  switch (rcode)
  {
  case 0:
    /* regexp matched */
    return true;
  case REG_NOMATCH:
    /* regexp did not match */
    return false;
  default:
    {
      /* regexp error */
      char errbuf[256];
      if (opt_include_flag)
        regerror(rcode, &include_regexp, errbuf, sizeof(errbuf));
      if (opt_exclude_flag)
        regerror(rcode, &exclude_regexp, errbuf, sizeof(errbuf));
      if (opt_include_flag)
        cerr << "Error: --include option: regexp: " << errbuf << endl;
      else
        cerr << "Error: --exclude option: regexp: " << errbuf << endl;
      exit(1);
    }
  }
}

/*  The setup_regexp function compiles the regular expression
    for include_option.

   @param pattern  A null-terminated string containing a POSIX style
                   regular expression

   @paran flag     Indicates whether the event is to be matched for
                             inclusion or exclusion
                             flag= 'i' => include
                             flag= 'e' => exclude

    @return Status
      @retval 0 if call succeeds
      @retval a non-zero return code otherwise
*/
int setup_regexp(const string& pattern, char flag)
{
  int rcode;
  if (flag == 'i')
  {
    include_pattern= pattern;
    rcode= regcomp(&include_regexp,
                   include_pattern.c_str(),
                   REG_EXTENDED | REG_NOSUB);
  }
  else
  {
    exclude_pattern= pattern;
    rcode= regcomp(&exclude_regexp,
                   exclude_pattern.c_str(),
                   REG_EXTENDED | REG_NOSUB);
  }

  if (rcode != 0)
  {
    /* regexp error */
    char errbuf[256];
    switch (flag)
    {
    case 'i':
      regerror(rcode, &include_regexp, errbuf, sizeof(errbuf));
      cout << "Error: --include option: regexp: " << errbuf << endl;
      break;
    case 'x':
      regerror(rcode, &exclude_regexp, errbuf, sizeof(errbuf));
      cout << "Error: --exclude option: regexp: " << errbuf << endl;
      break;
    default:
      cout << "Wrong option specified " << endl;
    }
  }
  return rcode;
}


static void parse_args(int *argc, char **argv)
{

/* Create an array of structure 'option' for the sake of getopt_long */
  for (int i= 0;
       i < (sizeof(my_long_options) / sizeof(my_long_options[0])); i++)
  {
    long_options[i].name= my_long_options[i].name;
    long_options[i].has_arg= my_long_options[i].has_arg;
    long_options[i].flag= my_long_options[i].flag;
    long_options[i].val= my_long_options[i].val;
  }

  /* key is the field of the events and value is the flag */
  opt_format["start_pos"]= 1;
  opt_format["end_pos"]= 2;
  opt_format["length"]= 3;
  opt_format["type"]= 4;
  opt_format["info"]= 5;

  while (true)
  {
    //getopt_long stores the option index here
    int option_index= 0;
    int c= getopt_long(*argc, argv,
                       "a:b:d:o:e:i:x:fn:F:",
                       long_options, &option_index);
    //Detect the end of options
    if (c == -1)
      break;
    switch (c)
    {
    case 'a':
      if (!check_conversion(optarg, &opt_start_pos))
      {
        cerr << optarg << "is not an integer" << endl;
        usage(my_long_options, *argv);
        exit(2);
      }
      if (opt_start_pos < 4)
      {
        cerr << "option 'start-position': value " << opt_start_pos
             << " adjusted to 4"
             << endl;
        opt_start_pos= 4;
      }
      break;
    case 'b':
      if (!check_conversion(optarg, &opt_stop_pos))
      {
        cerr << optarg << "is not an integer" << endl;
        usage(my_long_options, *argv);
        exit(2);
      }
      break;
    case 'd':
      {
        opt_database= optarg;
        opt_db_flag= 1;
        break;
      }
    case 'o':
      if (!check_conversion(optarg, &opt_offset) )
      {
        cerr << optarg << "is not an integer" << endl;
        usage(my_long_options, *argv);
        exit(2);
      }
      if (opt_offset < 0)
      {
        cerr << "option 'offset': value " << opt_offset
             << "adjusted to 0"
             << endl;
        opt_offset= 0;
      }
      break;
    case 'e':
      if (!check_opt_event_type(optarg))
      {
        usage(my_long_options, *argv);
        exit(2);
      }
      opt_event_type= optarg;
      opt_event_flag= 1;
      break;
    case 'x':
      opt_exclude_flag= 1;
      if (setup_regexp(optarg, 'x') != 0)
      {
        usage(my_long_options, *argv);
        exit(2);
      }
      break;
    case 'i':
      opt_include_flag= 1;
      if (setup_regexp(optarg, 'i') != 0)
      {
        usage(my_long_options, *argv);
        exit(2);
      }
      break;
    case 'F':
      {
        opt_format_flag= 1;
        stringstream stream(optarg);
        string word;
        map<string, int>::iterator it= opt_format.begin();
        while (getline(stream, word, ','))
        {
          it= opt_format.find(word);
          if (it == opt_format.end())
          {
            cerr << "Incorrect argument for option --format" << endl;
            usage(my_long_options, *argv);
            exit(2);
          }
          it->second= it->second + 10;
        }
        break;
      }
    case 'n':
      {
        if (strcmp(optarg, "long") == 0)
        {
          opt_para_flag= 1;
          opt_line_flag= 0;
        }
        else if (strcmp(optarg, "line") == 0)
          opt_line_flag= 1;
        else
        {
          cerr << "Incorrect argument for option --info-format" << endl;
          usage(my_long_options, *argv);
          exit(2);
        }
        break;
      }
    case 'f':
     {
       opt_force_read= 1;
       break;
     }
    case '?':
    default:
      //getopt has already printed out the error
      usage(my_long_options, *argv);
      exit(2);
    }//end of switch
  }//end of while loop

  if (optind == *argc)
  {
    usage(my_long_options, *argv);
    exit(2);
  }
  else
    *argc= optind;
}//endparse_args

static void usage(const struct my_option *options, const char *argv)
{
  const struct my_option *optp;
  const char *base_name, *ptr;
  ptr= strrchr(argv, '/');
  base_name= (ptr == NULL? argv: ptr + 1);
  cerr << "Usage: " << base_name << " [options]  (mysql_uri|log_files)\n\n"
       << "Example:\n\n"
       << base_name << " [options] mysql://root@127.0.0.1:13000\n"
       << base_name << " [options] ../data/binary_log.000001\n\n";

  for (optp= options; optp->name; optp++)
  {
    cout << "-" << setw(1) << (char)optp->val
         << (strlen(optp->name) ? ", " : "  ");
    std::string info;
    info.append("--");
    info.append(optp->name);
    if (optp->has_arg  == 1)
    {
       info.append("=");
       info.append(optp->arg);
       cerr << setw(30) << left << info;
    }
    else
       cerr << setw(30) << left << info;
    cerr << setw(60) << left << optp->description << endl;
  }
  cerr << "Allowed values for (--info-format=ARG)\n"
       << "line\nlong\n";
  cerr << "Allowed values for (--format=FIELD-LIST)\n";
  map<string, int>::iterator it= opt_format.begin();
  for (it= opt_format.begin(); it != opt_format.end(); ++it)
    cerr << it->first << "\n";

  print_acceptable_event_type();
}

int main(int argc, char** argv)
{
  cout << "Important: this is an example program to show the use of the"
       << " library. It should not be used in production."
       << endl;
  string uri;
  int number_of_args= argc;
  char cwd[PATH_MAX];

  getcwd(cwd, PATH_MAX);
  parse_args(&argc, argv);

  while (argc != number_of_args)
  {
    uri= argv[argc++];
    if ( strncmp("mysql://", uri.c_str(), 8) != 0)
    {
      uri.insert(0, "file://");
      if (uri[7] == '.')
      {
        uri.insert(7, cwd);
        uri.insert((7 + strlen(cwd)), "/");
      }
      cout << uri << endl;
    }

    Decoder decode(opt_force_read);
    Binary_log_driver *drv= create_transport(uri.c_str());
    Binary_log binlog(drv);
    int error_number= binlog.connect();

    if (const char* msg= str_error(error_number))
        cerr << msg << endl;

    if (error_number != ERR_OK)
    {
      cerr << "Unable to setup connection" << endl;
      delete drv;
      return 1;
    }

    if (binlog.set_position("mysql-bin.000001", opt_start_pos) != ERR_OK)
    {
      cerr << "The specified position "
           << opt_start_pos
           << " cannot be set"
           << endl;
      return 1;
    }

    if (opt_format_flag)
    {
      map<string, int>::iterator it;
      for (it= opt_format.begin(); it != opt_format.end(); ++it)
      {
        if (it->second > 10 && it->second != 15)
        {
          cout << setw(17) << left
               << it->first;
        }
      }
      cout << endl;
    }
    else
    {
      cout << setw(17) << left
           << "Start Position"
           << setw(15) << left
           << "End Position"
           << setw(15) << left
           << "Event Length"
           << setw(25) << left
           << "Event Type"
           << endl;
    }
    // when the transport is file we add "file://" at the beginning.
    string transport_name= uri.substr(0, uri.find(':'));
    Row_Fields_map row_fields_val;
    Row_map rows_val;
    while (true)
    {
      Binary_log_event *event;
      long int event_start_pos;
      string event_type;
      string database_dot_table;
      std::pair<unsigned char *, size_t> buffer_buflen;
      error_number= drv->get_next_event(&buffer_buflen);
      if (error_number == ERR_OK)
      {
        const char *error= NULL;
        if (!(event= decode.decode_event((char*)buffer_buflen.first, buffer_buflen.second, &error, 1)))
        {
          cerr << error << endl;
          break;
        }
        //TODO: Consider modifying this

        if (transport_name == "file")
          if ((error_number= static_cast<Binlog_file_driver*>(drv)->update_pos(event))
                             != ERR_OK)
          {
            const char* msg=  str_error(error_number);
            std::cerr << msg << std::endl;
            break;
          }
      }
      else
      {
        const char* msg=  str_error(error_number);
        cerr << msg << endl;
        break;
      }
      //std::cout << "event :" << event->get_event_type() << ", position :" << binlog.get_position() << std::endl;

      if (event->get_event_type() == binary_log::INCIDENT_EVENT ||
         (event->get_event_type() == binary_log::ROTATE_EVENT &&
          event->header()->log_pos == 0 ||
          event->header()->log_pos == 0))
      {
          /*
            If the event type is an Incident event or a Rotate event
            which occurs when a server starts on a fresh binlog file,
            the event will not be written in the binlog file.
          */
          event_start_pos= 0;

      }
      else
      {
        event_start_pos= (event->header()->log_pos) -
                       (event->header())->data_written;
      }

      if (event_start_pos >= opt_stop_pos)
        break;

      /*
      if ((opt_offset > 0 && opt_offset--) ||
          (opt_db_flag && check_event_db(&event)) ||
          (opt_event_flag && check_event_type(&event)))
        continue;
      */

      if (event->get_event_type() == binary_log::TABLE_MAP_EVENT ||
          event->get_event_type() == binary_log::PRE_GA_WRITE_ROWS_EVENT ||
          event->get_event_type() == binary_log::PRE_GA_UPDATE_ROWS_EVENT ||
          event->get_event_type() == binary_log::PRE_GA_DELETE_ROWS_EVENT ||
          event->get_event_type() == binary_log::WRITE_ROWS_EVENT ||
          event->get_event_type() == binary_log::WRITE_ROWS_EVENT_V1 ||
          event->get_event_type() == binary_log::UPDATE_ROWS_EVENT ||
          event->get_event_type() == binary_log::UPDATE_ROWS_EVENT_V1 ||
          event->get_event_type() == binary_log::DELETE_ROWS_EVENT ||
          event->get_event_type() == binary_log::DELETE_ROWS_EVENT_V1)
      {
         if (event->get_event_type() == binary_log::TABLE_MAP_EVENT)
         {
           Table_map_event *table_map_event= static_cast<Table_map_event*>(event);
           database_dot_table= table_map_event->m_dbnam;
           database_dot_table.append(".");
           database_dot_table.append(table_map_event->m_tblnam);
           tid2tname[table_map_event->get_table_id()]= database_dot_table;
           tid2tevent[table_map_event->get_table_id()] = table_map_event;
        }
        else
        {
          //std::cout << "write row" << std::endl;
          rows_val.clear();
          // It is a row event
          Rows_event *row_event= static_cast<Rows_event*>(event);
          tb_it= tid2tname.begin();
          tb_it= tid2tname.find(row_event->get_table_id());
          if (tb_it != tid2tname.end())
          {
            tbe_it = tid2tevent.find(row_event->get_table_id());
            database_dot_table= tb_it->second;

            binary_log::Row_event_set rows(row_event, tbe_it->second);
            binary_log::Row_event_set::iterator row_it = rows.begin();
            std::cout << tbe_it->second->m_colcnt << std::endl;
            do
            {
                row_fields_val.clear();
                binary_log::Row_of_fields fields = *row_it;
                binary_log::Row_of_fields::iterator field_it = fields.begin();
                binary_log::Converter converter;
                if (field_it != fields.end())
                    do{
                        std::string str;
                        converter.to(str, *field_it);
                        Row_Fields_pair field;
                        field.first = (*field_it).type();
                        field.second = str;
                        row_fields_val.push_back(field);
                        if (field.first != MYSQL_TYPE_TIMESTAMP2) {
                          std::cout << field.first << "]--[" << field.second << ":" << (*field_it).is_bit_set() << std::endl;
                        } else {
                          long timestamp = 0;
                          const unsigned char* bytes = (*field_it).storage();
                          timestamp = ((long) (0xff & bytes[0]) << 24) | ((long) (0xff & bytes[1]) << 16) | ((long) (0xff & bytes[2]) << 8) | ((long) (0xff & bytes[3]));
                          std::cout << field.first << "]--[" << timestamp << ":" << (*field_it).is_bit_set() << std::endl;
                        }
                    } while (++field_it != fields.end());

                rows_val.push_back(row_fields_val);
                std::cout << "--test--" << std::endl;
            } while (++row_it != rows.end());

            if (row_event->get_flags() == Rows_event::STMT_END_F) {
              tid2tname.erase(tb_it);
              delete tbe_it->second;
              tid2tevent.erase(tbe_it);
            } 

          }

        }
      }


      if (opt_format_flag)
      {
        int info_flag= 0;
        map<string, int>::iterator it= opt_format.begin();
        for (it= opt_format.begin(); it != opt_format.end(); ++it)
        {
          cout << setw(17) << left;
          if (it->second == 11)
            cout << event_start_pos;
          if (it->second == 12)
            cout << (event->header())->log_pos;
          if (it->second == 13)
            cout << (event->header())->data_written;
          if (it->second == 14)
            cout << event_types[(event->get_event_type())]
                 << "[" << event->get_event_type()
                 << "]";
          if (it->second == 15)
            info_flag= 1;
        }
        cout << endl;
        if (info_flag)
        {
           cout << "Event info: ";
           opt_para_flag ?
             event->print_long_info(cout) : event->print_event_info(cout);
           cout << endl;
        }
      }
      else
      {
        cout << setw(17) << left
             << event_start_pos
             << setw(15) << left
             << (event->header())->log_pos
             << setw(15) << left
             << (event->header())->data_written
             << event_types[(event->get_event_type())]
             << "[" << event->get_event_type()
             << "]"
             << endl;
        cout << "Event Info: ";
        opt_para_flag ?
          event->print_long_info(cout) : event->print_event_info(cout);
        cout << endl;
      }
      if (event->get_event_type() != binary_log::TABLE_MAP_EVENT) {
          delete event;
      }
      event= NULL;
    }
    delete drv;
  }//end of outer while loop
}
