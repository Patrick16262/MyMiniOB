

#include "in_memory_text_storage.h"

int InMemoryTextStorage::put(const std::string &text)
{
  storage_[next_key] = text;
  return next_key++;
}

const std::string &InMemoryTextStorage::get(int key) { return storage_[key]; }

void InMemoryTextStorage::remove(int key) { storage_.erase(key); }

InMemoryTextStorage g_mem_text;
