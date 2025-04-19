#include "include/terminalpassthrough.h"

TerminalPassthrough::TerminalPassthrough() : displayWholePath(false) {
    currentDirectory = std::filesystem::current_path().string();
    terminalCacheUserInput = std::vector<std::string>();
    terminalCacheTerminalOutput = std::vector<std::string>();
    lastGitStatusCheck = std::chrono::steady_clock::now() - std::chrono::seconds(10);
    isGitStatusCheckRunning = false;
    shouldTerminate = false; // Initialize termination flag
    //     std::string terminalID= "dtt";
    //     char buffer[256];
    //     std::string command = "ps -p " + std::to_string(getppid()) + " -o comm=";
    //     FILE* pipe = popen(command.c_str(), "r");
    //     if (pipe) {
    //         if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    //             terminalID = std::string(buffer);
    //             terminalID.erase(std::remove(terminalID.begin(), terminalID.end(), '\n'), terminalID.end());
    //             terminalID = removeSpecialCharacters(terminalID);
    //         }
    //         pclose(pipe);
    //     }
    // terminalName = terminalID;
    terminalName = "dtt";
    
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

TerminalPassthrough::~TerminalPassthrough() {
    shouldTerminate = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

std::string TerminalPassthrough::removeSpecialCharacters(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (isalnum(c)) {
            result += c;
        }
    }
    return result;
}

std::string TerminalPassthrough::getTerminalName(){
    return terminalName;
}

std::vector<std::string> TerminalPassthrough::getFilesAtCurrentPath(const bool& includeHidden, const bool& fullFilePath, const bool& includeDirectories){
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(getCurrentFilePath())) {
        if (includeHidden || entry.path().filename().string()[0] != '.') {
            if (includeDirectories || !std::filesystem::is_directory(entry.path())) {
                std::string fileName = entry.path().filename().string();
                if (fullFilePath) {
                    fileName = entry.path().string();
                } else {
                    fileName = entry.path().filename().string();
                }
            }
        }
    }
    return files;
}

void TerminalPassthrough::setDisplayWholePath(bool displayWholePath){
    this->displayWholePath = displayWholePath;
}

void TerminalPassthrough::setAliases(const std::map<std::string, std::string>& aliases){
    this->aliases = aliases;
}

std::string TerminalPassthrough::getFullPathOfFile(const std::string& file){
    if (std::filesystem::exists (std::filesystem::path(getCurrentFilePath()) / file)) {
        return (std::filesystem::path(getCurrentFilePath()) / file).string();
    }
    return "";
}

void TerminalPassthrough::printCurrentTerminalPosition(){
    std::cout << returnCurrentTerminalPosition();
}

int TerminalPassthrough::getTerminalCurrentPositionRawLength(){
    return terminalCurrentPositionRawLength;
}

std::string TerminalPassthrough::returnCurrentTerminalPosition(){
    int gitInfoLength = 0;
    std::string gitInfo;
    std::filesystem::path currentPath = std::filesystem::path(getCurrentFilePath());
    std::filesystem::path gitHeadPath;
    while (!isRootPath(currentPath)) {
        gitHeadPath = currentPath / ".git" / "HEAD";
        if (std::filesystem::exists(gitHeadPath)) {
            break;
        }
        currentPath = currentPath.parent_path();
    }
    bool gitRepo = std::filesystem::exists(gitHeadPath);
    if (gitRepo) {
        try {
            std::ifstream headFile(gitHeadPath);
            std::string line;
            std::regex headPattern("ref: refs/heads/(.*)");
            std::smatch match;
            std::string branchName;
            while (std::getline(headFile, line)) {
                if (std::regex_search(line, match, headPattern)) {
                    branchName = match[1];
                    break;
                }
            }

            std::string statusSymbols = "";
            std::string gitDir = currentPath.string();
            bool isCleanRepo = true;
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastGitStatusCheck).count();
            
            if ((elapsed > 30 || cachedGitDir != gitDir) && !isGitStatusCheckRunning) {
                isGitStatusCheckRunning = true;
                if (cachedGitDir != gitDir) {
                    std::thread statusThread([this, gitDir]() {
                        if (shouldTerminate) {
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            isGitStatusCheckRunning = false;
                            return;
                        }
                        
                        std::string gitStatusCmd = "sh -c \"cd " + gitDir + 
                            " && git status --porcelain | head -1\"";
                        
                        FILE* statusPipe = popen(gitStatusCmd.c_str(), "r");
                        char statusBuffer[1024];
                        std::string statusOutput = "";
                        
                        if (statusPipe) {
                            while (fgets(statusBuffer, sizeof(statusBuffer), statusPipe) != nullptr) {
                                statusOutput += statusBuffer;
                                if (shouldTerminate) {
                                    pclose(statusPipe);
                                    std::lock_guard<std::mutex> lock(gitStatusMutex);
                                    isGitStatusCheckRunning = false;
                                    return;
                                }
                            }
                            pclose(statusPipe);
                            
                            if (shouldTerminate) {
                                std::lock_guard<std::mutex> lock(gitStatusMutex);
                                isGitStatusCheckRunning = false;
                                return;
                            }
                            
                            bool isClean = statusOutput.empty();
                            std::string symbols = "";
                            
                            if (!isClean) {
                                symbols = "*";
                            }
                            
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            cachedGitDir = gitDir;
                            cachedStatusSymbols = symbols;
                            cachedIsCleanRepo = isClean;
                            lastGitStatusCheck = std::chrono::steady_clock::now();
                            isGitStatusCheckRunning = false;
                        } else {
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            isGitStatusCheckRunning = false;
                        }
                    });
                    statusThread.detach();
                } else {
                    std::thread statusThread([this, gitDir]() {
                        if (shouldTerminate) {
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            isGitStatusCheckRunning = false;
                            return;
                        }
                        
                        std::string gitStatusCmd = "sh -c \"cd " + gitDir + 
                            " && git status --porcelain | head -1\"";
                        
                        FILE* statusPipe = popen(gitStatusCmd.c_str(), "r");
                        char statusBuffer[1024];
                        std::string statusOutput = "";
                        
                        if (statusPipe) {
                            while (fgets(statusBuffer, sizeof(statusBuffer), statusPipe) != nullptr) {
                                statusOutput += statusBuffer;
                                if (shouldTerminate) {
                                    pclose(statusPipe);
                                    std::lock_guard<std::mutex> lock(gitStatusMutex);
                                    isGitStatusCheckRunning = false;
                                    return;
                                }
                            }
                            pclose(statusPipe);
                            
                            if (shouldTerminate) {
                                std::lock_guard<std::mutex> lock(gitStatusMutex);
                                isGitStatusCheckRunning = false;
                                return;
                            }
                            
                            bool isClean = statusOutput.empty();
                            std::string symbols = "";
                            
                            if (!isClean) {
                                symbols = "*";
                            }
                            
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            cachedGitDir = gitDir;
                            cachedStatusSymbols = symbols;
                            cachedIsCleanRepo = isClean;
                            lastGitStatusCheck = std::chrono::steady_clock::now();
                            isGitStatusCheckRunning = false;
                        } else {
                            std::lock_guard<std::mutex> lock(gitStatusMutex);
                            isGitStatusCheckRunning = false;
                        }
                    });
                    statusThread.detach();
                }
                
                statusSymbols = cachedStatusSymbols;
                isCleanRepo = cachedIsCleanRepo;
            } else {
                std::lock_guard<std::mutex> lock(gitStatusMutex);
                statusSymbols = cachedStatusSymbols;
                isCleanRepo = cachedIsCleanRepo;
            }
            
            std::string repoName = displayWholePath ? getCurrentFilePath() : getCurrentFileName();
            std::string statusInfo;
            
            if (isCleanRepo) {
                statusInfo = " ✓";
            } else {
                statusInfo = " " + statusSymbols;
            }
            
            gitInfoLength = repoName.length() + branchName.length() + statusInfo.length() + 9;
            gitInfo = GIT_COLOR + repoName + RESET_COLOR + DIRECTORY_COLOR + " git:(" + RESET_COLOR + BRANCH_COLOR + branchName + RESET_COLOR;
            
            if (isCleanRepo) {
                gitInfo += DIRECTORY_COLOR + statusInfo + RESET_COLOR;
            } else if (!statusSymbols.empty()) {
                gitInfo += DIRECTORY_COLOR + statusInfo + RESET_COLOR;
            }
            
            gitInfo += DIRECTORY_COLOR + ")" + RESET_COLOR;
        } catch (const std::exception& e) {
            std::cerr << "Error reading git HEAD file: " << e.what() << std::endl;
        }
        terminalCurrentPositionRawLength = getTerminalName().length() + 2 + gitInfoLength;
        return SHELL_COLOR + getTerminalName() + RESET_COLOR + " " + gitInfo + " ";
    }
    
    if (displayWholePath) {
        terminalCurrentPositionRawLength = getCurrentFilePath().length() + getTerminalName().length() + 2;
        return SHELL_COLOR+getTerminalName()+RESET_COLOR + " " + DIRECTORY_COLOR + getCurrentFilePath() + RESET_COLOR + " ";
    } else {
        terminalCurrentPositionRawLength = getCurrentFileName().length() + getTerminalName().length() + 2;
        return SHELL_COLOR+getTerminalName()+RESET_COLOR + " " + DIRECTORY_COLOR + getCurrentFileName() + RESET_COLOR + " ";
    }
}

std::thread TerminalPassthrough::executeCommand(std::string command) {
    addCommandToHistory(command);
    return std::thread([this, command = std::move(command)]() {
        try {
            std::string result;
            
            std::string processedCommand = command;
            std::istringstream iss(command);
            std::string commandName;
            iss >> commandName;
            
            if (commandName == "cd") {
                std::string dirArg;
                iss >> dirArg;
                
                if (!dirArg.empty() && aliases.find(dirArg) != aliases.end()) {
                    std::string aliasDef = aliases[dirArg];
                    std::filesystem::path potentialDir = std::filesystem::path(currentDirectory) / aliasDef;
                    
                    if (std::filesystem::exists(potentialDir) && std::filesystem::is_directory(potentialDir)) {
                        std::string remainingArgs;
                        std::getline(iss >> std::ws, remainingArgs);
                        
                        processedCommand = "cd " + aliasDef;
                        if (!remainingArgs.empty()) {
                            processedCommand += " " + remainingArgs;
                        }
                    }
                }
            }
            else if (!commandName.empty() && aliases.find(commandName) != aliases.end()) {
                std::string args;
                std::getline(iss >> std::ws, args);
                
                processedCommand = aliases[commandName];
                if (!args.empty()) {
                    processedCommand += " " + args;
                }
            }
            
            parseAndExecuteCommand(processedCommand, result);
            terminalCacheTerminalOutput.push_back(result);
            
        } catch (const std::exception& e) {
            std::string errorMsg = "Error executing command: '" + command + "' " + e.what();
            std::cerr << errorMsg << std::endl;
            terminalCacheTerminalOutput.push_back(std::move(errorMsg));
        }
    });
}

void TerminalPassthrough::parseAndExecuteCommand(const std::string& command, std::string& result) {
    // Split the command by "&&" and execute each part sequentially
    std::string remainingCommand = command;
    bool success = true;
    std::string commandResults;
    
    while (!remainingCommand.empty() && success) {
        size_t andPos = remainingCommand.find("&&");
        std::string currentCommand;
        
        if (andPos != std::string::npos) {
            currentCommand = remainingCommand.substr(0, andPos);
            // Trim whitespace
            size_t lastNonSpace = currentCommand.find_last_not_of(" \t");
            if (lastNonSpace != std::string::npos) {
                currentCommand = currentCommand.substr(0, lastNonSpace + 1);
            }
            remainingCommand = remainingCommand.substr(andPos + 2);
            // Trim leading whitespace from remaining command
            size_t firstNonSpace = remainingCommand.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos) {
                remainingCommand = remainingCommand.substr(firstNonSpace);
            } else {
                remainingCommand.clear();
            }
        } else {
            currentCommand = remainingCommand;
            remainingCommand.clear();
        }
        
        std::string partialResult;
        success = executeIndividualCommand(currentCommand, partialResult);
        
        if (!commandResults.empty()) {
            commandResults += "\n";
        }
        commandResults += partialResult;
        
        if (!success) {
            break;
        }
    }
    
    result = commandResults;
}

bool TerminalPassthrough::executeIndividualCommand(const std::string& command, std::string& result) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    if (cmd == "cd") {
        std::string dir;
        std::getline(iss >> std::ws, dir);
        return changeDirectory(dir, result);
    } 
    else if (cmd == "jobs") {
        std::stringstream jobOutput;
        updateJobStatus();
        
        std::lock_guard<std::mutex> lock(jobsMutex);
        if (jobs.empty()) {
            jobOutput << "No active jobs";
        } else {
            for (size_t i = 0; i < jobs.size(); i++) {
                const auto& job = jobs[i];
                jobOutput << "[" << (i+1) << "] " 
                          << (job.foreground ? "Running " : "Stopped ")
                          << job.command << " (PID: " << job.pid << ")\n";
            }
        }
        result = jobOutput.str();
        return true;
    }
    else if (cmd == "fg") {
        int jobId = 0;
        iss >> jobId;
        if (jobId <= 0) jobId = 1;
        
        if (bringJobToForeground(jobId)) {
            result = "Job brought to foreground";
            return true;
        } else {
            result = "No such job";
            return false;
        }
    }
    else if (cmd == "bg") {
        int jobId = 0;
        iss >> jobId;
        if (jobId <= 0) jobId = 1;
        
        if (sendJobToBackground(jobId)) {
            result = "Job sent to background";
            return true;
        } else {
            result = "No such job";
            return false;
        }
    }
    else if (cmd == "kill") {
        int jobId = 0;
        iss >> jobId;
        
        if (killJob(jobId)) {
            result = "Job killed";
            return true;
        } else {
            result = "No such job";
            return false;
        }
    }
    else {
        bool background = false;
        
        std::string fullCommand = command;
        if (!fullCommand.empty() && fullCommand.back() == '&') {
            background = true;
            fullCommand.pop_back();

            size_t lastNonSpace = fullCommand.find_last_not_of(" \t");
            if (lastNonSpace != std::string::npos) {
                fullCommand = fullCommand.substr(0, lastNonSpace + 1);
            }
        }
        
        if (background) {
            pid_t pid = executeChildProcess(fullCommand, false);
            {
                std::lock_guard<std::mutex> lock(jobsMutex);
                jobs.push_back(Job(pid, fullCommand, false));
                result = "Started background process [" + std::to_string(jobs.size()) + "] (PID: " + std::to_string(pid) + ")";
            }
            return true;
        } else {
            pid_t pid = executeChildProcess(fullCommand, true);
            
            updateJobStatus();
            result = "Command completed";
            return true;
        }
    }
}

pid_t TerminalPassthrough::executeChildProcess(const std::string& command, bool foreground) {
    pid_t pid = fork();
    
    if (pid == -1) {
        throw std::runtime_error("Failed to fork process");
    }
    else if (pid == 0) {
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        
        pid_t childPid = getpid();
        setpgid(childPid, childPid);

        if (foreground) {
            tcsetpgrp(STDIN_FILENO, childPid);
        }

        if (!foreground) {
            setsid();
        }

        chdir(currentDirectory.c_str());
        
        std::vector<std::string> args = parseCommandIntoArgs(command);
        if (!args.empty()) {
            std::vector<char*> argv;
            for (auto& arg : args) {
                argv.push_back(arg.data());
            }
            argv.push_back(nullptr);
            
            std::string executable = findExecutableInPath(args[0]);
            if (!executable.empty()) {
                execvp(executable.c_str(), argv.data());
            }
        }
        
        std::cerr << "dtt: command not found: " << args[0] << std::endl;
        exit(EXIT_FAILURE);
    }
    
    setpgid(pid, pid);
    
    if (foreground) {
        tcsetpgrp(STDIN_FILENO, pid);
        
        waitForForegroundJob(pid);
        
        tcsetpgrp(STDIN_FILENO, getpgid(0));
    }
    
    return pid;
}

bool TerminalPassthrough::changeDirectory(const std::string& dir, std::string& result) {
    std::string targetDir = dir;
    
    if (targetDir.empty() || targetDir == "~") {
        const char* homeDir = getenv("HOME");
        if (homeDir) {
            targetDir = homeDir;
        } else {
            result = "Could not determine home directory";
            return false;
        }
    }
    
    if (targetDir == "/") {
        currentDirectory = "/";
    } else if (targetDir == "..") {
        std::filesystem::path dirPath = std::filesystem::path(currentDirectory).parent_path();
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
            currentDirectory = dirPath.string();
        } else {
            result = "Cannot go up from root directory";
            return false;
        }
    } else {
        std::filesystem::path dirPath;
        
        if (targetDir[0] == '/') {
            dirPath = targetDir;
        } else {
            dirPath = std::filesystem::path(currentDirectory) / targetDir;
        }
        
        if (!std::filesystem::exists(dirPath)) {
            result = "cd: " + targetDir + ": No such file or directory";
            return false;
        }
        
        if (!std::filesystem::is_directory(dirPath)) {
            result = "cd: " + targetDir + ": Not a directory";
            return false;
        }
        
        try {
            currentDirectory = std::filesystem::canonical(dirPath).string();
        } catch (const std::filesystem::filesystem_error& e) {
            result = "cd: " + targetDir + ": " + e.what();
            return false;
        }
    }
    
    if (chdir(currentDirectory.c_str()) != 0) {
        std::string errorMsg = "cd: " + std::string(strerror(errno));
        result = errorMsg;
        return false;
    }
    
    result = "Changed directory to: " + currentDirectory;
    return true;
}

void TerminalPassthrough::waitForForegroundJob(pid_t pid) {
    struct termios term_settings;
    tcgetattr(STDIN_FILENO, &term_settings);
    
    int status;
    waitpid(pid, &status, WUNTRACED);
    
    if (WIFSTOPPED(status)) {
        for (auto& job : jobs) {
            if (job.pid == pid) {
                job.foreground = false;
                job.status = status;
                
                tcsetpgrp(STDIN_FILENO, getpgid(0));
                return;
            }
        }
        
        jobs.push_back(Job(pid, "Unknown command", false));
        jobs.back().status = status;
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term_settings);
}

void TerminalPassthrough::updateJobStatus() {
    std::lock_guard<std::mutex> lock(jobsMutex);
    for (auto it = jobs.begin(); it != jobs.end(); ) {
        int status;
        pid_t result = waitpid(it->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        
        if (result == 0) {
            ++it;
        } else if (result == it->pid) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                it = jobs.erase(it);
            } else {
                it->status = status;
                it->foreground = false;
                ++it;
            }
        } else {
            it = jobs.erase(it);
        }
    }
}

void TerminalPassthrough::listJobs() {
    updateJobStatus();
    
    std::lock_guard<std::mutex> lock(jobsMutex);
    if (jobs.empty()) {
        std::cout << "No active jobs" << std::endl;
        return;
    }
    
    for (size_t i = 0; i < jobs.size(); i++) {
        const auto& job = jobs[i];
        std::cout << "[" << (i+1) << "] " 
                  << (WIFSTOPPED(job.status) ? "Stopped " : "Running ")
                  << job.command << " (PID: " << job.pid << ")" << std::endl;
    }
}

bool TerminalPassthrough::bringJobToForeground(int jobId) {
    updateJobStatus();
    
    std::lock_guard<std::mutex> lock(jobsMutex);
    if (jobId <= 0 || jobId > static_cast<int>(jobs.size())) {
        return false;
    }
    
    struct termios term_settings;
    tcgetattr(STDIN_FILENO, &term_settings);
    
    Job& job = jobs[jobId - 1];
    job.foreground = true;
    
    if (WIFSTOPPED(job.status)) {
        kill(-job.pid, SIGCONT);
    }
    
    tcsetpgrp(STDIN_FILENO, job.pid);
    
    waitForForegroundJob(job.pid);
    
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    
    tcsetattr(STDIN_FILENO, TCSANOW, &term_settings);
    
    return true;
}

bool TerminalPassthrough::sendJobToBackground(int jobId) {
    updateJobStatus();
    
    std::lock_guard<std::mutex> lock(jobsMutex);
    if (jobId <= 0 || jobId > static_cast<int>(jobs.size())) {
        return false;
    }
    
    Job& job = jobs[jobId - 1];
    job.foreground = false;
    
    if (WIFSTOPPED(job.status)) {
        kill(-job.pid, SIGCONT);
    }
    
    return true;
}

bool TerminalPassthrough::killJob(int jobId) {
    updateJobStatus();
    
    std::lock_guard<std::mutex> lock(jobsMutex);
    if (jobId <= 0 || jobId > static_cast<int>(jobs.size())) {
        return false;
    }
    
    Job& job = jobs[jobId - 1];
    
    kill(-job.pid, SIGTERM);
    
    return true;
}

void TerminalPassthrough::toggleDisplayWholePath(){
    setDisplayWholePath(!displayWholePath);
}

bool TerminalPassthrough::isDisplayWholePath(){
    return displayWholePath;
}

std::vector<std::string> TerminalPassthrough::getTerminalCacheUserInput(){
    return terminalCacheUserInput;
}

std::vector<std::string> TerminalPassthrough::getTerminalCacheTerminalOutput(){
    return terminalCacheTerminalOutput;
}

void TerminalPassthrough::clearTerminalCache(){
    terminalCacheUserInput.clear();
    terminalCacheTerminalOutput.clear();
}

std::string TerminalPassthrough::returnMostRecentUserInput(){
    return terminalCacheUserInput.back();
}

std::string TerminalPassthrough::returnMostRecentTerminalOutput(){
    return terminalCacheTerminalOutput.back();
}

std::string TerminalPassthrough::getPreviousCommand() {
    if (terminalCacheUserInput.empty()) {
        return "";
    }
    if (commandHistoryIndex < terminalCacheUserInput.size() - 1) {
        commandHistoryIndex++;
    } else {
        commandHistoryIndex = 0;
    }
    return terminalCacheUserInput[commandHistoryIndex];
}

std::string TerminalPassthrough::getNextCommand() {
    if (terminalCacheUserInput.empty()) {
        return "";
    }
    if (commandHistoryIndex > 0) {
        commandHistoryIndex--;
    } else {
        commandHistoryIndex = terminalCacheUserInput.size() - 1;
    }
    return terminalCacheUserInput[commandHistoryIndex];
}

std::string TerminalPassthrough::getCurrentFilePath(){
    if (currentDirectory.empty()) {
        return std::filesystem::current_path().string();
    }
    return currentDirectory;
}

std::string TerminalPassthrough::getCurrentFileName(){
    std::string currentDirectory = getCurrentFilePath();
    std::string currentFileName = std::filesystem::path(currentDirectory).filename().string();
    if (currentFileName.empty()) {
        return "/";
    }
    return currentFileName;
}

bool TerminalPassthrough::isRootPath(const std::filesystem::path& path){
    return path == path.root_path();
}

void TerminalPassthrough::addCommandToHistory(const std::string& command) {
    if (command.empty()) {
        return;
    }
    if (std::find(terminalCacheUserInput.begin(), terminalCacheUserInput.end(), command) != terminalCacheUserInput.end()) {
        return;
    }
    commandHistoryIndex = terminalCacheUserInput.size();
    terminalCacheUserInput.push_back(command);
}

void TerminalPassthrough::setShellColor(const std::string& color){
    this->SHELL_COLOR = color;
}

void TerminalPassthrough::setDirectoryColor(const std::string& color){
    this->DIRECTORY_COLOR = color;
}

void TerminalPassthrough::setBranchColor(const std::string& color){
    this->BRANCH_COLOR = color;
}

void TerminalPassthrough::setGitColor(const std::string& color){
    this->GIT_COLOR = color;
}

std::string TerminalPassthrough::getShellColor() const {
    return SHELL_COLOR;
}

std::string TerminalPassthrough::getDirectoryColor() const {
    return DIRECTORY_COLOR;
}

std::string TerminalPassthrough::getBranchColor() const {
    return BRANCH_COLOR;
}

std::string TerminalPassthrough::getGitColor() const {
    return GIT_COLOR;
}

std::vector<std::string> TerminalPassthrough::getCommandHistory(size_t count) {
    std::vector<std::string> recentCommands;
    size_t historySize = terminalCacheUserInput.size();
    size_t numCommands = std::min(count, historySize);
    
    for (size_t i = 0; i < numCommands; i++) {
        size_t index = historySize - 1 - i;
        recentCommands.push_back(terminalCacheUserInput[index]);
    }
    
    return recentCommands;
}

void TerminalPassthrough::terminateAllChildProcesses() {
    std::lock_guard<std::mutex> lock(jobsMutex);
    for (const auto& job : jobs) {
        kill(-job.pid, SIGKILL);
    }
    jobs.clear();
}

std::vector<std::string> TerminalPassthrough::parseCommandIntoArgs(const std::string& command) {
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;
    bool inQuotes = false;
    char quoteChar = 0;
    std::string currentArg;
    
    for (size_t i = 0; i < command.length(); i++) {
        char c = command[i];
        
        if ((c == '"' || c == '\'') && (i == 0 || command[i-1] != '\\')) {
            if (!inQuotes) {
                inQuotes = true;
                quoteChar = c;
            } else if (c == quoteChar) {
                inQuotes = false;
                quoteChar = 0;
            } else {
                currentArg += c;
            }
        } else if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!currentArg.empty()) {
                args.push_back(currentArg);
                currentArg.clear();
            }
        } else {
            currentArg += c;
        }
    }
    
    if (!currentArg.empty()) {
        args.push_back(currentArg);
    }
    
    return args;
}

std::string TerminalPassthrough::findExecutableInPath(const std::string& command) {
    if (command[0] == '/' || command[0] == '.') {
        if (access(command.c_str(), X_OK) == 0) {
            return command;
        }
        return "";
    }
    
    const char* pathEnv = getenv("PATH");
    if (!pathEnv) return "";
    
    std::string path(pathEnv);
    std::string delimiter = ":";
    size_t pos = 0;
    std::string token;
    
    while ((pos = path.find(delimiter)) != std::string::npos) {
        token = path.substr(0, pos);
        std::string candidatePath = token + "/" + command;
        
        if (access(candidatePath.c_str(), X_OK) == 0) {
            return candidatePath;
        }
        
        path.erase(0, pos + delimiter.length());
    }
    
    std::string candidatePath = path + "/" + command;
    if (access(candidatePath.c_str(), X_OK) == 0) {
        return candidatePath;
    }
    
    return "";
}

