require 'test/unit'

require_relative '../CRM114'

class ControlBlockTest < Test::Unit::TestCase
  def test_classes
    cb = CRM114::ControlBlock.new(0)
    cb.classes = ["a", "b", "c"]
    assert_equal ["a","b","c"], cb.classes
  end
end