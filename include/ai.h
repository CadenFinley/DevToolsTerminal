#pragma once

#include <curl/curl.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;

class Ai {
 public:
  Ai(const std::string& apiKey, const std::string& assistantType,
     const std::string& initialInstruction);
  Ai(const std::string& apiKey, const std::string& assistantType,
     const std::string& initialInstruction,
     const std::vector<std::string>& userFiles);
  Ai(const std::string& apiKey, const std::string& assistantType,
     const std::string& initialInstruction,
     const std::vector<std::string>& userFiles,
     const std::string& saveDirectory, bool enabled);
  Ai();

  void setAPIKey(const std::string& apiKey);
  std::string getAPIKey() const;
  void setInitialInstruction(const std::string& instruction);
  std::string getInitialInstruction() const;
  void setAssistantType(const std::string& assistantType);
  std::string getAssistantType() const;
  void setMaxPromptLength(int maxPromptLength);
  int getMaxPromptLength() const;
  void setCacheTokens(bool cacheTokens);
  bool getCacheTokens() const;
  void toggleCacheTokens();
  void clearAllCachedTokens();
  std::vector<std::string> getFiles() const;
  std::string getFileContents() const;
  std::vector<std::string> getChatCache() const;
  void setChatCache(const std::vector<std::string>& chatCache);
  void clearChatCache();
  void setDynamicPromptLength(bool dynamicPromptLength);
  bool getDynamicPromptLength() const;
  void toggleDynamicPromptLength();
  void addChatToCache(const std::string& chat);
  std::string getResponseData(const std::string& key) const;
  std::string getLastPromptUsed() const;
  std::string getLastResponseReceived() const;
  void setMaxPromptPrecision(bool maxPromptPrecision);
  bool getMaxPromptPrecision() const;
  void toggleMaxPromptPrecision();
  void setTimeoutFlagSeconds(float timeoutFlagSeconds);
  float getTimeoutFlagSeconds() const;
  void setModel(const std::string& model);
  std::string getModel() const;
  void setDynamicPromptLengthScale(float dynamicPromptLengthScale);
  float getDynamicPromptLengthScale() const;
  void removeFile(const std::string& userFile);
  void clearFiles();
  void setSaveDirectory(const std::string& directory);
  std::string getSaveDirectory() const;
  void setEnabled(bool enabled);
  bool isEnabled() const;

  std::string chatGPT(const std::string& message, bool format);
  std::string forceDirectChatGPT(const std::string& message, bool format);
  int setFiles(const std::vector<std::string>& userFiles);
  int setFile(const std::string& userFile);
  int addFile(const std::string& userFile);
  int addFiles(const std::vector<std::string>& userFiles);
  static bool testAPIKey(const std::string& apiKey);
  void rejectChanges();
  void refreshFiles();

 private:
  void initialize(const std::string& apiKey, const std::string& assistantType,
                  const std::string& initialInstruction,
                  const std::vector<std::string>& userFiles);
  bool isValidConfiguration() const;
  std::string getInvalidConfigurationMessage() const;
  std::string buildPrompt(const std::string& message);
  std::string makeCallToChatGPT(const std::string& message);
  static std::string filterMessage(const std::string& message);
  std::map<std::string, nlohmann::json> parseJSONResponse(
      const std::string& jsonResponse) const;
  std::string extractContentFromJSON(const std::string& jsonResponse) const;
  int processFileContents();
  void processTextFile(const std::string& file, std::string& out);
  void processOtherFile(const std::string& file, std::string& out);
  static std::vector<std::string> extractCodeSnippet(
      const std::string& content);
  std::string processCodeBlocksForCodeInterpreter(const std::string& message);
  static std::string formatMarkdown(const std::string& text);
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                              std::string* userp);
  std::vector<std::string> splitString(const std::string& str, char delimiter);
  std::string sanitizeFileName(const std::string& fileName);
  static void monitorCancellation(std::atomic<bool>& loading,
                                  std::atomic<bool>& requestCancelled);

  std::string USER_API_KEY;
  std::string initialInstruction;
  std::string assistantType;
  std::vector<std::string> files;
  int maxPromptLength = -1;
  bool cacheTokens = false;
  bool maxPromptPrecision = false;
  bool dynamicPromptLength = false;
  float dynamicPromptLengthScale = 5;
  float timeoutFlagSeconds = 300;
  std::string currentModel = "gpt-3.5-turbo";
  std::vector<std::string> lastUsedFiles;
  std::string fileContents;
  std::vector<std::string> chatCache;
  std::string lastPromptUsed;
  std::string lastResponseReceived;
  std::map<std::string, nlohmann::json> responseDataMap;
  std::string saveDirectory;
  bool enabled = true;
  std::atomic<bool> requestInProgress{false};
};