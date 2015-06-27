require 'test/unit'

require_relative '../CRM114'

class ControlBlockTest < Test::Unit::TestCase
  def test_classes
    cb = CRM114.new(CRM114::SVM | CRM114::STRING)
    cb.config do |config|
      config.classes = ["a","b","c"]
    end
    assert_equal ["a","b","c"], cb.classes
  end

  def test_db
  end
end