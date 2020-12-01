//Logger.h - Header file for the logger
//
// 13-Apr-19  G. Wu         Created.
// 14-Apr-19  G. Wu  Added Comments.
  
//The 4 severity levels
enum LOG_LEVEL : int {DEBUG = 0, WARNING = 1, ERROR = 2, CRITICAL = 3};

int InitializeLog();//Initalize the logger
void SetLogLevel(int level);//Setting the filter log level
void Log(int level, const char *prog, const char *func, int line, const char *message);//Send a log the server
void ExitLog();//Terminate the logger



