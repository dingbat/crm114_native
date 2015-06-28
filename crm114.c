#include "ruby.h"

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"

typedef struct Classifier
{
  CRM114_CONTROLBLOCK *cb;
  CRM114_DATABLOCK *db;
  CRM114_ERR err;
} Classifier;

static VALUE config_init(VALUE obj, VALUE classifier);
static VALUE config_setClasses(VALUE obj, VALUE rb_classes_ary);
static VALUE config_setDatablockSize(VALUE obj, VALUE rb_size);
static VALUE config_setRegex(VALUE obj, VALUE regex);
static VALUE config_setPipeline(VALUE obj);

static VALUE crm_config(VALUE obj);
static VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text);
static VALUE crm_getClasses(VALUE obj);
static VALUE crm_getDatablockSize(VALUE obj);

static VALUE crm_alloc(VALUE klass);
static VALUE crm_init(VALUE obj, VALUE flags);
static void crm_mark(Classifier *crm);
static void crm_free(Classifier *crm);

// Config class for static access
static VALUE ConfigClass;

void Init_CRM114()
{
  VALUE crm114_module = rb_define_module("CRM114");
  VALUE classifier_class = rb_define_class_under(crm114_module, "Classifier", rb_cObject);
  rb_define_alloc_func(classifier_class, crm_alloc);
  rb_define_method(classifier_class, "initialize", crm_init, 1);

  // Virtual attrs
  rb_define_method(classifier_class, "classes", crm_getClasses, 0);
  rb_define_method(classifier_class, "datablock_size", crm_getDatablockSize, 0);

  // Methods
  rb_define_method(classifier_class, "config", crm_config, 0);
  rb_define_method(classifier_class, "learn_text", crm_learn_text, 2);

  // Constants
  rb_define_const(classifier_class, "OSB", LONG2FIX(CRM114_OSB));
  rb_define_const(classifier_class, "SVM", LONG2FIX(CRM114_SVM));
  rb_define_const(classifier_class, "FSCM", LONG2FIX(CRM114_FSCM));
  rb_define_const(classifier_class, "HYPERSPACE", LONG2FIX(CRM114_HYPERSPACE));
  rb_define_const(classifier_class, "ENTROPY", LONG2FIX(CRM114_ENTROPY));
  rb_define_const(classifier_class, "STRING", LONG2FIX(CRM114_STRING));
  rb_define_const(classifier_class, "UNIQUE", LONG2FIX(CRM114_UNIQUE));
  rb_define_const(classifier_class, "CROSSLINK", LONG2FIX(CRM114_CROSSLINK));  

  ConfigClass = rb_define_class_under(classifier_class, "Config", rb_cObject);
  rb_define_alloc_func(ConfigClass, crm_alloc);
  rb_define_method(ConfigClass, "initialize", config_init, 1);
  rb_define_method(ConfigClass, "classes=", config_setClasses, 1);
  rb_define_method(ConfigClass, "datablock_size=", config_setDatablockSize, 1);
  rb_define_method(ConfigClass, "regex=", config_setRegex, 1);
  rb_define_method(ConfigClass, "pipeline=", config_setPipeline, 0); //TODO
}

static VALUE crm_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, crm_mark, crm_free, NULL);
}

static void crm_mark(Classifier *crm)
{
}

static void crm_free(Classifier *crm)
{
}

///////////////////////
/// CLASSIFIER
//////////////

static VALUE crm_init(VALUE obj, VALUE flags)
{
  Classifier *crm = malloc(sizeof(Classifier));
  crm->cb = crm114_new_cb();

  DATA_PTR(obj) = crm;

  crm114_cb_setflags(crm->cb, FIX2LONG(flags));
  crm114_cb_setclassdefaults(crm->cb);

  return Qnil;
}

static VALUE crm_config(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);

  VALUE new_config = rb_funcall(ConfigClass, rb_intern("new"), 1, obj);
  rb_yield(new_config);

  crm114_cb_setblockdefaults(crm->cb);

  return Qnil;
}

static VALUE crm_getClasses(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);

  int length = crm->cb->how_many_classes;
  VALUE ary = rb_ary_new2(length);
  for (int i = 0; i < length; i++) {
    rb_ary_store(ary, i, rb_str_new_cstr(crm->cb->class[i].name));
  }

  return ary;
}

static VALUE crm_getDatablockSize(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);
  return INT2FIX(crm->cb->datablock_size);
}

VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text)
{
  Classifier *crm = DATA_PTR(obj);

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


///////////////
//// CONFIG
///////////

static VALUE config_init(VALUE obj, VALUE classifier)
{
  DATA_PTR(obj) = DATA_PTR(classifier);
  return Qnil;
}

static VALUE config_setClasses(VALUE obj, VALUE rb_classes_ary)
{
  Classifier *classifier = DATA_PTR(obj);

  int length = RARRAY_LEN(rb_classes_ary);
  classifier->cb->how_many_classes = length;
  for (int i = 0; i < length; i++) {
    char *name = RSTRING_PTR(rb_ary_entry(rb_classes_ary, i));
    strcpy(classifier->cb->class[i].name, name);
  }

  return rb_classes_ary;
}

static VALUE config_setDatablockSize(VALUE obj, VALUE rb_size)
{
  Classifier *classifier = DATA_PTR(obj);
  classifier->cb->datablock_size = FIX2INT(rb_size);
  return rb_size;
}

static VALUE config_setRegex(VALUE obj, VALUE regex)
{
  Classifier *classifier = DATA_PTR(obj);
  
  char *regex_str = RSTRING_PTR(regex);
  int regex_len = RSTRING_LEN(regex);
  crm114_cb_setregex(classifier->cb, regex_str, regex_len);
  
  return regex;
}

static VALUE config_setPipeline(VALUE obj)
{
  //TODO

  return Qnil;
}


