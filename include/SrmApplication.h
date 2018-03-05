#ifndef SRMAPPLICATION_H
#define SRMAPPLICATION_H

#include <pthread.h>
#include <queue>
#include "AppConfig.h"
#include "EventLoop.h"

class SrmApplication
{
private:
  SrmApplication();
  SrmApplication(const SrmApplication&);
  void operator=(const SrmApplication&);
  ~SrmApplication();

  enum DIRECTION {
    D_IN	=	0,
    D_OUT =	1
  };

  typedef void (*lpf_proc_result)(int32_t, int32_t*, size_t);
  static const int ALIVE_ANSWER[2] ;
  static const int FINISH_ANSWER[2];
  static const int END_MESSAGE;

  static char m_eolbyte;
  static int m_fd_uio;
  static int m_file_chan;
  static int m_table_chan;
  static volatile bool m_running_flag;
  static int m_command_socket; //command socket
  static int m_data_socket; //data socket
  static CEventLoop *m_event_loop;
  static size_t m_files_count;
  static int8_t* m_bufferOut;
  static int8_t* m_bufferIn;
  static const uint32_t m_page_size;

  static int8_t* m_table_chan_buff;
  static int8_t* m_file_chan_buff;

  static void dev_access(int fd, int offset, DIRECTION direction, int value, int &result);
  static void dma_load_table(register int8_t* bufferIn, uint32_t buffLen);
  static void dma_load_input_file(int fd_in, int32_t file_id, lpf_proc_result lpf_process_result);

  static void set_quiet_flag_to_fpga(void);

  static void set_match_count_to_zero();
  static int  open_dev_files();
  static int  reload_dfa_table(const std::string& re_pattern);

  static void send_data_file_to_fpga(const char *input_file, int32_t file_id, lpf_proc_result lpf_process_result);
  static void process_input_data_net(std::string buffer, lpf_proc_result lpf_process_result);
  static void process_result_data_net(int32_t file_id, int32_t* dw_buff_result, size_t len);
  static void process_result_data_net_internal(int32_t file_id, int32_t* dw_buff_result, size_t len);

  static void process_result_data_app(int32_t file_id, int32_t* dw_buff_result, size_t len);

  static void ctrl_c_handler(int s);
  static bool register_signal_handlers();

  static int init_sockets();
  static void listen_to_master();

  static void print_matchs_count();
  //event loop handlers
  static void el_on_handle_exception(std::exception& elExceptionInfo);
  static void el_on_handle_method_timeout(const char* methodName);
  static void el_on_log(const char* logMessage);
  static void close_opened_files();

public:
  static int run(const char *input_file);
};

#endif // SRMAPPLICATION_H
