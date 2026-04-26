Installation Guide: Debugger GDB/MI Plugin
A CodeBlocks plugin using the machine interface of the GDB debugger.

For Code::Blocks Nightly Build <Date>_<revision>_<OS> only.

Download binary zip: https://github.com/pecanh/debugger_gdbmi/assets/debugger_gdbmi_<nightlyDate>_<SVNrevision>_<OS>.zip
Source is available at Github: https://github.com/pecanh/debugger_gdbmi
1. Preparation
• Ensure Code::Blocks is not running.
• Extract the ZIP contents into a temporary folder.
• Note: While installing, the .exe, .dll, and .zip files must stay together.

2. Installation
• Run InstallDebuggerMI.exe.
• Select your Code::Blocks Nightly folder when prompted.
• Constraint: Your CB Nightly version date must match the install zips file name date.
• Installing into a mismatched nightly version will cause it to stall and crash.

3. Configuration
Inside Code::Blocks, go to:
Settings -> Debugger -> GDB/MI default
• Executable Path: Browse to your gdb.exe.
• Verify the othe options meet your requirements.
• Click on Ok button.

4. Usage
Go to Debug -> Active Debuggers and select GDB/MI default.

Report issues in this thread with your Nightly Build date and OS.
