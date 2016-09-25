#include <arbiter/Version.h>
#include "Version.h"

namespace ArbiterNodeBindings {

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

Persistent<Function> SemanticVersion::constructor;

SemanticVersion::SemanticVersion(unsigned major, unsigned minor, unsigned patch, const char *prereleaseVersion, const char *buildMetadata) {
  _semanticVersion = ArbiterCreateSemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
}

SemanticVersion::~SemanticVersion() {
}

void SemanticVersion::Init(Local<Object> exports) {
  Isolate *isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "SemanticVersion"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getMajorVersion", GetMajorVersion);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getMinorVersion", GetMinorVersion);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getPatchVersion", GetPatchVersion);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getPrereleaseVersion", GetPrereleaseVersion);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBuildMetadata", GetBuildMetadata);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "SemanticVersion"),
               tpl->GetFunction());
}

void SemanticVersion::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new SemanticVersion(...)`
    unsigned major = args[0]->Uint32Value();
    unsigned minor = args[1]->Uint32Value();
    unsigned patch = args[2]->Uint32Value();
    const char *prereleaseVersion = *String::Utf8Value(args[3]->ToString());
    const char *buildMetadata = *String::Utf8Value(args[4]->ToString());
    SemanticVersion *obj = new SemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `SemanticVersion(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> result = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(result);
  }
}

void SemanticVersion::GetMajorVersion(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SemanticVersion *obj = ObjectWrap::Unwrap<SemanticVersion>(args.Holder());
  unsigned major = ArbiterGetMajorVersion(obj->_semanticVersion);
  args.GetReturnValue().Set(Number::New(isolate, major));
}

void SemanticVersion::GetMinorVersion(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SemanticVersion *obj = ObjectWrap::Unwrap<SemanticVersion>(args.Holder());
  unsigned minor = ArbiterGetMinorVersion(obj->_semanticVersion);
  args.GetReturnValue().Set(Number::New(isolate, minor));
}

void SemanticVersion::GetPatchVersion(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SemanticVersion *obj = ObjectWrap::Unwrap<SemanticVersion>(args.Holder());
  unsigned patch = ArbiterGetPatchVersion(obj->_semanticVersion);
  args.GetReturnValue().Set(Number::New(isolate, patch));
}

void SemanticVersion::GetPrereleaseVersion(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SemanticVersion *obj = ObjectWrap::Unwrap<SemanticVersion>(args.Holder());
  const char *prereleaseVersion = ArbiterGetPrereleaseVersion(obj->_semanticVersion);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, prereleaseVersion));
}

void SemanticVersion::GetBuildMetadata(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SemanticVersion *obj = ObjectWrap::Unwrap<SemanticVersion>(args.Holder());
  const char *buildMetadata = ArbiterGetBuildMetadata(obj->_semanticVersion);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, buildMetadata));
}

} // namespace ArbiterNodeBindings
