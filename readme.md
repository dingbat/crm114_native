# crm114_native

a ruby gem for the [crm114 discriminator](https://en.wikipedia.org/wiki/CRM114_(program)) that uses native C bindings to the [libcrm114](http://crm114.sourceforge.net/wiki/doku.php?id=download) library.

## installing

gemfile:

```ruby
gem 'crm114_native', git: "https://github.com/dingbat/crm114_native"
```

manually (it's not on rubygems yet):
```
$ gem build crm114_native.gemspec
$ gem install crm114_native-1.0.gem
```

## using

##### learning & classifying:

```ruby
flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
classifier = CRM114::Classifier.new(flags)
classifier.config do |config|
  config.datablock_size = 8000000
  config.classes = [:alice, :hamlet]
end

classifier.learn_text :hamlet, HAMLET_TEXT   #=> nil (or error)
classifier.learn_text :alice, ALICE_TEXT

result = classifier.classify_text MYSTERY_TEXT  #=> CRM114::Result
result.best_match           #=> :alice
result.error                #=> nil
result.overall_probability  #=> 0.84
```

##### serializing & deserializing:

```ruby
dump = classifier.datablock_memory  #=> machine-specific(!) binary as string

classifier_clone = CRM114::Classifier.new(classifier.flags)
classifier_clone.config do |config|
  config.datablock_size = classifier.datablock_size
  config.classes = classifier.classes
  config.load_datablock_memory(dump)
end

a = classifier.classify_text(FOO).best_match
b = classifier_clone.classify_text(FOO).best_match
a == b #=> true
```

##### possible classifier flags (for now):

```ruby
CRM114::Classifier::OSB
CRM114::Classifier::SVM
CRM114::Classifier::FSCM
CRM114::Classifier::HYPERSPACE
CRM114::Classifier::ENTROPY
CRM114::Classifier::STRING
CRM114::Classifier::UNIQUE
CRM114::Classifier::CROSSLINK
```

##### possible errors:

```ruby
CRM114::Error::UNK                   # unknown error
CRM114::Error::BADARG                # bad arguments (config)
CRM114::Error::NOMEM                 # couldn't allocate memory
CRM114::Error::REGEX_ERR             # regex error
CRM114::Error::FULL                  # buffer full
CRM114::Error::CLASS_FULL            # datablock is full, can't learn more
CRM114::Error::OPEN_FAILED           # file open failed
CRM114::Error::NOT_YET_IMPLEMENTED   # not yet implemented!
```

more complete documentation coming later...

## testing

run `rake` (this will `rake compile` as well)