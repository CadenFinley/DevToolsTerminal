#include "openaipromptengine.h"

// Constructors
OpenAIPromptEngine::OpenAIPromptEngine(const std::string& apiKey, const std::string& assistantType, const std::string& initialInstruction) {
    initialize(apiKey, assistantType, initialInstruction, {});
}

OpenAIPromptEngine::OpenAIPromptEngine(const std::string& apiKey, const std::string& assistantType, const std::string& initialInstruction, const std::vector<std::string>& userFiles) {
    initialize(apiKey, assistantType, initialInstruction, userFiles);
}

OpenAIPromptEngine::OpenAIPromptEngine() {}

// Getters and Setters
void OpenAIPromptEngine::setAPIKey(const std::string& apiKey) {
    USER_API_KEY = apiKey;
}

std::string OpenAIPromptEngine::getAPIKey() const {
    return USER_API_KEY;
}

void OpenAIPromptEngine::setInitialInstruction(const std::string& instruction) {
    initialInstruction = instruction;
}

std::string OpenAIPromptEngine::getInitialInstruction() const {
    return initialInstruction;
}

void OpenAIPromptEngine::setAssistantType(const std::string& assistantType) {
    this->assistantType = assistantType;
}

std::string OpenAIPromptEngine::getAssistantType() const {
    return assistantType;
}

void OpenAIPromptEngine::setMaxPromptLength(int maxPromptLength) {
    this->maxPromptLength = maxPromptLength;
}

int OpenAIPromptEngine::getMaxPromptLength() const {
    return maxPromptLength;
}

void OpenAIPromptEngine::setCacheTokens(bool cacheTokens) {
    this->cacheTokens = cacheTokens;
}

bool OpenAIPromptEngine::getCacheTokens() const {
    return cacheTokens;
}

void OpenAIPromptEngine::toggleCacheTokens() {
    cacheTokens = !cacheTokens;
}

void OpenAIPromptEngine::clearAllCachedTokens() {
    makeCallToChatGPT("Clear all cached tokens.");
}

std::vector<std::string> OpenAIPromptEngine::getFiles() const {
    return files;
}

std::string OpenAIPromptEngine::getFileContents() const {
    return fileContents;
}

std::vector<std::string> OpenAIPromptEngine::getChatCache() const {
    return chatCache;
}

void OpenAIPromptEngine::setChatCache(const std::vector<std::string>& chatCache) {
    this->chatCache = chatCache;
}

void OpenAIPromptEngine::clearChatCache() {
    chatCache.clear();
}

void OpenAIPromptEngine::setDynamicPromptLength(bool dynamicPromptLength) {
    this->dynamicPromptLength = dynamicPromptLength;
}

bool OpenAIPromptEngine::getDynamicPromptLength() const {
    return dynamicPromptLength;
}

void OpenAIPromptEngine::toggleDynamicPromptLength() {
    dynamicPromptLength = !dynamicPromptLength;
}

void OpenAIPromptEngine::addChatToCache(const std::string& chat) {
    chatCache.push_back(chat);
}

std::string OpenAIPromptEngine::getResponseData(const std::string& key) const {
    auto it = responseDataMap.find(key);
    if (it != responseDataMap.end()) {
        return it->second.dump();
    }
    return "";
}

std::string OpenAIPromptEngine::getLastPromptUsed() const {
    return lastPromptUsed;
}

void OpenAIPromptEngine::removeFile(const std::string& userFile) {
    files.erase(std::remove(files.begin(), files.end(), userFile), files.end());
}

void OpenAIPromptEngine::clearFiles() {
    files.clear();
}

std::string OpenAIPromptEngine::getLastResponseReceived() const {
    return lastResponseReceived;
}

void OpenAIPromptEngine::setMaxPromptPrecision(bool maxPromptPrecision) {
    this->maxPromptPrecision = maxPromptPrecision;
}

bool OpenAIPromptEngine::getMaxPromptPrecision() const {
    return maxPromptPrecision;
}

void OpenAIPromptEngine::toggleMaxPromptPrecision() {
    maxPromptPrecision = !maxPromptPrecision;
}

void OpenAIPromptEngine::setTimeoutFlagSeconds(float timeoutFlagSeconds) {
    this->timeoutFlagSeconds = timeoutFlagSeconds;
}

float OpenAIPromptEngine::getTimeoutFlagSeconds() const {
    return timeoutFlagSeconds;
}

void OpenAIPromptEngine::setModel(const std::string& model) {
    currentModel = model;
}

std::string OpenAIPromptEngine::getModel() const {
    return currentModel;
}

void OpenAIPromptEngine::setDynamicPromptLengthScale(float dynamicPromptLengthScale) {
    this->dynamicPromptLengthScale = dynamicPromptLengthScale;
}

float OpenAIPromptEngine::getDynamicPromptLengthScale() const {
    return dynamicPromptLengthScale;
}

// Public Methods
std::string OpenAIPromptEngine::chatGPT(const std::string& message, bool format) {
    if (!isValidConfiguration()) {
        return getInvalidConfigurationMessage();
    }

    std::string response = makeCallToChatGPT(buildPrompt(message));
    
    if (maxPromptPrecision && response.length() > maxPromptLength) {
        return chatGPT(message, format);
    }

    if (!response.empty()) {
        chatCache.push_back("User: " + message);
        chatCache.push_back("AI: " + response);
    }

    if (assistantType == "code-interpreter" && !response.empty()) {
        response += processCodeBlocksForCodeInterpreter(response);
    }

    return format ? formatMarkdown(response) : response;
}

std::string OpenAIPromptEngine::forceDirectChatGPT(const std::string& message, bool format) {
    std::string response = makeCallToChatGPT(message);
    return format ? formatMarkdown(response) : response;
}

int OpenAIPromptEngine::setFiles(const std::vector<std::string>& userFiles) {
    this->files = userFiles;
    if (lastUsedFiles != files) {
        lastUsedFiles = files;
        return processFileContents();
    }
    return 0;
}

int OpenAIPromptEngine::setFile(const std::string& userFile) {
    this->files = {userFile};
    if (lastUsedFiles != files) {
        lastUsedFiles = files;
        return processFileContents();
    }
    return 0;
}

int OpenAIPromptEngine::addFile(const std::string& userFile) {
    if (files.empty()) {
        files = {userFile};
    } else {
        files.push_back(userFile);
    }
    if (lastUsedFiles != files) {
        lastUsedFiles = files;
        return processFileContents();
    }
    return 0;
}

int OpenAIPromptEngine::addFiles(const std::vector<std::string>& userFiles) {
    if (files.empty()) {
        files = userFiles;
    } else {
        files.insert(files.end(), userFiles.begin(), userFiles.end());
    }
    if (lastUsedFiles != files) {
        lastUsedFiles = files;
        return processFileContents();
    }
    return 0;
}

// Private Methods
void OpenAIPromptEngine::initialize(const std::string& apiKey, const std::string& assistantType, const std::string& initialInstruction, const std::vector<std::string>& userFiles) {
    USER_API_KEY = apiKey;
    this->assistantType = assistantType;
    this->initialInstruction = initialInstruction;
    this->files = userFiles;
}

bool OpenAIPromptEngine::isValidConfiguration() const {
    bool validAssistantType = assistantType == "chat" || 
                             assistantType == "file-search" || 
                             assistantType == "code-interpreter";
    return !USER_API_KEY.empty() && !initialInstruction.empty() && !assistantType.empty() && validAssistantType;
}

std::string OpenAIPromptEngine::getInvalidConfigurationMessage() const {
    if (USER_API_KEY.empty()) {
        return "API key not set.";
    }
    if (initialInstruction.empty()) {
        return "Initial instruction not set.";
    }
    if (assistantType.empty()) {
        return "Assistant type not set.";
    }
    return "Invalid configuration.";
}

std::string OpenAIPromptEngine::buildPrompt(const std::string& message) const {
    std::stringstream prompt;
    if(assistantType != "code-interpreter"){
        prompt << initialInstruction;
    } else {
        prompt << "Please only make the edits that the user requests, and nothing else even if there are other issues. Please return the edits in full in the response.";
    }
    if (maxPromptLength != -1 && assistantType != "code-interpreter") {
        int promptLength = dynamicPromptLength ? std::max(static_cast<int>(message.length() * dynamicPromptLengthScale), 100) : maxPromptLength;
        prompt << " Please keep the response length under " << promptLength << " characters.";
    }

    if (!chatCache.empty()) {
        prompt << " This is the chat history between you and the user: [ ";
        for (const auto& chat : chatCache) {
            prompt << chat << " ";
        }
        prompt << "] This is the latest message from the user: [" << message << "] ";
    } else {
        prompt << " This is the first message from the user: [" << message << "] ";
    }

    if (assistantType == "file-search") {
        prompt << " This is the contents of the provided files from the user: [ " << fileContents << " ]";
        if (cacheTokens) {
            prompt << " Please keep this content of these files in cached tokens.";
        }
    }

    if (assistantType == "code-interpreter") {
        prompt << " User Files: [ " << fileContents << " ]";
    }

    return prompt.str();
}

std::string OpenAIPromptEngine::makeCallToChatGPT(const std::string& message) {
    auto start = std::chrono::steady_clock::now();
    std::string url = "https://api.openai.com/v1/chat/completions";
    std::string filteredMessage = filterMessage(message);
    lastPromptUsed = filteredMessage;

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + USER_API_KEY).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    nlohmann::json requestBody = {
        {"model", currentModel},
        {"messages", {{
            {"role", "user"},
            {"content", filteredMessage}
        }}}
    };

    std::string requestBodyStr = requestBody.dump();
    std::string responseData;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBodyStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeoutFlagSeconds));

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    try {
        auto jsonResponse = nlohmann::json::parse(responseData);
        lastResponseReceived = jsonResponse["choices"][0]["message"]["content"];
        
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        responseDataMap["processing_time_ms"] = elapsed.count() * 1000;
        responseDataMap["total_tokens"] = jsonResponse["usage"]["total_tokens"];
        
        if (!files.empty() && assistantType == "file-search") {
            responseDataMap["file_names"] = files;
        }
        
        responseDataMap["assistant_type"] = assistantType;
        responseDataMap["initial_instruction"] = initialInstruction;
        responseDataMap["received_message_length"] = lastResponseReceived.length();
        
        return lastResponseReceived;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return "";
    }
}

// Static callback function for CURL
size_t OpenAIPromptEngine::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string OpenAIPromptEngine::filterMessage(const std::string& message) {
    std::string filtered = message;
    filtered.erase(std::remove_if(filtered.begin(), filtered.end(),
        [](char c) { 
            return !(std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.' || c == '~');
        }), filtered.end());
    
    // Replace newlines with spaces
    std::replace(filtered.begin(), filtered.end(), '\n', ' ');
    return filtered;
}

std::map<std::string, nlohmann::json> OpenAIPromptEngine::parseJSONResponse(const std::string& jsonResponse) const {
    std::map<std::string, nlohmann::json> responseData;
    try {
        nlohmann::json jsonObject = nlohmann::json::parse(jsonResponse);
        for (auto& [key, value] : jsonObject.items()) {
            responseData[key] = value;
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
    }
    return responseData;
}

std::string OpenAIPromptEngine::extractContentFromJSON(const std::string& jsonResponse) const {
    try {
        nlohmann::json jsonObject = nlohmann::json::parse(jsonResponse);
        return jsonObject["choices"][0]["message"]["content"];
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to extract content from JSON response: " << e.what() << std::endl;
        return "";
    }
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int OpenAIPromptEngine::processFileContents() {
    if (files.empty()) {
        return 0;
    }

    std::stringstream out;
    for (const auto& file : files) {
        out << "File: " << file << "\n";
        if (endsWith(file, ".txt")) {
            std::string content;
            processTextFile(file, content);
            out << content;
        } else {
            std::string content;
            processOtherFile(file, content);
            out << content;
        }
    }
    fileContents = out.str();
    return fileContents.length();
}

std::vector<std::string> OpenAIPromptEngine::extractCodeSnippet(const std::string& content) {
    std::vector<std::string> codeSnippets;
    std::stringstream codeSnippet;
    bool inCodeBlock = false;
    
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.substr(0, 3) == "```") {
            if (inCodeBlock) {
                codeSnippets.push_back(codeSnippet.str());
                codeSnippet.str("");
                inCodeBlock = false;
            } else {
                inCodeBlock = true;
            }
        } else if (inCodeBlock) {
            codeSnippet << line << '\n';
        }
    }
    
    return codeSnippets;
}

std::map<std::string, std::vector<std::string>> originalFileContents;

std::string OpenAIPromptEngine::processCodeBlocksForCodeInterpreter(const std::string& message) {
    std::vector<std::string> codeBlocks = extractCodeSnippet(message);
    if (codeBlocks.empty()) {
        return "";
    }

    size_t i = 0;
    std::stringstream changesSummary;
    for (const auto& codeBlock : codeBlocks) {
        if (i >= files.size()) break;
        
        const std::string& fileToChange = files[i];
        try {
            std::vector<std::string> originalLines;
            std::vector<std::string> newLines;
            std::vector<std::string> updatedLines;
            
            // Read original file
            std::ifstream inFile(fileToChange);
            std::string line;
            while (std::getline(inFile, line)) {
                originalLines.push_back(line);
            }
            inFile.close();
            
            // Store original file contents
            originalFileContents[fileToChange] = originalLines;
            
            // Split new code into lines
            std::stringstream ss(codeBlock);
            while (std::getline(ss, line)) {
                newLines.push_back(line);
            }
            
            // Compare and update lines
            for (size_t j = 0; j < originalLines.size() || j < newLines.size(); j++) {
                if (j < newLines.size() && (j >= originalLines.size() || originalLines[j] != newLines[j])) {
                    updatedLines.push_back(newLines[j]);
                } else {
                    updatedLines.push_back(originalLines[j]);
                }
            }

                        // Write changes back to file
                        std::ofstream outFile(fileToChange);
                        for (const auto& updatedLine : updatedLines) {
                            outFile << updatedLine << "\n";
                        }
                        outFile.close();

            // Write changes summary using git-like format with color highlighting
            changesSummary << "\033[1;34mdiff --git a/" << fileToChange << " b/" << fileToChange << "\033[0m\n";
            changesSummary << "\033[1;31m--- a/" << fileToChange << "\033[0m\n";
            changesSummary << "\033[1;32m+++ b/" << fileToChange << "\033[0m\n";
            for (size_t j = 0; j < updatedLines.size(); j++) {
                if (j >= originalLines.size()) {
                    changesSummary << "\033[1;32m+ " << updatedLines[j] << "\033[0m\n";
                } else if (originalLines[j] != updatedLines[j]) {
                    changesSummary << "\033[1;31m- " << originalLines[j] << "\033[0m\n";
                    changesSummary << "\033[1;32m+ " << updatedLines[j] << "\033[0m\n";
                }
            }
            for (size_t j = updatedLines.size(); j < originalLines.size(); j++) {
                changesSummary << "\033[1;31m- " << originalLines[j] << "\033[0m\n";
            }
        } catch (const std::exception& e) {
            return "\nFailed to apply changes to file: " + fileToChange;
        }
        i++;
    }
    
    return "\nSuccessfully applied changes to files.\nChanges Summary:\n" + changesSummary.str();
}

void OpenAIPromptEngine::rejectChanges() {
    for (const auto& [file, originalLines] : originalFileContents) {
        std::ofstream outFile(file);
        for (const auto& line : originalLines) {
            outFile << line << "\n";
        }
        outFile.close();
    }
    originalFileContents.clear();
}

void OpenAIPromptEngine::processTextFile(const std::string& file, std::string& out) {
    std::ifstream inFile(file);
    if (inFile.is_open()) {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        out += buffer.str() + "\n";
        inFile.close();
    } else {
        std::cerr << "Failed to read text file: " << file << std::endl;
    }
}

void OpenAIPromptEngine::processOtherFile(const std::string& file, std::string& out) {
    std::ifstream inFile(file);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            out += line + "\n";
        }
        inFile.close();
    } else {
        std::cerr << "Failed to read file: " << file << std::endl;
    }
}

std::string OpenAIPromptEngine::formatMarkdown(const std::string& text) {
    std::string formatted = text;
    bool inCodeBlock = false;
    std::istringstream stream(text);
    std::string line;
    std::stringstream result;

    while (std::getline(stream, line)) {
        if (line.substr(0, 3) == "```") {
            inCodeBlock = !inCodeBlock;
        } else if (line.substr(0, 2) == "**" && line.substr(line.length() - 2) == "**") {
            result << line.substr(2, line.length() - 4) << "\n";
        } else if (line.substr(0, 1) == "*" && line.substr(line.length() - 1) == "*") {
            result << line.substr(1, line.length() - 2) << "\n";
        } else if (line.substr(0, 2) == "# ") {
            result << line.substr(2) << "\n";
        } else {
            result << line << "\n";
        }
    }

    return result.str();
}

bool OpenAIPromptEngine::testAPIKey(const std::string& apiKey) {
    std::string url = "https://api.openai.com/v1/engines";
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We only need the headers

    CURLcode res = curl_easy_perform(curl);
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && response_code == 200);
}
