#pragma once

#include <filesystem>
#include <string>
#include <ctime>
#include <sys/socket.h>
#include <sys/un.h>

class DaemonManager {
public:
    DaemonManager(const std::filesystem::path& dataDirectory);
    ~DaemonManager();
    
    bool startDaemon();
    bool stopDaemon();
    bool restartDaemon();
    bool isDaemonRunning();
    bool forceUpdateCheck();
    bool refreshExecutablesCache();
    std::string getDaemonStatus();
    std::string getDaemonVersion();
    void setUpdateCheckInterval(int intervalSeconds);
    int getUpdateCheckInterval();
    bool isUpdateAvailable();
    std::string getLatestVersion();
    time_t getLastUpdateCheckTime();
    
private:
    std::filesystem::path dataDir;
    std::filesystem::path daemonDir;
    std::filesystem::path daemonPidFile;
    std::filesystem::path daemonStatusFile;
    std::filesystem::path daemonConfigFile;
    std::filesystem::path daemonPath;
    std::filesystem::path updateCacheFile;
    std::filesystem::path daemonLogFile;
    std::filesystem::path socketPath;
    
    void updateDaemonConfig();
    int getDaemonPid();
    
    bool connectToSocket();
    void disconnectFromSocket();
    std::string sendCommand(const std::string& command);
    int socketFd;
    bool socketConnected;
};
