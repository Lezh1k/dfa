#include "AppConfig.h"
#include "IfstreamWrapper.h"

#define CONFIG_FILE_PATH "/etc/re_dfa_pattern"

const char  CAppConfig::ACK_SEPARATOR = ':';
const char* CAppConfig::ACK_ONLY_MATCHING = "only_matching";
const char* CAppConfig::ACK_QUIET = "quiet";
const char* CAppConfig::ACK_RE_PATTERN = "re_pattern";
const char* CAppConfig::ACK_USE_INPUT_FILE = "use_input_file";
const char* CAppConfig::ACK_PORT = "port";
const char* CAppConfig::ACK_SERVER = "server";
const char* CAppConfig::ACK_USE_HARDWARE = "use_hardware";
const char* CAppConfig::ACK_VERBOSE = "verbose";
const char* CAppConfig::ACK_EXCLUDE_CARRIAGE_RETURN = "exclude_carriage_return";

CAppConfig* CAppConfig::p_instance = NULL;
CAppConfig::mem_manager_t CAppConfig::p_mem_manager;

CAppConfig::CAppConfig() :
  m_re_pattern(""),
  m_quiet(false),
  m_only_matching(false),
  m_use_input_file(false),
  m_port(9987),
  m_server("localhost"),
  m_use_hardware(true),
  m_verbose(false),
  m_exclude_carriage_return(false){}

CAppConfig::~CAppConfig(void)
{
}

CAppConfig *CAppConfig::Instance()
{
  if (p_instance == NULL)
    p_instance = new CAppConfig;
  return p_instance;
}

void CAppConfig::Init(bool debug)
{
  if (debug) return;
  if (int r = read_from_file(CONFIG_FILE_PATH) != 0)
    printf("Error reading settings from %s. Code : %d", CONFIG_FILE_PATH, r);
}
////////////////////////////////////////////////////////////////////////////

int CAppConfig::read_from_file(const char *fn)
{
  CIfstreamWrapper wrapper(fn);
  if (!wrapper.IsOpen()) return -1;

  std::string tmp;
  while (true) {
    tmp = wrapper.GetLine();
    parse_line(tmp);
    if (wrapper.IsEof()) break;
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void CAppConfig::parse_line(std::string &line)
{
  CCommons::trim(line);
  if  (line[0] == '[' || line[0] == '#')
    return;

  int indexOf = line.find_first_of(ACK_SEPARATOR);
  if (indexOf == -1) return;

  std::string key = line.substr(0, indexOf);
  std::string value = line.substr(indexOf+1, line.size());

  if (key.compare(ACK_ONLY_MATCHING) == 0)
    m_only_matching = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else if (key.compare(ACK_QUIET) == 0)
    m_quiet = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else if (key.compare(ACK_RE_PATTERN) == 0)
    m_re_pattern = value;
  else if (key.compare(ACK_USE_INPUT_FILE) == 0)
    m_use_input_file = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else if (key.compare(ACK_PORT) == 0)
    m_port = std::atoi(value.c_str());
  else if (key.compare(ACK_SERVER) == 0)
    m_server = CCommons::trim(value);
  else if (key.compare(ACK_USE_HARDWARE) == 0)
    m_use_hardware = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else if (key.compare(ACK_VERBOSE) == 0)
    m_verbose = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else if (key.compare(ACK_EXCLUDE_CARRIAGE_RETURN) == 0)
    m_exclude_carriage_return = CCommons::to_lower(value) == "true" || CCommons::trim(value) == "1";
  else
    return;
}
////////////////////////////////////////////////////////////////////////////
