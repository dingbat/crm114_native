#include "ruby.h"

#include <signal.h>

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"

extern int crm114__user_trace;
extern int crm114__internal_trace;

typedef struct Classifier
{
  CRM114_CONTROLBLOCK *cb;
  CRM114_DATABLOCK *db;
  int has_db;
} Classifier;

typedef struct Result
{
  CRM114_MATCHRESULT result;
  CRM114_ERR error;
} Result;

#define BOOL2INT(x)  (((x) == Qtrue)?1:0)
#define INT2BOOL(x)  ((x)?Qtrue:Qfalse)


static VALUE crm_class_debug(int argc, VALUE*argv, VALUE module);

// CONFIG

static VALUE config_alloc(VALUE klass);
static void config_mark(Classifier *crm);
static void config_free(Classifier *crm);

static VALUE config_init(VALUE obj, VALUE classifier);
static VALUE config_set_classes(VALUE obj, VALUE rb_classes_ary);
static VALUE config_set_datablock_size(VALUE obj, VALUE rb_size);
static VALUE config_set_regex(VALUE obj, VALUE regex);
static VALUE config_set_pipeline(VALUE obj);
static VALUE config_load_datablock_memory(VALUE obj, VALUE mem);

// CLASSIFIER

static VALUE crm_alloc(VALUE klass);
static void crm_mark(Classifier *crm);
static void crm_free(Classifier *crm);

static VALUE crm_init(VALUE obj, VALUE flags);
static VALUE crm_config(VALUE obj);
static VALUE crm_config_without_db_defaults(VALUE obj);
static VALUE crm_learn_text(VALUE obj, VALUE the_class, VALUE text);
static VALUE crm_classify_text(VALUE obj, VALUE text);
static VALUE crm_get_classes(VALUE obj);
static VALUE crm_get_datablock_size(VALUE obj);
static VALUE crm_get_flags(VALUE obj);

static VALUE crm_datablock_memory(VALUE obj);
static VALUE crm_controlblock_memory(VALUE obj);

//RESULT

static VALUE result_alloc(VALUE klass);
static void result_mark(Result *crm);
static void result_free(Result *crm);

static VALUE result_total_success_probability(VALUE obj);
static VALUE result_overall_pR(VALUE obj);
static VALUE result_best_match(VALUE obj);
static VALUE result_text_features(VALUE obj);
static VALUE result_get_error(VALUE obj);
static VALUE result_get_class(VALUE obj, VALUE key);
static VALUE result_each(VALUE obj);
static VALUE result_classification_results(VALUE obj);

//(private)
static void _result_set_success(VALUE obj, CRM114_MATCHRESULT result);
static void _result_set_error(VALUE obj, CRM114_ERR error);

// Global classes for static access
static VALUE ConfigClass;
static VALUE ResultClass;
static VALUE ClassResultStruct;
static int DEBUG = 0;

void clean_exit_on_sig(int sig_num)
{
  if (sig_num == SIGSEGV) {
    rb_raise(rb_eRuntimeError, "segfault in CRM114 gem");
  }
  else if (sig_num == SIGFPE) {
    rb_raise(rb_eRuntimeError, "floating point exception in CRM114 gem ('8' probably)");
  }
}

void Init_crm114_native()
{
  VALUE crm114_module = rb_define_module("CRM114");
  rb_define_singleton_method(crm114_module, "debug!", crm_class_debug, -1);

  //
  // Classifier
  ////////

  VALUE classifier_class = rb_define_class_under(crm114_module, "Classifier", rb_cObject);
  rb_define_alloc_func(classifier_class, crm_alloc);
  rb_define_method(classifier_class, "initialize", crm_init, 1);

  // Virtual attrs
  rb_define_method(classifier_class, "classes", crm_get_classes, 0);
  rb_define_method(classifier_class, "datablock_size", crm_get_datablock_size, 0);
  rb_define_method(classifier_class, "flags", crm_get_flags, 0);

  // Methods
  rb_define_method(classifier_class, "config", crm_config, 0);
  rb_define_method(classifier_class, "config_without_db_defaults", crm_config_without_db_defaults, 0);
  rb_define_method(classifier_class, "learn_text", crm_learn_text, 2);
  rb_define_method(classifier_class, "classify_text", crm_classify_text, 1);
  rb_define_method(classifier_class, "datablock_memory", crm_datablock_memory, 0);
  rb_define_method(classifier_class, "controlblock_memory", crm_controlblock_memory, 0);

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
  rb_define_method(ConfigClass, "classes=", config_set_classes, 1);
  rb_define_method(ConfigClass, "datablock_size=", config_set_datablock_size, 1);
  rb_define_method(ConfigClass, "regex=", config_set_regex, 1);
  rb_define_method(ConfigClass, "pipeline=", config_set_pipeline, 0); //TODO
  rb_define_method(ConfigClass, "load_datablock_memory", config_load_datablock_memory, 1);

  //
  // Result
  ////////

  ResultClass = rb_define_class_under(crm114_module, "Result", rb_cObject);
  rb_define_alloc_func(ResultClass, result_alloc);

  rb_define_method(ResultClass, "total_success_probability", result_total_success_probability, 0);
  rb_define_method(ResultClass, "overall_pR", result_overall_pR, 0);
  rb_define_method(ResultClass, "best_match", result_best_match, 0);
  rb_define_method(ResultClass, "text_features", result_text_features, 0);
  rb_define_method(ResultClass, "error", result_get_error, 0);
  rb_define_method(ResultClass, "[]", result_get_class, 1);
  rb_define_method(ResultClass, "each", result_each, 0);
  rb_define_method(ResultClass, "classification_results", result_classification_results, 0);

  ClassResultStruct = rb_eval_string_protect("Struct.new('ClassificationResult',\
    :name,\
    :pR,\
    :probability,\
    :documents,\
    :features,\
    :hits)", 0);

  //
  // Errors
  ////////

  VALUE error_module = rb_define_module_under(crm114_module, "Error");
  rb_define_const(error_module, "UNK", INT2FIX(CRM114_UNK));
  rb_define_const(error_module, "BADARG", INT2FIX(CRM114_BADARG));
  rb_define_const(error_module, "NOMEM", INT2FIX(CRM114_NOMEM));
  rb_define_const(error_module, "REGEX_ERR", INT2FIX(CRM114_REGEX_ERR));
  rb_define_const(error_module, "FULL", INT2FIX(CRM114_FULL));
  rb_define_const(error_module, "CLASS_FULL", INT2FIX(CRM114_CLASS_FULL));
  rb_define_const(error_module, "OPEN_FAILED", INT2FIX(CRM114_OPEN_FAILED));
  rb_define_const(error_module, "NOT_YET_IMPLEMENTED", INT2FIX(CRM114_NOT_YET_IMPLEMENTED));

  signal(SIGSEGV, clean_exit_on_sig); // Trap segfault
  signal(SIGFPE, clean_exit_on_sig); // Trap "floating point exception" that comes up with OSB
}

void debug(const char *fmt, ...)
{
  if (DEBUG == 0) {
    return;
  }

  char buf[500];

  va_list args;

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  rb_funcall(rb_mKernel, rb_intern("puts"), 1, rb_str_new2(buf));
}

VALUE crm_class_debug(int argc, VALUE*argv, VALUE module)
{
  VALUE rb_level;
  rb_scan_args(argc, argv, "01", &rb_level);

  int level = 0;
  if (!NIL_P(rb_level))
      level = FIX2INT(rb_level);

  DEBUG = 1;
  if (level > 0) {
    crm114__internal_trace = 1;
    if (level > 1) {
      crm114__user_trace = 1;
    }
  }
  debug("CRM114 debug mode enabled");
  return Qtrue;
}

int index_of_class(VALUE the_class, int how_many_classes, CRM114_CONTROLBLOCK *cb, CRM114_MATCHRESULT *result)
{
  if (TYPE(the_class) == T_STRING || TYPE(the_class) == T_SYMBOL) {
    const char *string;
    if (TYPE(the_class) == T_SYMBOL) {
      string = rb_id2name(SYM2ID(the_class));
    } else {
      string = RSTRING_PTR(the_class);
    }
    //get the index of the string
    for (int idx = 0; idx < how_many_classes; idx++) {
      if ((cb && strcmp(cb->class[idx].name, string) == 0) ||
          (result && strcmp(result->class[idx].name, string) == 0)) {
        return idx;
      }
    }

    return -1;
  } else {
    return FIX2INT(the_class);
  }
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
  if (crm->cb) {
    free(crm->cb);
  }
  if (crm->has_db) {
    free(crm->db);
  }
  free(crm);
}

static VALUE crm_init(VALUE obj, VALUE flags)
{
  Classifier *crm = malloc(sizeof(Classifier));
  crm->cb = NULL;
  if ((crm->cb = crm114_new_cb())) {
    crm->has_db = 0;

    DATA_PTR(obj) = crm;

    crm114_cb_setflags(crm->cb, NUM2LONG(flags));
    crm114_cb_setclassdefaults(crm->cb);
  }
  else {
    rb_raise(rb_eRuntimeError, "couldn't allocate CRM114 control block");
  }

  return Qnil;
}

static VALUE _crm_config(VALUE obj, int with_db_defaults)
{
  Classifier *crm = DATA_PTR(obj);

  VALUE new_config = rb_funcall(ConfigClass, rb_intern("new"), 1, obj);
  rb_yield(new_config);

  if (crm->has_db == 0) {
    if (with_db_defaults) {
      crm114_cb_setblockdefaults(crm->cb);
    }

    if ((crm->db = crm114_new_db(crm->cb))) {
      crm->has_db = 1;
    } else {
      rb_raise(rb_eRuntimeError, "couldn't allocate CRM114 datablock");
    }
  }

  return Qnil;
}

static VALUE crm_config(VALUE obj)
{
  return _crm_config(obj, 1);
}

static VALUE crm_config_without_db_defaults(VALUE obj)
{
    return _crm_config(obj, 0);
}

static VALUE crm_get_classes(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);

  int length = crm->cb->how_many_classes;
  VALUE hash = rb_hash_new();
  int i;
  for (i = 0; i < length; i++) {
    rb_hash_aset(hash, ID2SYM(rb_intern(crm->cb->class[i].name)), INT2BOOL(crm->cb->class[i].success));
  }

  return hash;
}

static VALUE crm_get_datablock_size(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);
  return LONG2NUM(crm->cb->datablock_size);
}

static VALUE crm_get_flags(VALUE obj)
{
  Classifier *crm = DATA_PTR(obj);
  return LONG2NUM(crm->cb->classifier_flags);
}

VALUE crm_learn_text(VALUE obj, VALUE the_class, VALUE text)
{
  Classifier *crm = DATA_PTR(obj);

  int idx = index_of_class(the_class, crm->cb->how_many_classes, crm->cb, NULL);

  char *text_str = RSTRING_PTR(text);
  int text_len = RSTRING_LEN(text);

  CRM114_ERR err;
  err = crm114_learn_text(&(crm->db), idx, text_str, text_len);
  if (err == CRM114_OK) {
    return Qnil;
  } else {
    return INT2FIX(err);
  }
}

VALUE crm_classify_text(VALUE obj, VALUE text)
{
  Classifier *crm = DATA_PTR(obj);

  char *text_str = RSTRING_PTR(text);
  int text_len = RSTRING_LEN(text);

  CRM114_MATCHRESULT result;
  
  CRM114_ERR err;
  err = crm114_classify_text(crm->db, text_str, text_len, &result);
  
  VALUE rb_result = rb_funcall(ResultClass, rb_intern("new"), 0);
  
  if (err) {
    _result_set_error(rb_result, err);
  } else {
    _result_set_success(rb_result, result);
  }
  
  return rb_result;
}

VALUE crm_datablock_memory(VALUE obj)
{
  Classifier *classifier = DATA_PTR(obj);

  VALUE rb_str = rb_str_new((char *)(classifier->db), classifier->cb->datablock_size);
  return rb_str;
}

VALUE crm_controlblock_memory(VALUE obj)
{
  Classifier *classifier = DATA_PTR(obj);

  VALUE rb_str = rb_str_new((char *)(classifier->cb), sizeof(*classifier->cb));
  return rb_str;
}

///////////////
//// RESULT
///////////

static VALUE result_alloc(VALUE klass)
{
  VALUE obj = Data_Wrap_Struct(klass, result_mark, result_free, NULL);
  Result *res = malloc(sizeof(Result));
  res->error = CRM114_OK;
  DATA_PTR(obj) = res;
  return obj;
}

static void result_mark(Result *res)
{
}

static void result_free(Result *res)
{
  free(res);
}

static void _result_set_success(VALUE obj, CRM114_MATCHRESULT result)
{
  Result *res = DATA_PTR(obj);
  res->result = result;
}

static void _result_set_error(VALUE obj, CRM114_ERR error)
{
  Result *res = DATA_PTR(obj);
  res->error = error;
}

static VALUE _result_get_classification_result(Result *res, int idx)
{
  return rb_funcall(ClassResultStruct, rb_intern("new"), 6,
    rb_str_new2(res->result.class[idx].name),
    DBL2NUM(res->result.class[idx].pR),
    DBL2NUM(res->result.class[idx].prob),
    INT2FIX(res->result.class[idx].documents),
    INT2FIX(res->result.class[idx].features),
    INT2FIX(res->result.class[idx].hits));
}

static VALUE result_get_error(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error == CRM114_OK) {
    return Qnil;
  }
  return INT2FIX(res->error);
}

static VALUE result_classification_results(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }

  VALUE ary = rb_ary_new2(res->result.how_many_classes);
  for (int i = 0; i < res->result.how_many_classes; i++) {
    rb_ary_push(ary, _result_get_classification_result(res, i));
  }
  return ary;
}

static VALUE result_each(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }

  for (int i = 0; i < res->result.how_many_classes; i++) {
    rb_yield(_result_get_classification_result(res, i));
  }

  return Qnil;
}

static VALUE result_get_class(VALUE obj, VALUE key)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }

  int idx = index_of_class(key, res->result.how_many_classes, NULL, &(res->result));
  return _result_get_classification_result(res, idx);
}

static VALUE result_total_success_probability(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }
  return DBL2NUM(res->result.tsprob);
}

static VALUE result_overall_pR(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }
  return DBL2NUM(res->result.overall_pR);
}

static VALUE result_best_match(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }
  char *class_name = res->result.class[res->result.bestmatch_index].name;
  return ID2SYM(rb_intern(class_name));
}

static VALUE result_text_features(VALUE obj)
{
  Result *res = DATA_PTR(obj);
  if (res->error != CRM114_OK) {
    return Qnil;
  }
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

static VALUE config_load_datablock_memory(VALUE obj, VALUE mem)
{
  Classifier *classifier = DATA_PTR(obj);

  if (classifier->cb->datablock_size < RSTRING_LEN(mem)) {
    rb_raise(rb_eRuntimeError,
      "datablock size set for classifier (%lu) is smaller than that of datablock memory loaded (%lu)",
      classifier->cb->datablock_size,
      RSTRING_LEN(mem));
    return Qnil;
  }

  if ((classifier->db = malloc(classifier->cb->datablock_size))) {
    memcpy(classifier->db, RSTRING_PTR(mem), classifier->cb->datablock_size);
    classifier->has_db = 1;
  } else {
    rb_raise(rb_eRuntimeError, "couldn't allocate CRM114 datablock for loading memory");
  }

  return Qnil;
}

int classes_hash_foreach(VALUE key, VALUE val, VALUE obj)
{
  Classifier *classifier = DATA_PTR(obj);

  const char *name;
  if (TYPE(key) == T_STRING) {
    name = RSTRING_PTR(key);
  } else {
    name = rb_id2name(SYM2ID(key));
  }

  int i = classifier->cb->how_many_classes;
  strcpy(classifier->cb->class[i].name, name);
  classifier->cb->class[i].success = BOOL2INT(val);
  classifier->cb->how_many_classes++;

  return ST_CONTINUE;
}

static VALUE config_set_classes(VALUE obj, VALUE rb_classes)
{
  Classifier *classifier = DATA_PTR(obj);

  if (TYPE(rb_classes) == T_ARRAY) {
    int length = RARRAY_LEN(rb_classes);
    classifier->cb->how_many_classes = length;
    for (int i = 0; i < length; i++) {
      char *name = RSTRING_PTR(rb_ary_entry(rb_classes, i));
      strcpy(classifier->cb->class[i].name, name);
    }
  }
  else if (TYPE(rb_classes) == T_HASH) {
    classifier->cb->how_many_classes = 0;
    rb_hash_foreach(rb_classes, classes_hash_foreach, obj);
  }

  crm114_cb_setblockdefaults(classifier->cb);

  return rb_classes;
}

static VALUE config_set_datablock_size(VALUE obj, VALUE rb_size)
{
  Classifier *classifier = DATA_PTR(obj);
  classifier->cb->datablock_size = NUM2LONG(rb_size);
  return rb_size;
}

static VALUE config_set_regex(VALUE obj, VALUE regex)
{
  Classifier *classifier = DATA_PTR(obj);
  
  char *regex_str = RSTRING_PTR(regex);
  int regex_len = RSTRING_LEN(regex);
  crm114_cb_setregex(classifier->cb, regex_str, regex_len);
  
  return regex;
}

static VALUE config_set_pipeline(VALUE obj)
{
  //TODO

  return Qnil;
}

