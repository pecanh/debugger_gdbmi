#ifndef _Debugger_GDB_MI_ACTIONS_H_
#define _Debugger_GDB_MI_ACTIONS_H_

#include <memory>
#include<unordered_map>
#include "cmd_queue.h"
#include "definitions.h"
#include "cbplugin.h"
#include <infowindow.h>
#include "helpers.h"

//class cbDebuggerPlugin;

namespace dbg_mi
{

// ----------------------------------------------------------------------------
class SimpleAction : public Action
// ----------------------------------------------------------------------------
{
public:
    //-SimpleAction(wxString const &cmd) :
    SimpleAction(wxString const &cmd, Logger& logger) :
        m_command(cmd),
        m_logger(logger)
    {
    }

    //-virtual void OnCommandOutput(CommandID const & /*id*/, ResultParser const & /*result*/)
    virtual void OnCommandOutput(CommandID const & id, ResultParser const & result)
    {
        Finish();
    }
protected:
    virtual void OnStart()
    {
        Execute(m_command);
    }
private:
    wxString m_command;
    Logger& m_logger;
};

// ----------------------------------------------------------------------------
class BarrierAction : public Action
// ----------------------------------------------------------------------------
{
public:
    BarrierAction()
    {
        SetWaitPrevious(true);
    }
    virtual void OnCommandOutput(CommandID const & /*id*/, ResultParser const & /*result*/) {}
protected:
    virtual void OnStart()
    {
        Finish();
    }
};

class Breakpoint;

// ----------------------------------------------------------------------------
class BreakpointAddAction : public Action
// ----------------------------------------------------------------------------
{
public:
    BreakpointAddAction(std::shared_ptr<Breakpoint> const &breakpoint, Logger &logger) :
        m_breakpoint(breakpoint),
        m_logger(logger)
    {
    }
    virtual ~BreakpointAddAction()
    {
        m_logger.Debug(_T("BreakpointAddAction::destructor"));
    }
    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();
    void RemoveBreakpoint(wxString filename, int lineNo);

private:
    std::shared_ptr<Breakpoint> m_breakpoint;
    CommandID m_initial_cmd, m_disable_cmd;

    Logger &m_logger;
};

// ----------------------------------------------------------------------------
template<typename StopNotification>
class RunAction : public Action
// ----------------------------------------------------------------------------
{
public:
    RunAction(cbDebuggerPlugin *plugin, const wxString &command,
              StopNotification notification, Logger &logger) :
        m_plugin(plugin),
        m_command(command),
        m_notification(notification),
        m_logger(logger)
    {
        SetWaitPrevious(true);
    }
    virtual ~RunAction()
    {
        m_logger.Debug(_T("RunAction::destructor"));
    }

    //-virtual void OnCommandOutput(CommandID const &/*id*/, ResultParser const &result)
    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result)
    {
        if(result.GetResultClass() == ResultParser::ClassRunning)
        {
            m_logger.Debug(_T("RunAction success, the debugger is !stopped!"));
            m_logger.Debug(_T("RunAction::Output - ") + result.MakeDebugString());
            m_notification(false);
        }

        Finish();
    }
protected:
    virtual void OnStart()
    {
        Execute(m_command);
        m_logger.Debug(_T("RunAction::OnStart -> ") + m_command);
    }

private:
    cbDebuggerPlugin *m_plugin;
    wxString m_command;
    StopNotification m_notification;
    Logger &m_logger;
};

struct SwitchToFrameInvoker
{
    virtual ~SwitchToFrameInvoker() {}

    virtual void Invoke(int frame_number) = 0;
};

// ----------------------------------------------------------------------------
class GenerateBacktrace : public Action
// ----------------------------------------------------------------------------
{
    GenerateBacktrace(GenerateBacktrace &);
    GenerateBacktrace& operator =(GenerateBacktrace &);
public:
    GenerateBacktrace(SwitchToFrameInvoker *switch_to_frame, BacktraceContainer &backtrace,
                      CurrentFrame &current_frame, Logger &logger);
    virtual ~GenerateBacktrace();
    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();
private:
    SwitchToFrameInvoker *m_switch_to_frame;
    CommandID m_backtrace_id, m_args_id, m_frame_info_id;
    BacktraceContainer &m_backtrace;
    Logger &m_logger;
    CurrentFrame &m_current_frame;
    int m_first_valid, m_old_active_frame;
    bool m_parsed_backtrace, m_parsed_args, m_parsed_frame_info;
};

// ----------------------------------------------------------------------------
class GenerateThreadsList : public Action
// ----------------------------------------------------------------------------
{
public:
    GenerateThreadsList(ThreadsContainer &threads, int current_thread_id, Logger &logger);
    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();
private:
    ThreadsContainer &m_threads;
    Logger &m_logger;
    int m_current_thread_id;
};


// ----------------------------------------------------------------------------
template<typename Notification>
class SwitchToThread : public Action
// ----------------------------------------------------------------------------
{
public:
    SwitchToThread(int thread_id, Logger &logger, Notification const &notification) :
        m_thread_id(thread_id),
        m_logger(logger),
        m_notification(notification)
    {
    }

    virtual void OnCommandOutput(CommandID const &/*id*/, ResultParser const &result)
    {
        m_notification(result);
        Finish();
    }
protected:
    virtual void OnStart()
    {
        Execute(wxString::Format(_T("-thread-select %d"), m_thread_id));
    }

private:
    int m_thread_id;
    Logger &m_logger;
    Notification m_notification;
};

// ----------------------------------------------------------------------------
template<typename Notification>
class SwitchToFrame : public Action
// ----------------------------------------------------------------------------
{
public:
    SwitchToFrame(int frame_id, Notification const &notification, bool user_action) :
        m_frame_id(frame_id),
        m_notification(notification),
        m_user_action(user_action)
    {
    }

    virtual void OnCommandOutput(CommandID const &/*id*/, ResultParser const &result)
    {
        m_notification(result, m_frame_id, m_user_action);
        Finish();
    }
protected:
    virtual void OnStart()
    {
        Execute(wxString::Format(_T("-stack-select-frame %d"), m_frame_id));
    }
private:
    int m_frame_id;
    Notification m_notification;
    bool m_user_action;
};

// ----------------------------------------------------------------------------
class WatchBaseAction : public Action
// ----------------------------------------------------------------------------
{
public:
    WatchBaseAction(WatchesContainer &watches, Logger &logger);
    virtual ~WatchBaseAction();


protected:
    void ExecuteListCommand(Watch::Pointer watch, Watch::Pointer parent = Watch::Pointer());
    void ExecuteListCommand(wxString const &watch_id, Watch::Pointer parent);
    bool ParseListCommand(CommandID const &id, ResultValue const &value);

    void SetRange(int start, int end) { m_start = start; m_end = end; }
protected:
    typedef std::unordered_map<CommandID, Watch::Pointer> ListCommandParentMap;
protected:
    ListCommandParentMap m_parent_map;
    WatchesContainer& m_watches;
    Logger &m_logger;
    int m_sub_commands_left;
    int m_start, m_end;
};

// ----------------------------------------------------------------------------
class WatchCreateAction : public WatchBaseAction
// ----------------------------------------------------------------------------
{
    enum Step
    {
        StepCreate = 0,
        StepListChildren,
        StepSetRange
    };
public:
    WatchCreateAction(Watch::Pointer const &watch, WatchesContainer &watches, Logger &logger);

    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();

protected:
    Watch::Pointer m_watch;
    Step m_step;
};

// ----------------------------------------------------------------------------
class WatchCreateTooltipAction : public WatchCreateAction
// ----------------------------------------------------------------------------
{
public:
    WatchCreateTooltipAction(Watch::Pointer const &watch, WatchesContainer &watches, Logger &logger, wxRect const &rect) :
        WatchCreateAction(watch, watches, logger),
        m_rect(rect)
    {
    }
    virtual ~WatchCreateTooltipAction();
private:
    wxRect m_rect;
};

// ----------------------------------------------------------------------------
class WatchesUpdateAction : public WatchBaseAction
// ----------------------------------------------------------------------------
{
public:
    WatchesUpdateAction(WatchesContainer &watches, Logger &logger);
    WatchesUpdateAction(WatchesContainer &watches, Logger &logger, Watch::Pointer singleWatch); // single watch

    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();

private:
    bool ParseUpdate(ResultParser const &result);
private:
    CommandID   m_update_command;
    Watch::Pointer watch;
};

// ----------------------------------------------------------------------------
class WatchEvaluateExpression : public WatchBaseAction
// ----------------------------------------------------------------------------
{
public:
    WatchEvaluateExpression(Watch::Pointer const &watch, WatchesContainer &watches, Logger &logger);

    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();

private:
    bool ParseUpdate(ResultParser const &result);
private:
    CommandID   m_update_command;
    Watch::Pointer m_watch;
};

////// ----------------------------------------------------------------------------
////class WatchesUpdateFormat : public WatchBaseAction
////// ----------------------------------------------------------------------------
////{
////public:
////    WatchesUpdateFormat(WatchesContainer &watches, Logger &logger);
////
////    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
////protected:
////    virtual void OnStart();
////
////private:
////    bool ParseUpdate(ResultParser const &result);
////private:
////    //CommandID   m_update_command;
////
////};

class WatchExpandedAction : public WatchBaseAction
{
public:
    WatchExpandedAction(Watch::Pointer parent_watch, Watch::Pointer expanded_watch,
                        WatchesContainer &watches, Logger &logger) :
        WatchBaseAction(watches, logger),
        m_watch(parent_watch),
        m_expanded_watch(expanded_watch)
    {
        SetRange(0, 100);
        if (parent_watch->IsArray()) // implement edit Watches IsArray()
            SetRange(parent_watch->GetArrayStart(),
                        parent_watch->GetArrayStart()+parent_watch->GetArrayCount());
    }

    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();

private:
    CommandID m_update_id;
    Watch::Pointer m_watch;
    Watch::Pointer m_expanded_watch;
};

// ----------------------------------------------------------------------------
class WatchCollapseAction : public WatchBaseAction
// ----------------------------------------------------------------------------
{
public:
    WatchCollapseAction(Watch::Pointer parent_watch, Watch::Pointer collapsed_watch,
                        WatchesContainer &watches, Logger &logger) :
        WatchBaseAction(watches, logger),
        m_watch(parent_watch),
        m_collapsed_watch(collapsed_watch)
    {
    }

    virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
protected:
    virtual void OnStart();

private:
    Watch::Pointer m_watch;
    Watch::Pointer m_collapsed_watch;
};
// ----------------------------------------------------------------------------
class InfoRegisters : public Action
// ----------------------------------------------------------------------------
{

    InfoRegisters(InfoRegisters &);
    InfoRegisters& operator =(InfoRegisters &);

    public:
        // only tested on mingw/pc/win env
        InfoRegisters(Logger& logger, wxString disassemblyFlavor = wxEmptyString);
        virtual ~InfoRegisters();
        virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
    protected:
        virtual void OnStart();
    private:
        Logger& m_logger;
        wxString m_disassemblyFlavor;
        CommandID m_reg_values_id;
        CommandID m_reg_names_id;
        bool m_parsed_reg_names;
        bool m_parsed_reg_values;
        wxArrayString regNames;
        wxArrayString regValues;
};
// ----------------------------------------------------------------------------
class GenerateDisassembly : public Action
// ----------------------------------------------------------------------------
{
  // Command to run a disassembly.

    GenerateDisassembly(GenerateDisassembly &);
    GenerateDisassembly& operator =(GenerateDisassembly &);


    Logger& m_logger;
    bool m_mixedMode;
    CommandID m_frame_info_id;
    CommandID m_disassemble_info_id;
    bool m_parsed_frame_info;
    bool m_parsed_disassemble_info;

    public:
        GenerateDisassembly(Logger &logger, bool MixedMode=false, wxString hexAddrStr= _T(""));
        virtual ~GenerateDisassembly();
        //virtual void ParseOutput(const wxString& output);
        virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
    protected:
        virtual void OnStart();
};
// ----------------------------------------------------------------------------
class ExamineMemory : public Action
// ----------------------------------------------------------------------------
{

    ExamineMemory(ExamineMemory &);
    ExamineMemory& operator =(ExamineMemory &);

    public:
        // only tested on mingw/pc/win env
        ExamineMemory(Logger& logger);
        virtual ~ExamineMemory();
        virtual void OnCommandOutput(CommandID const &id, ResultParser const &result);
    protected:
        virtual void OnStart();
    private:
        Logger& m_logger;
        CommandID m_memory_values_id;
        bool m_parsed_memory_values;
};

} // namespace dbg_mi

#endif // _Debugger_GDB_MI_ACTIONS_H_
