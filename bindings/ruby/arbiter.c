#include <ruby.h>

#include <arbiter/Dependency.h>
#include <arbiter/Requirement.h>
#include <arbiter/Types.h>
#include <arbiter/Version.h>

static VALUE cDependency;

static bool value_equal(const void *first, const void *second) {
  VALUE a = *(VALUE *)first, b = *(VALUE *)second;
  return rb_equal(a, b) == Qtrue;
}

static bool value_less_than(const void *first, const void *second) {
  VALUE a = *(VALUE *)first, b = *(VALUE *)second;
  return rb_funcall(a, rb_intern("<"), 1, b) == Qtrue;
}

static size_t value_hash(const void *data) {
  VALUE value = *(VALUE *)data;
  VALUE hash = rb_funcall(value, rb_intern("hash"), 0);
  return FIX2ULONG(hash);
}

static char *value_create_description(const void *data) {
  VALUE value = *(VALUE *)data;
  char *description = StringValueCStr(value);
  return strdup(description);
}

static void value_free(void *data) {
  VALUE *ptr = (VALUE *)data;
  *ptr = Qnil;
}

typedef struct {
  VALUE user_value;
  ArbiterProjectIdentifier *project_identifier;
} ProjectIdentifierData;

static void project_identifier_gc_mark(ProjectIdentifierData *data) {
  rb_gc_mark(data->user_value);
}

static void project_identifier_free(ProjectIdentifierData *data) {
  ArbiterFree(data->project_identifier);
  data->user_value = Qnil;
}

static VALUE project_identifier_allocate(VALUE klass) {
  ProjectIdentifierData *data;
  return Data_Make_Struct(klass, ProjectIdentifierData, project_identifier_gc_mark, project_identifier_free, data);
}

static VALUE project_identifier_initialize(VALUE self, VALUE value) {
  ProjectIdentifierData *data;
  Data_Get_Struct(self, ProjectIdentifierData, data);

  data->user_value = value;
  data->project_identifier = ArbiterCreateProjectIdentifier((ArbiterUserValue){
    .data = (void *)&data->user_value,
    .equalTo = value_equal,
    .lessThan = value_less_than,
    .hash = value_hash,
    .createDescription = value_create_description,
    .destructor = value_free,
  });

  return self;
}

static VALUE project_identifier_create_dependency(VALUE self, VALUE requirement) {
  ProjectIdentifierData *data;
  Data_Get_Struct(self, ProjectIdentifierData, data);

  ArbiterDependency *dependency = ArbiterCreateDependency(data->project_identifier, DATA_PTR(requirement));
  return Data_Wrap_Struct(cDependency, NULL, ArbiterFree, dependency);
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
  rb_define_method(cProjectIdentifier, "create_dependency", project_identifier_create_dependency, 1);

  VALUE cSemanticVersion = rb_define_class_under(mArbiter, "SemanticVersion", rb_cObject);
  rb_define_alloc_func(cSemanticVersion, semantic_version_allocate);
  rb_define_method(cSemanticVersion, "initialize", semantic_version_initialize, -1);

  VALUE cRequirement = rb_define_class_under(mArbiter, "Requirement", rb_cObject);
  rb_define_singleton_method(cRequirement, "any", requirement_any, 0);
  rb_define_singleton_method(cRequirement, "at_least", requirement_at_least, 1);
  rb_define_singleton_method(cRequirement, "compatible_with", requirement_compatible_with, 1);
  rb_define_singleton_method(cRequirement, "exactly", requirement_compatible_with, 1);

  cDependency = rb_define_class_under(mArbiter, "Dependency", rb_cObject);
}
