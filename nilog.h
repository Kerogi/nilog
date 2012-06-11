#ifndef __LOG_H__
#define __LOG_H__

#include <sstream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <fstream>

//-----------------------------------------------------------------------------
inline std::string GetNowTime();
inline std::string GetRunningProgramName();

//-----------------------------------------------------------------------------
//! A Non Intrusive Loger class.
/*!
    Simple class for logging use destructor for logging itself.
*/
template <typename OutputPolicy, typename FormatPolicy>
//! Template parameter OutputPolicy
/*!
    Output policy responcible how format output
*/
class NiLog
{
public:
    NiLog();
    virtual ~NiLog();
    std::ostringstream& Get();
protected:
    std::ostringstream os;

private:
    NiLog(const NiLog&);
	NiLog& operator =(const NiLog&);
};

template <typename OutputPolicy, typename FormatPolicy>
NiLog<OutputPolicy, FormatPolicy>::NiLog()
{

}

template <typename OutputPolicy, typename FormatPolicy>
std::ostringstream& NiLog<OutputPolicy, FormatPolicy>::Get()
{
    os << FormatPolicy::BeginMessage();
    return os;
}

template <typename OutputPolicy, typename FormatPolicy>
NiLog<OutputPolicy, FormatPolicy>::~NiLog()
{
	os << FormatPolicy::EndMessage();
    OutputPolicy::Output(FormatPolicy::Format(os.str()));
}

// Default output to file
//-----------------------------------------------------------------------------
class FileOutput
{
public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
};

inline FILE*& FileOutput::Stream()
{
    static FILE* pStream = fopen((GetRunningProgramName()+"_log.txt").c_str(), "w");;
    return pStream;
}

inline void FileOutput::Output(const std::string& msg)
{   
    FILE* pStream = Stream();
    if (!pStream)
        return;
    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}
//-----------------------------------------------------------------------------

// Some pretty formating
//-----------------------------------------------------------------------------
class ScopedIndent;
class NestedIndentFormat
{
    friend class ScopedIndent;
    static int indentLevel;
public:
	static std::string BeginMessage() {return GetNowTime() + " " + std::string((indentLevel>0)?indentLevel:0, '\t');};
	static std::string EndMessage() {return "\n";};
	static std::string Format(std::string msg) {return msg;};
};

int NestedIndentFormat::indentLevel = 0;

class ScopedIndent
{
public:
    ScopedIndent()
    {
		++NestedIndentFormat::indentLevel;
	}
	~ScopedIndent()
	{
		--NestedIndentFormat::indentLevel;
	}
};
//-----------------------------------------------------------------------------


// Default logger
//-----------------------------------------------------------------------------
class FileLog : public NiLog<FileOutput, NestedIndentFormat> {};

#define LOG \
    if (!Output2File::Stream()) ; \
    else FileLog().Get()

#define INDEND ScopedIndent indent;
//-----------------------------------------------------------------------------

// Some platform specific utility functions
//-----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

// Get string with a current system time
inline std::string GetNowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, 
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in GetNowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000); 
    return result;
}

// Get string with a current system time
inline std::string GetRunningProgramName()
{
    static char ModuleFileName[256];
    static bool inited = false;
    if(!inited) 
    {
		GetModuleFileNameA(NULL, ModuleFileName, sizeof(ModuleFileName));
		inited = true;
    }
    return ModuleFileName;
}

#else

#include <sys/time.h>

inline std::string GetNowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}

inline std::string GetRunningProgramName()
{
	return "empty";
}
#endif //WIN32

#endif //__LOG_H__
