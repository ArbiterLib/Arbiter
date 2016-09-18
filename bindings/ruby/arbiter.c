#include <ruby.h>

#include <arbiter/Dependency.h>
#include <arbiter/Requirement.h>
#include <arbiter/Types.h>
#include <arbiter/Version.h>

static bool value_equal(const void *first, const void *second) {
  VALUE a = (VALUE)first, b = (VALUE)second;
  return rb_equal(a, b) == Qtrue;
}

static bool value_less_than(const void *first, const void *second) {
  VALUE a = (VALUE)first, b = (VALUE)second;
  return rb_funcall(a, rb_intern("<"), 1, b) == Qtrue;
}

static size_t value_hash(const void *data) {
  VALUE hash = rb_funcall((VALUE)data, rb_intern("hash"), 0);
  return FIX2ULONG(hash);
}

static char *value_create_description(const void *data) {
  VALUE value = (VALUE)data;
  char *description = StringValueCStr(value);
  return strdup(description);
}

static void project_identifier_mark(ArbiterUserValue *user_value) {
  rb_gc_mark((VALUE)user_value->data);
}

static VALUE project_identifier_allocate(VALUE klass) {
  return Data_Wrap_Struct(klass, project_identifier_mark, ArbiterFree, NULL);
}

static VALUE project_identifier_initialize(VALUE self, VALUE value) {
  ArbiterUserValue user_value = {
    .data = (void *)value,
    .equalTo = value_equal,
    .lessThan = value_less_than,
    .hash = value_hash,
    .createDescription = value_create_description,
    .destructor = NULL,
  };

  DATA_PTR(self) = ArbiterCreateProjectIdentifier(user_value);

  return self;
}

static VALUE semantic_version_allocate(VALUE klass) {
  return Data_Wrap_Struct(klass, NULL, ArbiterFree, NULL);
}

static VALUE semantic_version_initialize(int argc, VALUE *argv, VALUE self) {
  // SemanticVersion.new("1.2.3.pre.meta")
  if (argc == 1) {
    Check_Type(argv[0], T_STRING);
    const char *string = StringValueCStr(argv[0]);

    DATA_PTR(self) = ArbiterCreateSemanticVersionFromString(string);
    return self;
  }

  if (argc < 3 || argc > 5) {
    rb_error_arity(argc, 3, 5);
  }

  // SemanticVersion.new(1, 2, 3)
  // SemanticVersion.new(1, 2, 3, "pre")
  // SemanticVersion.new(1, 2, 3, "pre", "meta")
  Check_Type(argv[0], T_FIXNUM);
  Check_Type(argv[1], T_FIXNUM);
  Check_Type(argv[2], T_FIXNUM);
  unsigned major = FIX2UINT(argv[0]);
  unsigned minor = FIX2UINT(argv[1]);
  unsigned patch = FIX2UINT(argv[2]);

  const char *prereleaseVersion = NULL;
  if (argc >= 4) {
    Check_Type(argv[3], T_STRING);
    prereleaseVersion = StringValueCStr(argv[3]);
  }
  const char *buildMetadata = NULL;
  if (argc >= 5) {
    Check_Type(argv[4], T_STRING);
    buildMetadata = StringValueCStr(argv[4]);
  }

  DATA_PTR(self) = ArbiterCreateSemanticVersion(major, minor, patch, prereleaseVersion, buildMetadata);
  return self;
}

static VALUE requirement_any(VALUE klass) {
  ArbiterRequirement *requirement = ArbiterCreateRequirementAny();
  return Data_Wrap_Struct(klass, NULL, ArbiterFree, requirement);
}

static VALUE requirement_at_least(VALUE klass, VALUE version) {
  ArbiterSemanticVersion *semantic_version = DATA_PTR(version);
  ArbiterRequirement *requirement = ArbiterCreateRequirementAtLeast(semantic_version);
  return Data_Wrap_Struct(klass, NULL, ArbiterFree, requirement);
}

static VALUE requirement_compatible_with(VALUE klass, VALUE version) {
  ArbiterSemanticVersion *semantic_version = DATA_PTR(version);
  ArbiterRequirement *requirement = ArbiterCreateRequirementCompatibleWith(semantic_version, ArbiterRequirementStrictnessAllowVersionZeroPatches);
  return Data_Wrap_Struct(klass, NULL, ArbiterFree, requirement);
}

static VALUE requirement_exactly(VALUE klass, VALUE version) {
  ArbiterSemanticVersion *semantic_version = DATA_PTR(version);
  ArbiterRequirement *requirement = ArbiterCreateRequirementExactly(semantic_version);
  return Data_Wrap_Struct(klass, NULL, ArbiterFree, requirement);
}

void Init_arbiter() {
  VALUE mArbiter = rb_define_module("Arbiter");

  VALUE cProjectIdentifier = rb_define_class_under(mArbiter, "ProjectIdentifier", rb_cObject);
  rb_define_alloc_func(cProjectIdentifier, project_identifier_allocate);
  rb_define_method(cProjectIdentifier, "initialize", project_identifier_initialize, 1);

  VALUE cSemanticVersion = rb_define_class_under(mArbiter, "SemanticVersion", rb_cObject);
  rb_define_alloc_func(cSemanticVersion, semantic_version_allocate);
  rb_define_method(cSemanticVersion, "initialize", semantic_version_initialize, -1);

  VALUE cRequirement = rb_define_class_under(mArbiter, "Requirement", rb_cObject);
  rb_define_singleton_method(cRequirement, "any", requirement_any, 0);
  rb_define_singleton_method(cRequirement, "at_least", requirement_at_least, 1);
  rb_define_singleton_method(cRequirement, "compatible_with", requirement_compatible_with, 1);
  rb_define_singleton_method(cRequirement, "exactly", requirement_compatible_with, 1);
}
