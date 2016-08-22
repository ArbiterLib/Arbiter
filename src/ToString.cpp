#include "ToString.h"

#include <cassert>

using namespace Arbiter;

void FreeDeleter::operator() (void *ptr) const noexcept
{
  free(ptr);
}

std::unique_ptr<char[]> Arbiter::copyCString (const std::string &str)
{
  size_t length = str.size();
  auto cStr = std::make_unique<char[]>(length + 1);

  memset(cStr.get(), 0, length + 1);
  str.copy(cStr.get(), length);

  return cStr;
}

std::unique_ptr<char[], FreeDeleter> Arbiter::acquireCString (char *str)
{
  return std::unique_ptr<char[], FreeDeleter>(str);
}

std::string Arbiter::copyAcquireCString (char *str)
{
  assert(str);

  auto ptr = acquireCString(str);
  return std::string(ptr.get());
}
