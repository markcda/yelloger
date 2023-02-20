/*! MIT License
 *  Copyright (c) 2021 danielblagy, 2023 Mark CDA
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.  */

#pragma once

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <mutex>
#include <ctime>
#include <QString>

/*! @class Yellog
 *  @brief TBD  */
class Yellog {
// Objects:
public:
  enum LogPriority { TracePriority, DebugPriority, InfoPriority, WarnPriority, ErrorPriority, CriticalPriority };

private:
  LogPriority priority = InfoPriority;
  std::mutex log_mutex;
  const char* filepath = 0;
  std::FILE* file = 0;
  /*! @details For timestamp formatting. */
  char buffer[80];
  const char* timestamp_format = "%T  %d-%m-%Y";

// Functions:
public:
  /*! @brief Sets desired priority for the logger (messages with lower priority will not be recorded). 
   *  @note  The default priority is Yellog::InfoPriority.  */
  static void SetPriority(LogPriority new_priority) { get_instance().priority = new_priority; }
  
  /*! @brief Gets the current logger priority. */
  static LogPriority GetPriority() { return get_instance().priority; }

  /*! @brief Enables file output.
   *  @details Logs will be written to /log.txt. If the file doesn't exist, it will create it automatically.  */
  static bool EnableFileOutput()
  {
    Yellog& logger_instance = get_instance();
    logger_instance.filepath = "log.txt";
    return logger_instance.enable_file_output();
  }

  /*! @brief Enables file output.
   *  @details Logs will be written to given filepath. If the file doesn't exist, it will create it automatically.  */
  static bool EnableFileOutput(const char* new_filepath)
  {
    Yellog& logger_instance = get_instance();
    logger_instance.filepath = new_filepath;
    return logger_instance.enable_file_output();
  }

  /*! @brief Returns the current filepath for file logging. */
  static const char* GetFilepath() { return get_instance().filepath; }

  /*! @brief Returns true is file output was enabled and file was successfully opened, false if it wasn't. */
  static bool IsFileOutputEnabled() { return get_instance().file != 0; }

  /*! @brief Sets a log timestamp format.
   *  @details Format follows <ctime> strftime format specification.  */
  static void SetTimestampFormat(const char* new_timestamp_format) {
    get_instance().timestamp_format = new_timestamp_format;
  }

  /*! @brief Gets the current log timestamp format.
   *  @details Format follows <ctime> strftime format specification.  */
  static const char* GetTimestampFormat() { return get_instance().timestamp_format; }

  /*! @brief Logs @a message with log priority level @a Yellog::TracePriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Trace(const QString &message, Args... args) { 
    get_instance().log("[TRACE]     ", TracePriority, message, args...);
  }

  /*! @brief Logs @a message with log priority level @a Yellog::DebugPriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Debug(const QString &message, Args... args) {
    get_instance().log("[DEBUG]     ", DebugPriority, message, args...);
  }

  /*! @brief Logs @a message with log priority level @a Yellog::InfoPriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Info(const QString &message, Args... args) {
    get_instance().log("[INFO ]     ", InfoPriority, message, args...);
  }

  /*! @brief Logs @a message with log priority level @a Yellog::WarnPriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Warn(const QString &message, Args... args) {
    get_instance().log("[WARN ]     ", WarnPriority, message, args...);
  }

  /*! @brief Logs @a message with log priority level @a Yellog::ErrorPriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Error(const QString &message, Args... args) {
    get_instance().log("[ERROR]     ", ErrorPriority, message, args...);
  }

  /*! @brief Logs @a message with log priority level @a Yellog::CriticalPriority.
   *  @details Follows @a printf specification.  */
  template<typename... Args>
  static void Critical(const QString &message, Args... args) {
    get_instance().log("[CRIT ]     ", CriticalPriority, message, args...);
  }

private:
  Yellog() {}
  Yellog(const Yellog&) = delete;
  Yellog& operator= (const Yellog&) = delete;
  ~Yellog() { free_file(); }
  static Yellog& get_instance() { static Yellog instance; return instance; }

  template<typename... Args>
  void log(
    [[maybe_unused]] const char* message_priority_str,
    LogPriority message_priority, 
    [[maybe_unused]] const QString &message,
    [[maybe_unused]] Args... args)
  {
    if (priority <= message_priority) {
      std::time_t current_time = std::time(0);
      std::tm* timestamp = std::localtime(&current_time);

      std::scoped_lock lock(log_mutex);
      std::strftime(buffer, 80, timestamp_format, timestamp);
      std::printf("%s    ", buffer);
      std::printf(message_priority_str);
      std::printf(message.toLocal8Bit().constData(), args...);
      std::printf("\n");

      if (file)
      {
        std::fprintf(file, "%s    ", buffer);
        std::fprintf(file, message_priority_str);
        std::fprintf(file, message.toLocal8Bit().constData(), args...);
        std::fprintf(file, "\n");
      }
    }
  }

  bool enable_file_output() {
    free_file();
    file = std::fopen(filepath, "a");
    if (file == 0) return false;
    return true;
  }

  void free_file() {
    if (not file) return;
    std::fclose(file);
    file = 0;
  }
};
