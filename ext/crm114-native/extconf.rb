require 'mkmf'

path = File.join File.expand_path(File.dirname(__FILE__)), "libcrm114"

# Make crm114
`cd #{path} && make` or raise

find_library("crm114", "crm114_new_cb", File.join(path,'lib')) or raise
find_header("crm114_lib.h", File.join(path,'include')) or raise

create_makefile("crm114-native")