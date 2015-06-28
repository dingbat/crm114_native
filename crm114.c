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

typedef struct Result
{
  CRM114_MATCHRESULT result;
} Result;

// CONFIG

static VALUE config_alloc(VALUE klass);
static void config_mark(Classifier *crm);
static void config_free(Classifier *crm);

static VALUE config_init(VALUE obj, VALUE classifier);
static VALUE config_setClasses(VALUE obj, VALUE rb_classes_ary);
static VALUE config_setDatablockSize(VALUE obj, VALUE rb_size);
static VALUE config_setRegex(VALUE obj, VALUE regex);
static VALUE config_setPipeline(VALUE obj);

// CLASSIFIER

static VALUE crm_alloc(VALUE klass);
static void crm_mark(Classifier *crm);
static void crm_free(Classifier *crm);

static VALUE crm_init(VALUE obj, VALUE flags);
static VALUE crm_config(VALUE obj);
static VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text);
static VALUE crm_classify_text(VALUE obj, VALUE text);
static VALUE crm_getClasses(VALUE obj);
static VALUE crm_getDatablockSize(VALUE obj);

//RESULT

static VALUE result_alloc(VALUE klass);
static void result_mark(Result *crm);
static void result_free(Result *crm);

static VALUE result_init(VALUE obj);
static void _result_set(VALUE obj, CRM114_MATCHRESULT result);
static VALUE result_tsprob(VALUE obj);
static VALUE result_overall_pR(VALUE obj);
static VALUE result_bestmatch_index(VALUE obj);
static VALUE result_unk_features(VALUE obj);


// Global classes for static access
static VALUE ConfigClass;
static VALUE ResultClass;

void Init_CRM114()
{
  VALUE crm114_module = rb_define_module("CRM114");

  //
  // Classifier
  ////////

  VALUE classifier_class = rb_define_class_under(crm114_module, "Classifier", rb_cObject);
  rb_define_alloc_func(classifier_class, crm_alloc);
  rb_define_method(classifier_class, "initialize", crm_init, 1);

  // Virtual attrs
  rb_define_method(classifier_class, "classes", crm_getClasses, 0);
  rb_define_method(classifier_class, "datablock_size", crm_getDatablockSize, 0);

  // Methods
  rb_define_method(classifier_class, "config", crm_config, 0);
  rb_define_method(classifier_class, "learn_text", crm_learn_text, 2);
  rb_define_method(classifier_class, "classify_text", crm_classify_text, 1);

  // Constants
  rb_define_const(classifier_class, "OSB", LONG2NUM(CRM114_OSB));
  rb_define_const(classifier_class, "SVM", LONG2NUM(CRM114_SVM));
  rb_define_const(classifier_class, "FSCM", LONG2NUM(CRM114_FSCM));
  rb_define_const(classifier_class, "HYPERSPACE", LONG2NUM(CRM114_HYPERSPACE));
  rb_define_const(classifier_class, "ENTROPY", LONG2NUM(CRM114_ENTROPY));
  rb_define_const(classifier_class, "STRING", LONG2NUM(CRM114_STRING));
  rb_define_const(classifier_class, "UNIQUE", LONG2NUM(CRM114_UNIQUE));
  rb_define_const(classifier_class, "CROSSLINK", LONG2NUM(CRM114_CROSSLINK));  

  //
  // Config
  ////////

  ConfigClass = rb_define_class_under(classifier_class, "Config", rb_cObject);
  rb_define_alloc_func(ConfigClass, config_alloc);
  rb_define_method(ConfigClass, "initialize", config_init, 1);
  rb_define_method(ConfigClass, "classes=", config_setClasses, 1);
  rb_define_method(ConfigClass, "datablock_size=", config_setDatablockSize, 1);
  rb_define_method(ConfigClass, "regex=", config_setRegex, 1);
  rb_define_method(ConfigClass, "pipeline=", config_setPipeline, 0); //TODO

  //
  // Result
  ////////

  ResultClass = rb_define_class_under(crm114_module, "Result", rb_cObject);
  rb_define_alloc_func(ResultClass, result_alloc);
  rb_define_method(ResultClass, "initialize", result_init, 0);

  rb_define_method(ResultClass, "tsprob", result_tsprob, 0);
  rb_define_method(ResultClass, "overall_pR", result_overall_pR, 0);
  rb_define_method(ResultClass, "bestmatch_index", result_bestmatch_index, 0);
  rb_define_method(ResultClass, "unk_features", result_unk_features, 0);
}

///////////////////////
/// CLASSIFIER
//////////////

static VALUE crm_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, crm_mark, crm_free, NULL);
}

static void crm_mark(Classifier *crm)
{
}

static void crm_free(Classifier *crm)
{
  free(crm->cb);
  free(crm->db);
  free(crm);
}

static VALUE crm_init(VALUE obj, VALUE flags)
{
  Classifier *crm = malloc(sizeof(Classifier));
  crm->cb = crm114_new_cb();

  DATA_PTR(obj) = crm;

  crm114_cb_setflags(crm->cb, NUM2LONG(flags));
  crm114_cb_setclassdefaults(crm->cb);

  return Qnil;
}

static VALUE crm_config(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);

  VALUE new_config = rb_funcall(ConfigClass, rb_intern("new"), 1, obj);
  rb_yield(new_config);

  // crm114_cb_setblockdefaults(crm->cb);
  crm->db = crm114_new_db(crm->cb);

  return Qnil;
}

static VALUE crm_getClasses(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);

  int length = crm->cb->how_many_classes;
  VALUE hash = rb_hash_new();
  for (int i = 0; i < length; i++) {
    rb_hash_aset(hash, ID2SYM(rb_intern(crm->cb->class[i].name)), INT2FIX(crm->cb->class[i].success));
  }

  return hash;
}

static VALUE crm_getDatablockSize(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);
  return LONG2NUM(crm->cb->datablock_size);
}

VALUE crm_learn_text(VALUE obj, VALUE whichClass, VALUE text)
{
  Classifier *crm = DATA_PTR(obj);

  int idx;
  if (TYPE(whichClass) == T_STRING || TYPE(whichClass) == T_SYMBOL) {
    const char *string;
    if (TYPE(whichClass) == T_SYMBOL) {
      string = rb_id2name(SYM2ID(whichClass));
    } else {
      string = RSTRING_PTR(whichClass);
    }
    //get the index of the string
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

VALUE crm_classify_text(VALUE obj, VALUE text)
{
  Classifier *crm = DATA_PTR(obj);

  char *text_str = RSTRING_PTR(text);
  int text_len = RSTRING_LEN(text);

  CRM114_MATCHRESULT result;
  crm->err = crm114_classify_text(crm->db, text_str, text_len, &result);
  if (crm->err) {
    return Qnil;
  } else {
    VALUE rb_result = rb_funcall(ResultClass, rb_intern("new"), 0);
    _result_set(rb_result, result);
    return rb_result;
  }
}


///////////////
//// RESULT
///////////

static VALUE result_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, result_mark, result_free, NULL);
}

static void result_mark(Result *res)
{
}

static void result_free(Result *res)
{
  free(res);
}

static VALUE result_init(VALUE obj)
{
  DATA_PTR(obj) = malloc(sizeof(Result));
  return Qnil;
}

static void _result_set(VALUE obj, CRM114_MATCHRESULT result)
{
  Result *res = DATA_PTR(obj);
  res->result = result;
}

static VALUE result_tsprob(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  return DBL2NUM(res->result.tsprob);
}

static VALUE result_overall_pR(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  return DBL2NUM(res->result.overall_pR);
}

static VALUE result_bestmatch_index(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  return INT2FIX(res->result.bestmatch_index);
}

static VALUE result_unk_features(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  return INT2FIX(res->result.unk_features);
}


///////////////
//// CONFIG
///////////

static VALUE config_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, config_mark, config_free, NULL);
}

static void config_mark(Classifier *crm)
{
}

static void config_free(Classifier *crm)
{
  //nothing, since it's just an assignment on init
}

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
  classifier->cb->datablock_size = NUM2LONG(rb_size);
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

