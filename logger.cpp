#include <iostream>
#include <fstream>

class Logger : public std::filebuf{

  public:
    enum LogType {};
    Logger(const char *fname) {
      m_file.open(fname, std::ofstream::out | std::ofstream::app);
    };
    ~Logger() {
      m_file.flush();
      m_file.close();
    };

  private:
    std::ofstream m_file;
}
