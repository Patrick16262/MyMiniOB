#pragma once

#include <string>
#include <unordered_map>

/**
 * @brief 用于存放超长数据
 * 
 */

class InMemoryTextStorage
{
  public:
  /**
   * @brief 用于存储超长数据
   * 
   * @param text 要存储的text 
   * @return int 返回的键
   */
  int put(const std::string& text);

  const std::string &get(int key);

  void remove(int key);

  private:
  int next_key = 0;
  std::unordered_map<int, std::string> storage_;
};

extern InMemoryTextStorage g_mem_text;