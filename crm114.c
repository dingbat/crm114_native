#include "ruby.h"

#include "crm114_sysincludes.h"
#include "crm114_config.h"
#include "crm114_structs.h"
#include "crm114_lib.h"
#include "crm114_internal.h"

static VALUE rb_dbClass;

void Init_ControlBlock(VALUE module);
void Init_DataBlock(VALUE module);
void _db_set(VALUE obj, CRM114_DATABLOCK *db);

void Init_CRM114()
{
  VALUE module = rb_define_module("CRM114");

  Init_DataBlock(module);
  Init_ControlBlock(module);
}

///////////////////////////////
//
//  ControlBlock
//
///////////////////////////////

static VALUE cb_setClasses(VALUE obj, VALUE rb_classes_ary);
static VALUE cb_setDatablockSize(VALUE obj, VALUE rb_size);

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

static VALUE cb_init(VALUE obj, VALUE flags, VALUE classes, VALUE datablock_size)
{
  CRM114_CONTROLBLOCK *cb = crm114_new_cb();

  DATA_PTR(obj) = cb;

  crm114_cb_setflags(cb, FIX2INT(flags));
  crm114_cb_setclassdefaults(cb);
  cb_setClasses(obj, classes);
  cb_setDatablockSize(obj, datablock_size);
  crm114_cb_setblockdefaults(cb);

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

static VALUE cb_setDatablockSize(VALUE obj, VALUE rb_size)
{
  CRM114_CONTROLBLOCK *cb = DATA_PTR(obj);
  cb->datablock_size = FIX2INT(rb_size);
  return rb_size;
}

static VALUE cb_getDatablockSize(VALUE obj)
{
  CRM114_CONTROLBLOCK *cb = DATA_PTR(obj);
  return INT2FIX(cb->datablock_size);
}

static VALUE cb_new_db(VALUE obj)
{
  CRM114_CONTROLBLOCK *cb = DATA_PTR(obj);
  CRM114_DATABLOCK *db = crm114_new_db(cb);

  VALUE klass = rb_dbClass;//rb_const_get(rb_cObject, rb_intern("CRM114::DataBlock"));  
  VALUE rb_db = rb_funcall(klass, rb_intern("new"), 0);
  _db_set(rb_db, db);

  return rb_db;
}

void Init_ControlBlock(VALUE module)
{
  VALUE rb_controlBlock = rb_define_class_under(module, "ControlBlock", rb_cObject);
  rb_define_alloc_func(rb_controlBlock, cb_alloc);
  rb_define_method(rb_controlBlock, "initialize", cb_init, 3);

  // Virtual attrs
  rb_define_method(rb_controlBlock, "classes", cb_getClasses, 0);
  rb_define_method(rb_controlBlock, "datablock_size", cb_getDatablockSize, 0);

  // Methods
  rb_define_method(rb_controlBlock, "new_db", cb_new_db, 0);

  // Constants
  VALUE rb_classifier_flags = rb_define_module_under(module, "Classifier");
  rb_define_const(rb_classifier_flags, "OSB", INT2FIX(CRM114_OSB));
  rb_define_const(rb_classifier_flags, "SVM", INT2FIX(CRM114_SVM));
  rb_define_const(rb_classifier_flags, "FSCM", INT2FIX(CRM114_FSCM));
  rb_define_const(rb_classifier_flags, "HYPERSPACE", INT2FIX(CRM114_HYPERSPACE));
  rb_define_const(rb_classifier_flags, "ENTROPY", INT2FIX(CRM114_ENTROPY));
  rb_define_const(rb_classifier_flags, "STRING", INT2FIX(CRM114_STRING));
  rb_define_const(rb_classifier_flags, "UNIQUE", INT2FIX(CRM114_UNIQUE));
  rb_define_const(rb_classifier_flags, "CROSSLINK", INT2FIX(CRM114_CROSSLINK));  
}

///////////////////////////////
//
//  DataBlock
//
///////////////////////////////

static void db_mark(CRM114_CONTROLBLOCK *crm)
{
}

static void db_free(CRM114_CONTROLBLOCK *crm)
{
}

static VALUE db_alloc(VALUE klass)
{
  return Data_Wrap_Struct(klass, db_mark, db_free, NULL);
}

void _db_set(VALUE obj, CRM114_DATABLOCK *db)
{
  DATA_PTR(obj) = db;
}

VALUE db_init(VALUE obj)
{
  return Qnil;
}

void Init_DataBlock(VALUE module)
{
  rb_dbClass = rb_define_class_under(module, "DataBlock", rb_cObject);
  rb_define_alloc_func(rb_dbClass, db_alloc);
  rb_define_method(rb_dbClass, "initialize", db_init, 0);
}
