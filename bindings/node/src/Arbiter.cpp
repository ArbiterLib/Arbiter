#include <node.h>
#include "Version.h"

namespace ArbiterNodeBindings {

using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports) {
  SemanticVersion::Init(exports);
}

NODE_MODULE(arbiter, InitAll)

} // namespace ArbiterNodeBindings
