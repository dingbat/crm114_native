Gem::Specification.new do |s|
  s.name          = 'crm114_native'
  s.version       = '1.0'
  s.summary       = 'CRM114 classifier'
  s.description   = 'Native Ruby extension of libcrm114'
  s.authors       = ['Dan Hassin']
  s.email         = ['danhassin@mac.com']

  # s.executables   = ['ext/crm114-native/libcrm114/lib/libcrm114.so.1']
  s.extensions    = ["ext/crm114_native/extconf.rb"]
  s.files         = Dir['ext/**/*.{c,h}', 
                        'ext/crm114_native/libcrm114/Makefile']
end