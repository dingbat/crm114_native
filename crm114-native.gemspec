Gem::Specification.new do |s|
  s.name          = 'crm114-native'
  s.version       = '1.0'
  s.summary       = 'CRM114 classifier'
  s.description   = 'Native Ruby extension of libcrm114'
  s.authors       = ['Dan Hassin']
  s.email         = ['danhassin@mac.com']

  s.extensions    = ["ext/crm114-native/extconf.rb"]
  s.files         = Dir['ext/crm114-native/libcrm114/**/*.{c,h}', 
                        'ext/crm114-native/libcrm114/Makefile',
                        'ext/crm114-native/crm114.c',
                        'ext/crm114-native/extconf.rb']
end