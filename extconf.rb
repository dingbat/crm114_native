require 'mkmf'

# find_header("../include")
# find_library("../lib", "crm114_strerror")

# find_library("CRM114", "lib")

path = File.expand_path(File.dirname(__FILE__),"libcrm114")
puts path

#$LDFLAGS << " -Wl,-rpath,#{File.join(path,'lib')}"

#dir_config("crm114", File.join(path,'include'), File.join(path,'lib'))
#have_library("crm114", "crm114_new_cb")
find_library("crm114", "crm114_new_cb", File.join(path,'lib')) or raise
find_header("crm114_lib.h", File.join(path,'include')) or raise

create_makefile("CRM114")