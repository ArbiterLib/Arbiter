#ifndef VERSION_H
#define VERSION_H

#include <node.h>
#include <node_object_wrap.h>
#include <arbiter/Version.h>

namespace ArbiterNodeBindings {

using v8::Maybe;

class SemanticVersion : public node::ObjectWrap {
  public:
    static void Init(v8::Local<v8::Object> exports);

  private:
    explicit SemanticVersion(unsigned major, unsigned minor, unsigned patch, Maybe<char *> prereleaseVersion, Maybe<char *> buildMetadata);
    ~SemanticVersion();

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetMajorVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetMinorVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetPatchVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetPrereleaseVersion(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetBuildMetadata(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Persistent<v8::Function> constructor;
    const ArbiterSemanticVersion *_semanticVersion;
};

} // namespace ArbiterNodeBindings

#endif
