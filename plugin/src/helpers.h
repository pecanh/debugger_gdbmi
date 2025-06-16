#ifndef _Debugger_GDB_MI_HELPERS_H_
#define _Debugger_GDB_MI_HELPERS_H_

//#include <wx\string.h>
#include "definitions.h"

class Compiler;
class ProjectBuildTarget;
class Debugger_GDB_MI;
class CommandID; //(ph 2024/04/13)
class ResultParser; //(ph 2024/04/13)

namespace dbg_mi
{
    int ParseParentPID(const char *line);

    enum ERR_MSGTYPE_RESPONSE{ errNone, errInfo, errCritical, errAbort };

    void      SetRunActionMsg(wxString message, wxString title, ERR_MSGTYPE_RESPONSE msgtype );
    bool      GetRunActionMsg(wxString& msgtype, wxString& msgTitle, wxString& msgText);
    size_t    IsRunActionMsgQueued();

    bool      IsProcessRunning(long pid);

    Compiler* GetActiveCompiler(const cbProject* pProject = 0, ProjectBuildTarget* pTarget = 0);
    wxString  GetActiveCompilerID(const cbProject* pProject = 0, ProjectBuildTarget* pTarget = 0);
    wxString  GetMasterPath(ProjectBuildTarget* inpTarget);
    bool      SetDebugMark(const bool onoff);
    bool      IsActionsMapEmpty();
    wxString  AddQuotesIfNeeded(const wxString& str);   //(ph 2024/03/02)
    void AddSourceLineByFile(unsigned long int lineno, const wxString& filename);

    cbDebuggerPlugin* GetPluginParentPtr();
    Debugger_GDB_MI* GetPluginPtr();
    dbg_mi::WatchesContainer& GetWatchesContainer();

    // Routines primarily used to convert watches values (cf EditWatchesDlg)
    long AnyToLong(const wxString& inString);
    wxString AnyToIntStr(wxString& inString);
    wxString AnyToUIntStr(wxString& inString);
    //-    wxString IntStrToHexStr(wxString& inString);
    wxString IntToCharStr(wxString& inString);
    wxString IntToHexStr(wxString& inNumber);
    wxString IntToHexStr(wxString& inNumber);
    //-    wxString LongToHexStr(wxString& inNumber);
    wxString StdStrToHexStr(std::string& inString);
    wxString UIntToHexStr(wxString& inNumber);
    wxString CharToHexStr(wxString& inNumber);

    wxString wxStringToHexStr(wxString& str);
    void     TextDecToHex(int dec, char* buf);

    //-    wxString HexStrToIntStr(wxString& inString);
    //-    wxString CharToIntStr(const wxChar& inChar);
    wxString UIntToBinStr(const wxString& inStr);
    std::string std_itoa(int value, int base); //gcc itoa() didn't work

    wxString FindGdbInitFile(const wxString& folderPath); //(ph 2025/01/15)

} // namespace dbg_mi

#endif // _Debugger_GDB_MI_HELPERS_H_

