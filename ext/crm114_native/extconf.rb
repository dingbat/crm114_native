require 'mkmf'

path = File.join File.expand_path(File.dirname(__FILE__)), "libcrm114"

have_library("tre") or raise

# Make crm114
`cd #{path} && make` or raise

dir_config("crm114", File.join(path,'include'), path)

$LOCAL_LIBS << " #{File.join(path,'libcrm114.a')}"
$CFLAGS << ' -std=c99'

create_makefile("crm114_native")