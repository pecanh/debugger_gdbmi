#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <shobjidl.h> // Required for IFileOpenDialog

std::string SelectDirectory();
bool ShowMsgBox(std::string msg);
std::wstring ToWideString(const std::string& input);
int ExitInstaller(int retcode);
std::string FindPluginFile(std::string searchPath, std::string prefix, std::string suffix);
std::string GetProgramPath(const std::string& programName);

namespace fs = std::filesystem;
fs::path tempInstallPath = "";

// ----------------------------------------------------------------------------
int main()
// ----------------------------------------------------------------------------
{
    // This installer will copy the nightly debugger_gdbmi files to a user selected
    // CodeBlocks installation.
    // it will first ask the user to select a nightly CodeBlocks installation folder,
    // unzip the plugin files and copy them to their proper position.

    std::string msg = "This installer will copy the nightly debugger_gdbmi";
    msg            += "\nfiles to a user selected CodeBlocks installation.\n";
    msg            += "\nIt will first ask the user to select a nightly CodeBlocks installation folder,";
    msg            += "\nthen ask permission to copy the plugin files to their proper position.";
    msg            += "\n\nNote: The installation will fail if the selected CodeBlocks is running.";
    msg            += "\n\n-----------------------------------------------------------------------";
    msg            += "\n| Please direct your attention to the console window to continue.    |";
    msg            += "\n-----------------------------------------------------------------------";

    // MessageBox Arguments: Window Owner, Text, Title, Icon/Buttons
    //MessageBox(NULL, msg.c_str(), "Alert", MB_OK | MB_ICONINFORMATION);
    bool ok = ShowMsgBox(msg);
    if (ok != true) { exit(0);}

    // Ask user to navigate to the Nightly build and selelect it's directory
    std::cout << "Note that this installer will fail if CodeBlocks is currently running." << std::endl;
    std::cout << "\nPlease navigate to the folder containing CodeBlocks.exe and select it." << std::endl;
    std::string cbExeFolder = SelectDirectory();

    if (cbExeFolder.empty()) {
        std::cout << "Selection cancelled." << std::endl;
        ExitInstaller(1);
    } else {
        std::cout << "Selected: " << cbExeFolder << std::endl;
    }

    // Verify that codeblocks.exe exists here
    // Convert string to a path object
    fs::path rootPath(cbExeFolder);

    // Construct the target path using the '/' operator for safety
    fs::path pluginPath = rootPath / "share" / "CodeBlocks" / "plugins";

    // Validation by looking for the plugin folder
    if (fs::exists(pluginPath) && fs::is_directory(pluginPath)) {
        std::cout << "Target found: " << pluginPath.string() << std::endl;
        // Proceed with the installation...
    } else {
        std::cerr << "\nError: The selected path does not appear to be a valid Code::Blocks directory." << std::endl;
        std::cerr << "Expected to find a CodeBlocks executable directory." << std::endl;
        ExitInstaller(1);
    }

    std::string cbPath = cbExeFolder;

    // Get the path of the running .exe
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    fs::path pgmExePath = fs::path(buffer).parent_path();

    // Point to the plugins .zip file relative to the .exe
    std::string zipFileName = FindPluginFile(pgmExePath.string(), "debugger_gdbmi", ".zip");
    if (zipFileName == ""){
        std::cerr << "Error: plugin file debugger_gdbmi.zip not found!" << std::endl;
        ExitInstaller(1);
    }
    fs::path zipFullPath = pgmExePath / zipFileName;
    // Verify Zip
    if (not fs::exists(zipFullPath)) {
        std::cerr << "Error: debuggerMI.zip not found!" << std::endl;
        ExitInstaller(1);
    }

    // Point to the .dll file relative to the .exe
    std::string dllFileName = FindPluginFile(pgmExePath.string(), "debugger_gdbmi", ".dll");
    if (dllFileName == ""){
        std::cerr << "Error: plugin file debugger_gdbmi.dll not found!" << std::endl;
        ExitInstaller(1);
    }
    fs::path dllFullPath = pgmExePath / dllFileName;
    // Verify Zip
    if (!fs::exists(dllFullPath)) {
        std::cerr << "Error: debuggerMI.zip not found!" << std::endl;
        ExitInstaller(1);
    }

    // Get Permission to install the plugin
    std::cout << "\nInstall Code::Blocks Plugin? (y/n): ";
    char input; std::cin >> input;
    if (tolower(input) != 'y') ExitInstaller(0);

    // Copying plugin to CodeBlocks plugin locations
    bool errorOccured = false;
    try {
        fs::path sourceFile = dllFullPath;
        fs::path pluginDest = fs::path(cbPath) / "share/CodeBlocks/plugins/debugger_gdbmi.dll";
        fs::copy_file(sourceFile, pluginDest, fs::copy_options::overwrite_existing);
        std::cout << "\n========================================" << std::endl;
        std::cout << "Success! Plugin installed successfully." << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (fs::filesystem_error& e) {
        // 1. Create copies so we don't modify the originals
        fs::path p1 = e.path1();
        fs::path p2 = e.path2();

        // Normalize the slashes to the Windows style (\)
        p1.make_preferred();
        p2.make_preferred();

        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "COPY FAILED!" << std::endl;
        std::cerr << "Error: " << e.code().message() << std::endl;
        // Print using .string() to avoid double backslashes
        std::cerr << "From: " << p1.string() << std::endl;
        std::cerr << "To:   " << p2.string() << std::endl;

        std::cerr << "\nPossible cause: Code::Blocks is likely running." << std::endl;
        std::cerr << "Please close Code::Blocks and try again." << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        errorOccured = true;
    }

    // copy the manifest/image file .zip
    if (not errorOccured) {
    try {
        fs::path sourceFile =zipFullPath;
        fs::path pluginDest = fs::path(cbPath) / "share/CodeBlocks/debugger_gdbmi.zip";
        fs::copy_file(sourceFile, pluginDest, fs::copy_options::overwrite_existing);
        std::cout << "\n========================================" << std::endl;
        std::cout << "Success! Resources installed successfully." << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (fs::filesystem_error& e) {
        // 1. Create copies so we don't modify the originals
        fs::path p1 = e.path1();
        fs::path p2 = e.path2();

        // Normalize the slashes to the Windows style (\)
        p1.make_preferred();
        p2.make_preferred();

        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "COPY FAILED!" << std::endl;
        std::cerr << "Error: " << e.code().message() << std::endl;
        // Print using .string() to avoid double backslashes
        std::cerr << "From: " << p1.string() << std::endl;
        std::cerr << "To:   " << p2.string() << std::endl;

        std::cerr << "\nPossible cause: Code::Blocks is likely running." << std::endl;
        std::cerr << "Please close Code::Blocks and try again." << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }}

    ExitInstaller(errorOccured);
}
#include <conio.h> // Required for _getch()
// ----------------------------------------------------------------------------
int ExitInstaller(int retcode)
// ----------------------------------------------------------------------------
{
    // Remove the temp install folder
    if (fs::exists(tempInstallPath)) {
        try {
            fs::remove_all(tempInstallPath);
        } catch (...) {} // Ignore errors during cleanup
    }

    // Keep the console window open
    std::cout << "\nExit this console to release locked resources." << std::endl;
    std::cout << "Press any key to exit..." << std::endl;

    // This waits for a single raw keypress and ignores everything
    // currently stuck in the std::cin buffers.
    _getch();

    exit(retcode);
}
// ----------------------------------------------------------------------------
//  Select Directory
// ----------------------------------------------------------------------------
//  The Native Windows Folder Picker (Recommended)
//  This uses the Windows IFileOpenDialog interface.
//  It looks like a standard "Open" window but is configured to only pick folders.
//  This will need to link the library ole32 and uuid.
//
//    Linker Settings in Code::Blocks:
//    Add -lole32 and -luuid to the Linker Settings > Other linker options.
// ----------------------------------------------------------------------------
std::string SelectDirectory()
// ----------------------------------------------------------------------------
{
    std::string selectedPath = "";

    // Initialize COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            // Set the dialog to pick folders instead of files
            DWORD dwOptions;
            if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
                pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
            }

            pFileOpen->SetTitle(L"Please select the Code::Blocks Nightly directory");

            // Show the dialog
            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Convert wide string to standard string
                    if (SUCCEEDED(hr)) {
                        std::wstring ws(pszFilePath);
                        selectedPath = std::string(ws.begin(), ws.end());
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return selectedPath;
}
#include <windows.h>
#include <commctrl.h>

// ----------------------------------------------------------------------------
// The Callback function: Sets the window to Topmost as soon as it's created
// ----------------------------------------------------------------------------
HRESULT CALLBACK TaskDialogCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData) {
    // Quiet the unused parameter warnings
    (void)wParam;
    (void)lParam;
    (void)lpRefData;

    if (msg == TDN_CREATED) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    return S_OK;
}
// ----------------------------------------------------------------------------
bool ShowMsgBox(std::string msg)
// ----------------------------------------------------------------------------
{
    std::wstring wmsg = ToWideString(msg);
    HWND hwndTop = GetForegroundWindow();

    TASKDIALOGCONFIG config = {};
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.hwndParent = hwndTop;
    config.pszWindowTitle = L"DebuggerMI Plugin installer";
    config.pszMainInstruction = L"Installer for the CodeBlocks debuggerMI plugin";
    config.pszContent = wmsg.c_str();
    config.cxWidth = 250;

    config.dwCommonButtons = TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON;
    config.pszMainIcon = TD_INFORMATION_ICON;

    // --- Add these two lines to enable "Stay on Top" ---
    config.pfCallback = TaskDialogCallback;
    config.dwFlags = TDF_CAN_BE_MINIMIZED; // Optional: allows minimizing while topmost

    int nButtonPressed = 0;
    HRESULT hr = TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);

    if (SUCCEEDED(hr)) {
        if (nButtonPressed == IDCANCEL) {
            return false;
        }
    }

    return true;
}

//#include <vector>
// Helper function to convert std::string to std::wstring
// ----------------------------------------------------------------------------
std::wstring ToWideString(const std::string& input)
// ----------------------------------------------------------------------------
{
    if (input.empty()) return L"";

    // Get the required size for the new string
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &input[0], (int)input.size(), NULL, 0);

    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &input[0], (int)input.size(), &wstrTo[0], size_needed);

    return wstrTo;
}
// ----------------------------------------------------------------------------
// Finds a file in a directory that starts with a prefix and ends with a suffix
// ----------------------------------------------------------------------------
std::string FindPluginFile(std::string searchPath, std::string prefix, std::string suffix)
{
    try {
        fs::path p(searchPath);

        // Basic validation of the search directory
        if (!fs::exists(p) || !fs::is_directory(p)) {
            std::cerr << "Error: Search path does not exist or is not a directory: " << searchPath << std::endl;
            return "";
        }

        // Ensure the suffix starts with a dot if one wasn't provided (e.g., "zip" -> ".zip")
        if (!suffix.empty() && suffix[0] != '.') {
            suffix = "." + suffix;
        }

        for (const auto& entry : fs::directory_iterator(p)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                // Check length first to avoid out_of_range errors in compare()
                if (filename.length() >= prefix.length() + suffix.length()) {

                    bool startsWith = (filename.compare(0, prefix.length(), prefix) == 0);
                    bool endsWith = (filename.compare(filename.length() - suffix.length(),
                                                     suffix.length(), suffix) == 0);

                    if (startsWith && endsWith) {
                        return filename; // Found the match
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
    }

    return ""; // No matching file found
}
// ----------------------------------------------------------------------------
std::string GetProgramPath(const std::string& programName)
// ----------------------------------------------------------------------------
{
    char pathBuffer[MAX_PATH];
    char* filePart;

    // SearchPath searches for a specified file in a specified path.
    // Passing NULL as the first parameter tells it to use the system search path:
    // 1. The directory from which the application loaded.
    // 2. The current directory.
    // 3. The Windows system directory (GetSystemDirectory).
    // 4. The 16-bit Windows system directory.
    // 5. The Windows directory (GetWindowsDirectory).
    // 6. The directories listed in the PATH environment variable.
    DWORD result = SearchPathA(
        NULL,                // lpPath: NULL uses system PATH
        programName.c_str(), // lpFileName: name of the executable
        NULL,                // lpExtension: NULL if extension is in filename
        MAX_PATH,            // nBufferLength
        pathBuffer,          // lpBuffer: receives the full path
        &filePart            // lpFilePart: receives address of the filename in lpBuffer
    );

    if (result > 0 && result < MAX_PATH) {
        return std::string(pathBuffer);
    } else {
        return ""; // Not found
    }
}
