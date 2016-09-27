#include <node.h>
#include "Version.h"

namespace ArbiterNodeBindings {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Value;

void CreateSemanticVersion(const FunctionCallbackInfo<Value>& args) {
  SemanticVersion::Create(args);
}

void InitAll(Local<Object> exports, Local<Object> module) {
  SemanticVersion::Init(exports->GetIsolate());
  NODE_SET_METHOD(exports, "createSemanticVersion", CreateSemanticVersion);
}

NODE_MODULE(arbiter, InitAll)

} // namespace ArbiterNodeBindings
