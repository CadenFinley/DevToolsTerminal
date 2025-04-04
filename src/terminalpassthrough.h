#ifndef TERMINALPASSTHROUGH_H
#define TERMINALPASSTHROUGH_H
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <regex>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <map>

class TerminalPassthrough {
public:
    TerminalPassthrough();

    std::string getTerminalName();
    std::string returnCurrentTerminalPosition();
    int getTerminalCurrentPositionRawLength();
    void printCurrentTerminalPosition();

    std::vector<std::string> getFilesAtCurrentPath();
    std::string getFullPathOfFile(const std::string& file);
    std::string getCurrentFilePath();
    void setDisplayWholePath(bool displayWholePath);
    void toggleDisplayWholePath();
    bool isDisplayWholePath();

    std::thread executeCommand(std::string command);
    void addCommandToHistory(const std::string& command);
    std::string getPreviousCommand();
    std::string getNextCommand();

    std::vector<std::string> getTerminalCacheUserInput();
    std::vector<std::string> getTerminalCacheTerminalOutput();
    void clearTerminalCache();
    std::string returnMostRecentUserInput();
    std::string returnMostRecentTerminalOutput();

    void setEnvVar(const std::string& name, const std::string& value);
    std::string getEnvVar(const std::string& name) const;
    bool hasEnvVar(const std::string& name) const;
    void removeEnvVar(const std::string& name);
    std::map<std::string, std::string> getAllEnvVars() const;
    std::string expandEnvVars(const std::string& command) const;

    void setShellColor(const std::string& color);
    void setDirectoryColor(const std::string& color);
    void setBranchColor(const std::string& color);
    void setGitColor(const std::string& color);
    std::string getShellColor() const;
    std::string getDirectoryColor() const;
    std::string getBranchColor() const;
    std::string getGitColor() const;

private:
    std::string currentDirectory;
    bool displayWholePath;
    std::vector<std::string> terminalCacheUserInput;
    std::vector<std::string> terminalCacheTerminalOutput;
    std::string SHELL_COLOR = "\033[1;31m";
    std::string RESET_COLOR = "\033[0m";
    std::string DIRECTORY_COLOR = "\033[1;34m";
    std::string BRANCH_COLOR = "\033[1;33m";
    std::string GIT_COLOR = "\033[1;32m";
    int commandHistoryIndex = -1;
    int terminalCurrentPositionRawLength = 0;
    std::map<std::string, std::string> envVars;

    std::string getCurrentFileName();
    bool isRootPath(const std::filesystem::path& path);
};

#endif // TERMINALPASSTHROUGH_H
