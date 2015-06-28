require 'test/unit'

require_relative '../CRM114'

class ControlBlockTest < Test::Unit::TestCase
  def test_config
    flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
    cb = CRM114::Classifier.new(flags)
    cb.config do |config|
      config.classes = ["a","b","c"]
    end
    assert_equal ["a","b","c"], cb.classes
  end

  def test_db
  end
end