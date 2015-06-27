#include "ruby.h"

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"

typedef struct ControlBlock
{
  CRM114_CONTROLBLOCK *c_controlBlock;
} ControlBlock;


static void cb_mark(ControlBlock *crm)
{
}

static void cb_free(ControlBlock *crm)
{
}

static VALUE cb_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, cb_mark, cb_free, NULL);
}

static VALUE cb_init(VALUE obj, VALUE rb_flags)
{
  ControlBlock *cb = malloc(sizeof(ControlBlock));
  cb->c_controlBlock = crm114_new_cb();

  int flags = FIX2INT(rb_flags);

  crm114_cb_setflags(cb->c_controlBlock, flags);
  crm114_cb_setclassdefaults(cb->c_controlBlock);

  DATA_PTR(obj) = cb;
  return Qnil;
}

static VALUE cb_setClasses(VALUE obj, VALUE rb_classes_ary)
{
  ControlBlock *cb = (ControlBlock *)DATA_PTR(obj);

  int length = RARRAY_LEN(rb_classes_ary);
  for (int i = 0; i < length; i++) {
    char *name = RSTRING_PTR(rb_ary_entry(rb_classes_ary, i));
    strcpy(cb->c_controlBlock->class[i].name, name);
  }
  cb->c_controlBlock->how_many_classes = length;

  return rb_classes_ary;
}

static VALUE cb_getClasses(VALUE obj)
{
  ControlBlock *cb = (ControlBlock *)DATA_PTR(obj);

  int length = cb->c_controlBlock->how_many_classes;
  VALUE ary = rb_ary_new2(length);
  for (int i = 0; i < length; i++) {
    rb_ary_store(ary, i, rb_str_new_cstr(cb->c_controlBlock->class[i].name));
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
}
