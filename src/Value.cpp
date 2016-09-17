#include "Value.h"

namespace {

void noOpDestructor (void *)
{}

} // namespace

namespace Arbiter {

std::shared_ptr<void> shareUserContext (ArbiterUserContext context)
{
  return std::shared_ptr<void>(context.data, (context.destructor ? context.destructor : &noOpDestructor));
}

} // namespace Arbiter
