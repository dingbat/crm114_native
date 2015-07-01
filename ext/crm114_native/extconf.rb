require 'mkmf'

path = File.join File.expand_path(File.dirname(__FILE__)), "libcrm114"

# Make crm114
`cd #{path} && make` or raise

dir_config("crm114", File.join(path,'include'), path)
have_library("tre")

$LOCAL_LIBS << '-lcrm114'

create_makefile("crm114_native")