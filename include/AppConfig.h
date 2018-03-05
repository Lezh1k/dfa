#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "Commons.h"
#include <string>
#include <algorithm>

class CAppConfig {
private:
  static const char  ACK_SEPARATOR;
  static const char* ACK_QUIET;
  static const char* ACK_RE_PATTERN;
  static const char* ACK_ONLY_MATCHING;
  static const char* ACK_USE_INPUT_FILE;
  static const char* ACK_PORT;
  static const char* ACK_SERVER;
  static const char* ACK_USE_HARDWARE;
  static const char* ACK_VERBOSE;
  static const char* ACK_EXCLUDE_CARRIAGE_RETURN;

  std::string m_re_pattern;
  bool        m_quiet;
  bool        m_only_matching;
  bool        m_use_input_file;
  unsigned short m_port;
  std::string m_server;
  bool        m_use_hardware;
  bool        m_verbose;
  bool        m_exclude_carriage_return;

  void parse_line(std::string& line);
  int read_from_file(const char* fn);

  CAppConfig(void);
  ~CAppConfig(void);
  static CAppConfig* p_instance;

  struct mem_manager_t {
    mem_manager_t(void){}
    ~mem_manager_t(void){if (CAppConfig::p_instance) delete CAppConfig::p_instance;}
  };

  static mem_manager_t p_mem_manager;

public:

  static CAppConfig* Instance(void) ;
  void Init(bool debug = false);

  const std::string& re_pattern(void) const {return m_re_pattern;}
  bool quiet(void) const {return m_quiet && !m_only_matching;}
  bool only_matching(void) const{return m_only_matching;}
  bool reload_table(void) const{return !m_re_pattern.empty();}
  bool use_input_file(void) const{return m_use_input_file;}
  unsigned short port(void) const{return m_port;}
  const std::string& server(void) const{return m_server;}
  bool use_hardware(void) const{return m_use_hardware;}
  bool verbose(void) const {return m_verbose;}
  bool exclude_carriage_return(void) const {return m_exclude_carriage_return;}
};

#endif // APPCONFIG_H

