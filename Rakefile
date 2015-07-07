require 'rake/extensiontask'

Rake::ExtensionTask.new('crm114_native')

desc "Clean"
task :clean do
  File.delete("lib/crm114_native.bundle") if File.exist?("lib/crm114_native.bundle")
  File.delete("ext/crm114_native/crm114.o") if File.exist?("ext/crm114_native/crm114.o")
  File.delete("ext/crm114_native/crm114_native.bundle") if File.exist?("ext/crm114_native/crm114_native.bundle")
end