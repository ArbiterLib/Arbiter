#ifndef VERSION_H
#define VERSION_H

#include <node.h>
#include <node_object_wrap.h>
#include <arbiter/Version.h>

namespace ArbiterNodeBindings {

using v8::Maybe;

class SemanticVersion : public node::ObjectWrap {
  public:
    static void Init(v8::Isolate* isolate);
    static void Create(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void CreateFromString(const v8::FunctionCallbackInfo<v8::Value>& args);

  private:
    explicit SemanticVersion(unsigned major, unsigned minor, unsigned patch, Maybe<char *> prereleaseVersion, Maybe<char *> buildMetadata);
    explicit SemanticVersion(const char *string);
    ~SemanticVersion();

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetMajorVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetMinorVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetPatchVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetPrereleaseVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetBuildMetadata(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Persistent<v8::Function> constructor;
    ArbiterSemanticVersion *_semanticVersion;
};

} // namespace ArbiterNodeBindings

#endif
