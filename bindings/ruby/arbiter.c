#include <ruby.h>
#include <arbiter/Dependency.h>
#include <arbiter/Types.h>

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

void Init_arbiter() {
  VALUE mArbiter = rb_define_module("Arbiter");

  VALUE cProjectIdentifier = rb_define_class_under(mArbiter, "ProjectIdentifier", rb_cObject);
  rb_define_alloc_func(cProjectIdentifier, project_identifier_allocate);
  rb_define_method(cProjectIdentifier, "initialize", project_identifier_initialize, 1);
}
