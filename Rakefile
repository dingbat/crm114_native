require 'rake/extensiontask'

# This gives us `rake compile`, which compiles the extension
# and places the shared object in the lib folder, which is then
# used by the tests
Rake::ExtensionTask.new('crm114_native')

desc "Clean"
task :clean do
  File.delete("lib/crm114_native.bundle") if File.exist?("lib/crm114_native.bundle")
  File.delete("ext/crm114_native/crm114.o") if File.exist?("ext/crm114_native/crm114.o")
  File.delete("ext/crm114_native/crm114_native.bundle") if File.exist?("ext/crm114_native/crm114_native.bundle")
end

desc "Tests"
task :default => [:compile] do
  puts `ruby tests/crm114_test.rb`
end