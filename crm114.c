#include "ruby.h"

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"

typedef struct CRM114
{
  CRM114_CONTROLBLOCK *cb;
  CRM114_DATABLOCK *db;
  CRM114_ERR err;
} CRM114;

static VALUE crm_getClasses(VALUE obj);
static VALUE crm_setClasses(VALUE obj, VALUE rb_classes_ary);
static VALUE crm_getDatablockSize(VALUE obj);
static VALUE crm_setDatablockSize(VALUE obj, VALUE rb_size);
static VALUE crm_setRegex(VALUE obj, VALUE regex);
static VALUE crm_setPipeline(VALUE obj);

static VALUE crm_config(VALUE obj);
static VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text);

static VALUE crm_alloc(VALUE klass);
static VALUE crm_init(VALUE obj, VALUE flags);
static void crm_mark(CRM114 *crm);
static void crm_free(CRM114 *crm);

void Init_CRM114()
{
  VALUE crm114_class = rb_define_class("CRM114", rb_cObject);
  rb_define_alloc_func(crm114_class, crm_alloc);
  rb_define_method(crm114_class, "initialize", crm_init, 1);

  // Virtual attrs
  rb_define_method(crm114_class, "classes", crm_getClasses, 0);
  rb_define_method(crm114_class, "classes=", crm_setClasses, 1);
  rb_define_method(crm114_class, "datablock_size", crm_getDatablockSize, 0);
  rb_define_method(crm114_class, "datablock_size=", crm_setDatablockSize, 1);
  rb_define_method(crm114_class, "regex=", crm_setRegex, 1);
  rb_define_method(crm114_class, "pipeline=", crm_setPipeline, 0); //TODO

  // Methods
  rb_define_method(crm114_class, "config", crm_config, 0);
  rb_define_method(crm114_class, "learn_text", crm_learn_text, 2);

  // Constants
  rb_define_const(crm114_class, "OSB", LONG2FIX(CRM114_OSB));
  rb_define_const(crm114_class, "SVM", LONG2FIX(CRM114_SVM));
  rb_define_const(crm114_class, "FSCM", LONG2FIX(CRM114_FSCM));
  rb_define_const(crm114_class, "HYPERSPACE", LONG2FIX(CRM114_HYPERSPACE));
  rb_define_const(crm114_class, "ENTROPY", LONG2FIX(CRM114_ENTROPY));
  rb_define_const(crm114_class, "STRING", LONG2FIX(CRM114_STRING));
  rb_define_const(crm114_class, "UNIQUE", LONG2FIX(CRM114_UNIQUE));
  rb_define_const(crm114_class, "CROSSLINK", LONG2FIX(CRM114_CROSSLINK));  
}

static VALUE crm_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, crm_mark, crm_free, NULL);
}

static void crm_mark(CRM114 *crm)
{
}

static void crm_free(CRM114 *crm)
{
}

/////

static VALUE crm_init(VALUE obj, VALUE flags)
{
  CRM114 *crm = malloc(sizeof(CRM114));
  crm->cb = crm114_new_cb();

  DATA_PTR(obj) = crm;

  crm114_cb_setflags(crm->cb, FIX2LONG(flags));
  crm114_cb_setclassdefaults(crm->cb);

  return Qnil;
}

static VALUE crm_config(VALUE obj)
{
  CRM114 *crm = DATA_PTR(obj);
  
  rb_yield(obj);
  crm114_cb_setblockdefaults(crm->cb);

  return Qnil;
}

static VALUE crm_setClasses(VALUE obj, VALUE rb_classes_ary)
{
  CRM114 *crm = DATA_PTR(obj);

  int length = RARRAY_LEN(rb_classes_ary);
  crm->cb->how_many_classes = length;
  for (int i = 0; i < length; i++) {
    char *name = RSTRING_PTR(rb_ary_entry(rb_classes_ary, i));
    strcpy(crm->cb->class[i].name, name);
  }

  return rb_classes_ary;
}

static VALUE crm_getClasses(VALUE obj)
{
  CRM114 *crm = DATA_PTR(obj);

  int length = crm->cb->how_many_classes;
  VALUE ary = rb_ary_new2(length);
  for (int i = 0; i < length; i++) {
    rb_ary_store(ary, i, rb_str_new_cstr(crm->cb->class[i].name));
  }

  return ary;
}

static VALUE crm_setDatablockSize(VALUE obj, VALUE rb_size)
{
  CRM114 *crm = DATA_PTR(obj);
  crm->cb->datablock_size = FIX2INT(rb_size);
  return rb_size;
}

static VALUE crm_getDatablockSize(VALUE obj)
{
  CRM114 *crm = DATA_PTR(obj);
  return INT2FIX(crm->cb->datablock_size);
}

static VALUE crm_setRegex(VALUE obj, VALUE regex)
{
  CRM114 *crm = DATA_PTR(obj);
  
  char *regex_str = RSTRING_PTR(regex);
  int regex_len = RSTRING_LEN(regex);
  crm114_cb_setregex(crm->cb, regex_str, regex_len);
  
  return regex;
}

static VALUE crm_setPipeline(VALUE obj)
{
  //TODO

  return Qnil;
}

VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text)
{
  CRM114 *crm = DATA_PTR(obj);

  int idx;
  if (TYPE(whichClass) == T_STRING) {
    //get the index of the string
    char *string = RSTRING_PTR(whichClass);
    int length = crm->cb->how_many_classes;
    for (idx = 0; idx < length; idx++) {
      if (strcmp(crm->cb->class[idx].name, string) == 0) {
        break;
      }
    }
  } else {
    idx = FIX2INT(whichClass);
  }

  char *text_str = RSTRING_PTR(text);
  int text_len = RSTRING_LEN(text);

  crm114_learn_text(&(crm->db), idx, text_str, text_len);

  return Qnil;
}
