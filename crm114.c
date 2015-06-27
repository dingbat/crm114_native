#include "ruby.h"

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"


static void cb_mark(CRM114_CONTROLBLOCK *crm)
{
}

static void cb_free(CRM114_CONTROLBLOCK *crm)
{
}

static VALUE cb_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, cb_mark, cb_free, NULL);
}

static VALUE cb_init(VALUE obj, VALUE rb_flags)
{
  CRM114_CONTROLBLOCK *cb = crm114_new_cb();

  int flags = FIX2INT(rb_flags);

  crm114_cb_setflags(cb, flags);
  crm114_cb_setclassdefaults(cb);

  DATA_PTR(obj) = cb;
  return Qnil;
}

static VALUE cb_setClasses(VALUE obj, VALUE rb_classes_ary)
{
  CRM114_CONTROLBLOCK *cb = DATA_PTR(obj);

  int length = RARRAY_LEN(rb_classes_ary);
  for (int i = 0; i < length; i++) {
    char *name = RSTRING_PTR(rb_ary_entry(rb_classes_ary, i));
    strcpy(cb->class[i].name, name);
  }
  cb->how_many_classes = length;

  return rb_classes_ary;
}

static VALUE cb_getClasses(VALUE obj)
{
  CRM114_CONTROLBLOCK *cb = DATA_PTR(obj);

  int length = cb->how_many_classes;
  VALUE ary = rb_ary_new2(length);
  for (int i = 0; i < length; i++) {
    rb_ary_store(ary, i, rb_str_new_cstr(cb->class[i].name));
  }

  return ary;
}

void Init_CRM114()
{
  VALUE rb_mCRM114 = rb_define_module("CRM114");
  VALUE rb_controlBlock = rb_define_class_under(rb_mCRM114, "ControlBlock", rb_cObject);
  rb_define_alloc_func(rb_controlBlock, cb_alloc);
  rb_define_method(rb_controlBlock, "initialize", cb_init, 1);
  rb_define_method(rb_controlBlock, "classes=", cb_setClasses, 1);
  rb_define_method(rb_controlBlock, "classes", cb_getClasses, 0);

  // Constants
  VALUE rb_classifier_flags = rb_define_module_under(rb_mCRM114, "Classifier");
  rb_define_const(rb_classifier_flags, "OSB", INT2FIX(CRM114_OSB));
  rb_define_const(rb_classifier_flags, "SVM", INT2FIX(CRM114_SVM));
  rb_define_const(rb_classifier_flags, "FSCM", INT2FIX(CRM114_FSCM));
  rb_define_const(rb_classifier_flags, "HYPERSPACE", INT2FIX(CRM114_HYPERSPACE));
  rb_define_const(rb_classifier_flags, "ENTROPY", INT2FIX(CRM114_ENTROPY));
  rb_define_const(rb_classifier_flags, "STRING", INT2FIX(CRM114_STRING));
  rb_define_const(rb_classifier_flags, "UNIQUE", INT2FIX(CRM114_UNIQUE));
  rb_define_const(rb_classifier_flags, "CROSSLINK", INT2FIX(CRM114_CROSSLINK));
}
