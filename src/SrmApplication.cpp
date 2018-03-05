#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "Commons.h"
#include "SrmApplication.h"
#include "DfaCell.h"
#include "FunctorWithoutResult.h"
#include "InternalCriticalSection.h"
#include "Locker.h"

#ifdef TIME_MEASUREMENTS
#include "IFunctor.h"
#include "TimeMeasurements.h"
#include "FunctorWithoutResult.h"
#include "FunctorWithResult.h"
#endif

static const int RESULT_OFFSET    = 0;			// Result register's offset
static const int RESET            = 1;
static const int DEV_QUIET_FLAG   = 2;      //add quiet flag by dev_access function
static const int RST_OFFSET       = 4;			// Result register's offset

static const uint32_t FIFO_LEN      = 4000;   // Max buffer size for DMA, 4KB
static const int TX_MAGNITUDE     = sizeof(int32_t);

static const char* UIO_DEV    = "/dev/uio0";
static const char* DMA_DEV    = "/dev/axi_dma_0";
static const char* DMA_TABLE  =	"/dev/axi_dma_1";

static const int DMA_BUFFER_LEN   = 1024*16; //get it from driver side.

const int SrmApplication::ALIVE_ANSWER[]      = {-2, -1};
const int SrmApplication::FINISH_ANSWER[]     = {-3, -1};
const int SrmApplication::END_MESSAGE         = -1;

// Option variables
int SrmApplication::m_fd_uio                  = -1;
int SrmApplication::m_file_chan               = -1;
int SrmApplication::m_table_chan              = -1;

volatile bool SrmApplication::m_running_flag  = true;
int SrmApplication::m_command_socket          = -1;
int SrmApplication::m_data_socket             = -1;
CEventLoop* SrmApplication::m_event_loop;
size_t SrmApplication::m_files_count          = -1;

const uint32_t SrmApplication::m_page_size    = sysconf(_SC_PAGESIZE);

int8_t* SrmApplication::m_file_chan_buff      = NULL;
int8_t* SrmApplication::m_table_chan_buff     = NULL;
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void SrmApplication::dev_access(int fd, int offset, DIRECTION direction, int value, int& result)
{
  int8_t *ptr;
  ptr = (int8_t*)mmap(NULL, m_page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  switch (direction) {
    case D_IN:
      result = *(uint32_t*)(ptr+offset);
      break;
    case D_OUT:
      *((uint32_t*)(ptr+offset)) = value;
      break;
  }
  munmap(ptr, m_page_size);
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::print_matchs_count()
{
  int result = -1;
  if (m_fd_uio != -1) {
    dev_access(m_fd_uio, RESULT_OFFSET, D_IN, RESET, result);
    printf("Matches: %d\n", result);
#ifdef TIME_MEASUREMENTS

#endif
  }
}
////////////////////////////////////////////////////////////////////////////

int SrmApplication::reload_dfa_table(const std::string &re_pattern)
{
  int res;
  CF_NFA::CDfaTable* dfa ;
#ifdef TIME_MEASUREMENTS
  IFunctor* func = new FunctorWithResult<CF_NFA::CDfaTable*, const char*, int&>(
        CF_NFA::CDfaTable::dfa_table_from_regexp, re_pattern.c_str(), res, "dfa_table_from_regexp");
  CTimeMeasurements::Instance().one_shot_measure(MT_construct_dfa_table, func);
  dfa = *((CF_NFA::CDfaTable**)func->RawResult());
  delete func;
#else
  dfa = CF_NFA::CDfaTable::dfa_table_from_regexp(re_pattern.c_str(), res);
#endif
  if (res != 0) {
    printf("Error while constructing dfa: %s\n", re_pattern.c_str());
    return -1;
  }

  uint32_t t_size = 0;
  int8_t* tableIn = dfa->serialize(t_size);
#ifdef TIME_MEASUREMENTS
  func = new FunctorWithoutResult<int8_t*, uint32_t>(dma_load_table, tableIn, t_size, "dma_load_table");
  CTimeMeasurements::Instance().one_shot_measure(MT_srm_dma_load_table, func);
  delete func;
#else
  dma_load_table(tableIn, t_size);
#endif
  delete dfa;
  delete[] tableIn;
  return 0;
}
////////////////////////////////////////////////////////////////////////////

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

void SrmApplication::dma_load_table(register int8_t *bufferIn, uint32_t buffLen)
{
  register uint32_t byteToMove;
  while(buffLen > 0){
    byteToMove = std::min(buffLen, FIFO_LEN);
    memcpy(m_table_chan_buff, bufferIn, byteToMove);
    write(m_table_chan, NULL, byteToMove); //write uses m_table_chan_buff in driver
    bufferIn += byteToMove;
    buffLen -= byteToMove;
  }
}

void SrmApplication::dma_load_input_file(int fd_in,
                                         int32_t file_id,
                                         SrmApplication::lpf_proc_result lpf_process_result)
{
#ifdef TIME_MEASUREMENTS
  int fd_in_size = 0;
  IFunctor *s_func = new FunctorWithResult<long, int, __off_t, int>(
        lseek, fd_in, 0, SEEK_END, "lseek");
  CTimeMeasurements::Instance().average_measure(file_id, AMT_file_seek, s_func);
  long f_size = (*(long*)s_func->RawResult());
  delete s_func;
  s_func = new FunctorWithResult<long, int, __off_t, int>(
        lseek, fd_in, 0, SEEK_SET, "lseek");
  CTimeMeasurements::Instance().average_measure(file_id, AMT_file_seek, s_func);
  delete s_func;
  long ch_count = f_size / FIFO_LEN;

  if (CAppConfig::Instance()->quiet()) {
    set_quiet_flag_to_fpga();
    IFunctor* rf_func = new FunctorWithResult<ssize_t, int, void*, size_t>(
          read, fd_in, m_file_chan_buff, FIFO_LEN, "read");
    IFunctor* wd_func = new FunctorWithResult<ssize_t, int, const void*, size_t>(
          write, m_file_chan, NULL, FIFO_LEN, "write dma");
    while (ch_count > 0) {
      CTimeMeasurements::Instance().average_measure(file_id, AMT_file_read, rf_func);
      CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_write, wd_func);
      --ch_count;
    }
    CTimeMeasurements::Instance().average_measure(file_id, AMT_file_read, rf_func);
    fd_in_size = (int)(*(ssize_t*)rf_func->RawResult());
    delete rf_func;
    delete wd_func;
    m_file_chan_buff[fd_in_size] = 0;
    wd_func = new FunctorWithResult<ssize_t, int, const void*, size_t>(
          write, m_file_chan, NULL, fd_in_size+1, "write dma");
    CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_write, wd_func);
    delete wd_func;
  } //if quiet
  else {
    int32_t* dw_buff_out = (int32_t*)m_file_chan_buff;
    IFunctor* rf_func = new FunctorWithResult<ssize_t, int, void*, size_t>(
          read, fd_in, m_file_chan_buff, FIFO_LEN, "read file");
    IFunctor* wd_func = new FunctorWithResult<ssize_t, int, const void*, size_t>(
          write, m_file_chan, NULL, FIFO_LEN, "write dma");
    IFunctor* rd_func = new FunctorWithResult<ssize_t, int, void*, size_t>(
          read, m_file_chan, NULL, FIFO_LEN*TX_MAGNITUDE, "read dma");


    while (ch_count > 0) {
      CTimeMeasurements::Instance().average_measure(file_id, AMT_file_read, rf_func);
      CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_write, wd_func);
      CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_read, rd_func);
      size_t to_fwrite = 0;
      for (to_fwrite = 0; dw_buff_out[to_fwrite+1] &&
           to_fwrite<=(size_t)FIFO_LEN ;++to_fwrite) {;}
      lpf_process_result(file_id, dw_buff_out, to_fwrite == 0 ? 0 : to_fwrite+1);
      --ch_count;
    }
    CTimeMeasurements::Instance().average_measure(file_id, AMT_file_read, rf_func);
    fd_in_size = *((int*)rf_func->RawResult());
    m_file_chan_buff[fd_in_size] = 0;
    delete wd_func;
    wd_func = new FunctorWithResult<ssize_t, int, const void*, size_t>(
          write, m_file_chan, NULL, fd_in_size+1, "write dma");
    CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_write, wd_func);
    delete wd_func;
    CTimeMeasurements::Instance().average_measure(file_id, AMT_dma_read, rd_func);
    delete rd_func;

    size_t to_fwrite = 0;
    for (to_fwrite = 0; dw_buff_out[to_fwrite+1] &&
         to_fwrite<=(size_t)fd_in_size;++to_fwrite) {;}
    lpf_process_result(file_id, dw_buff_out, to_fwrite == 0 ? 0 : to_fwrite+1);
  } //else (if (quiet))
#else //TIME_MEASUREMENTS
  int fd_in_size = 0;
  long f_size = lseek(fd_in, 0, SEEK_END);
  lseek(fd_in, 0, SEEK_SET); //return to begining of the file
  long ch_count = f_size / FIFO_LEN;

  if (CAppConfig::Instance()->quiet()) {
    set_quiet_flag_to_fpga();
    while (ch_count > 0) {
      /*fd_in_size = */
      read(fd_in, m_file_chan_buff, FIFO_LEN);
      write(m_file_chan, NULL, FIFO_LEN);
      --ch_count;
    }
    fd_in_size = read(fd_in, m_file_chan_buff, FIFO_LEN);
    m_file_chan_buff[fd_in_size] = 0;
    write(m_file_chan, NULL, fd_in_size+1);
  }
  else {
    int32_t* dw_buff_out = (int32_t*)m_file_chan_buff;
    while (ch_count > 0) {
      read(fd_in, m_file_chan_buff, FIFO_LEN);
      write(m_file_chan, NULL, FIFO_LEN);
      read(m_file_chan, NULL, FIFO_LEN*TX_MAGNITUDE);

      size_t to_fwrite = 0;
      for (to_fwrite = 0; dw_buff_out[to_fwrite+1] &&
           to_fwrite<=(size_t)FIFO_LEN ;++to_fwrite) {;}

      lpf_process_result(file_id, dw_buff_out, to_fwrite == 0 ? 0 : to_fwrite+1);
      --ch_count;
    }

    fd_in_size = read(fd_in, m_file_chan_buff, FIFO_LEN);
    m_file_chan_buff[fd_in_size] = 0;
    write(m_file_chan, NULL, fd_in_size+1);
    read(m_file_chan, NULL, FIFO_LEN*TX_MAGNITUDE);

    size_t to_fwrite = 0;
    for (to_fwrite = 0; dw_buff_out[to_fwrite+1] &&
         to_fwrite<=(size_t)fd_in_size ;++to_fwrite) {;}
    lpf_process_result(file_id, dw_buff_out, to_fwrite == 0 ? 0 : to_fwrite+1);
  } //else
#endif // no TIME_MEASUREMENTS
}

#pragma GCC diagnostic pop
////////////////////////////////////////////////////////////////////////////

void SrmApplication::set_quiet_flag_to_fpga()
{
  int unused = -1;
  if (m_fd_uio != -1) {
    dev_access(m_fd_uio, RST_OFFSET, D_OUT, RESET, unused);
    dev_access(m_fd_uio, RST_OFFSET, D_OUT, DEV_QUIET_FLAG, unused);
  }
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::send_data_file_to_fpga(const char *input_file,
                                            int32_t file_id,
                                            lpf_proc_result lpf_process_result)
{
  int inf = open(input_file, O_RDWR);
  if (inf == -1) {
    printf("%s\n", input_file);
    perror("can't open input file");
    return;
  }
  set_match_count_to_zero();
  dma_load_input_file(inf, file_id, lpf_process_result);
  close(inf);
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::set_match_count_to_zero()
{
  int unused = -1;
  if (m_fd_uio != -1)
    dev_access(m_fd_uio, RST_OFFSET, D_OUT, RESET, unused);
}
////////////////////////////////////////////////////////////////////////////

int SrmApplication::open_dev_files()
{
  m_fd_uio = open (UIO_DEV, O_RDWR);
  if (m_fd_uio == -1) {
    perror("Can not open UIO device");
    return -1;
  }

  m_table_chan = open(DMA_TABLE, O_RDWR);
  if (m_table_chan == -1) {
    perror("Can not open AXI DMA file");
    return -2;
  }
  m_table_chan_buff = (int8_t*)mmap(NULL, DMA_BUFFER_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, m_table_chan, 0);

  if (m_table_chan_buff == NULL) {
    perror("Can't mmap table_chan buffer");
    return -3;
  }

  m_file_chan = open(DMA_DEV, O_RDWR);
  if (m_file_chan == -1) {
    perror("Can not open AXI DMA file");
    return -4;
  }
  m_file_chan_buff = (int8_t*)mmap(NULL, DMA_BUFFER_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, m_file_chan, 0);

  if (m_file_chan_buff == NULL) {
    perror("Can not mmap file_chan buffer");
    return -5;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::process_input_data_net(std::string buffer, lpf_proc_result lpf_process_result)
{
  size_t index = buffer.find("%%%", 0);
  std::string str_number = buffer.substr(index+3);
  size_t space_index = str_number.find(' ');
  std::string str_file = str_number.substr(space_index+1, std::string::npos);

  str_number = str_number.substr(0, space_index);
  //send data file

  send_data_file_to_fpga(str_file.c_str(), std::atoi(str_number.c_str()), lpf_process_result);

  if (--m_files_count == 0) {
    printf("JOB FINISHED\n");
    send(m_data_socket, &FINISH_ANSWER, sizeof(FINISH_ANSWER), 0);

#ifdef TIME_MEASUREMENTS
    CTimeMeasurements::Instance().print_results();
#endif
  }
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::process_result_data_net(int32_t file_id, int32_t *dw_buff_result, size_t len)
{
  if (len == 0) return;
#ifdef TIME_MEASUREMENTS
  IFunctor* func = new FunctorWithoutResult<int32_t, int32_t*, size_t>(
        process_result_data_net_internal, file_id, dw_buff_result, len, "process_result_data_net");
  CTimeMeasurements::Instance().average_measure(file_id, AMT_network_transfer, func);
  delete func;
#else
  process_result_data_net_internal(file_id, dw_buff_result, len);
#endif
}

void SrmApplication::process_result_data_net_internal(int32_t file_id, int32_t *dw_buff_result, size_t len)
{
  send(m_data_socket, &file_id, sizeof(int32_t), 0);
  send(m_data_socket, dw_buff_result, len*TX_MAGNITUDE, 0);
  send(m_data_socket, &END_MESSAGE, sizeof(END_MESSAGE), 0);
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::process_result_data_app(int32_t file_id, int32_t *dw_buff_result, size_t len)
{
  UNUSED_ARG(file_id);
  if (!CAppConfig::Instance()->verbose()) return;
  if (len == 1) return;
  printf("offsets : \n");
  for (size_t i = 0; i < len; ++i)
    printf("%d ,", dw_buff_result[i]);
  printf("\n");
}
////////////////////////////////////////////////////////////////////////////

static SynchroPrimitives::CriticalSection cs;
void SrmApplication::close_opened_files()
{
  static const char* fmt = "Error closing file %s, err  : %d\n";
  SynchroPrimitives::Locker lock(&cs);
  if (m_fd_uio != -1){
    if (close(m_fd_uio))
      printf(fmt, "uio", errno);
    m_fd_uio = -1;
  }

  if (m_table_chan != -1) {
    munmap(m_table_chan_buff, DMA_BUFFER_LEN);
    if (close(m_table_chan))
      printf(fmt, "table_chan", errno);
    m_table_chan = -1;
  }

  if (m_file_chan != -1) {
    munmap(m_file_chan_buff, DMA_BUFFER_LEN);
    if (close(m_file_chan))
      printf(fmt, "file_chan", errno);
    m_file_chan = -1;
  }
}

int SrmApplication::run(const char* input_file) {

  int run_result = 0;
  register_signal_handlers();
  printf("Signals registered\n");
  if (int c = open_dev_files() != 0)
    return c;
  printf("Device files opened\n");

  if(CAppConfig::Instance()->reload_table()) {
    if (reload_dfa_table(CAppConfig::Instance()->re_pattern()) != 0) {
      run_result = -1;
      goto end_main;
    }
  }
  printf("Dfa table reloaded\n");

  if (CAppConfig::Instance()->use_input_file()) {
    set_match_count_to_zero();
    send_data_file_to_fpga(input_file, 0, process_result_data_app);
    print_matchs_count();
#ifdef TIME_MEASUREMENTS
    CTimeMeasurements::Instance().print_results();
#endif
  }
  else {
    m_event_loop = new CEventLoop(SrmApplication::el_on_handle_exception,
                                  SrmApplication::el_on_handle_method_timeout,
                                  SrmApplication::el_on_log, 10000, true);
    while(init_sockets() && m_running_flag) sleep(3);
    m_event_loop->Run();
    listen_to_master();
    delete m_event_loop;
  }
end_main:

  close_opened_files();
  return run_result;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void SrmApplication::ctrl_c_handler(int s) {
  UNUSED_ARG(s);
  m_running_flag = false;
  usleep(300);
  close_opened_files();
  exit(0);
}
//////////////////////////////////////////////////////////////////////////

bool SrmApplication::register_signal_handlers() {
  static const char* fmt = "Couldn't register signal %s. error : %d\n";
  if (signal(SIGINT, ctrl_c_handler) == SIG_ERR)
    printf(fmt, "SIGINT", errno);
  if (signal(SIGQUIT, ctrl_c_handler) == SIG_ERR)
    printf(fmt, "SIGQUIT", errno);

  return true;
}
////////////////////////////////////////////////////////////////////////////

int SrmApplication::init_sockets()
{
  struct sockaddr_in serv_addr;
  struct hostent *server;

  m_command_socket = socket(AF_INET, SOCK_STREAM, 0);
  m_data_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_command_socket < 0) {
    printf("ERROR opening command socket %d\n", errno);
    return 1;
  }
  if (m_data_socket < 0) {
    printf("ERROR opening data socket %d\n", errno);
    return 2;
  }

  server = gethostbyname(CAppConfig::Instance()->server().c_str());
  if (server == NULL) {
    printf("Error, no such host\n");
    return 3;
  }

  bzero((int8_t*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((int8_t*)server->h_addr, (int8_t*)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(CAppConfig::Instance()->port());

  if (connect(m_command_socket,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("ERROR connecting command socket %d\n", errno);
    return 4;
  }
  send(m_command_socket, "command", 8, 0);

  if (connect(m_data_socket,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("ERROR connecting data socket %d\n", errno);
    return 5;
  }
  send(m_data_socket, "data", 5, 0);
  printf("Connected to master host\n");
  return 0;
}

static const int INVALID_SOCKET = -1;
void SrmApplication::listen_to_master() {
  static const int BUFF_SIZE = 2048;
  ssize_t n_recv;
  size_t data_offset = 0;
  size_t first_offset = 0;
  size_t i = 0;

  int8_t buffer[BUFF_SIZE] = {0};
  while (m_running_flag) {

    n_recv = recv(m_command_socket, &buffer[data_offset], BUFF_SIZE-data_offset, 0) + data_offset;

    if (n_recv < 0) {
      printf("ERROR reading from socket %d\n", errno);
      sleep(1);
      continue;
    }
    else if (n_recv == 0) { //master host closed connection
      close(m_command_socket);
      close(m_data_socket);
      for(;init_sockets() && m_running_flag;)
        sleep(3);
      continue;
    }

    //we have received data here
    i = first_offset = 0;
    while ((ssize_t)i < n_recv) {

      for (i = first_offset; (ssize_t)i < n_recv && buffer[i]; ++i)
        ;

      if ((ssize_t)i == n_recv) {
        data_offset = n_recv-first_offset;
        memcpy(buffer, &buffer[first_offset], data_offset);
        continue;
      }

      std::string str_command((char*)&buffer[first_offset], i-first_offset);
      if (str_command.compare("PING") == 0) {
        send(m_command_socket, &ALIVE_ANSWER, sizeof(ALIVE_ANSWER), 0);
      }
      else if (str_command[0] == '!') {
        m_files_count = std::atoi(str_command.substr(1).c_str());
      }
      else {
        IFunctor* func = new FunctorWithoutResult<std::string, lpf_proc_result>(process_input_data_net,
                                                                                str_command, process_result_data_net, "proc_inc_data");
        m_event_loop->InvokeActionAsync(func);
      }
      first_offset = i+1;
    } //while

  } //while running_flag
  close(m_command_socket);
  close(m_data_socket);
}
////////////////////////////////////////////////////////////////////////////


void SrmApplication::el_on_handle_exception(std::exception &elExceptionInfo)
{
  printf("Exception in event loop : %s\n", elExceptionInfo.what());
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::el_on_handle_method_timeout(const char *methodName)
{
  printf("Timeout in method %s\n", methodName);
}
////////////////////////////////////////////////////////////////////////////

void SrmApplication::el_on_log(const char *logMessage)
{
  printf("%s\n", logMessage);
}
////////////////////////////////////////////////////////////////////////////
