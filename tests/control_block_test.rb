require 'test/unit'

require_relative '../CRM114'

class ControlBlockTest < Test::Unit::TestCase
  def test_classes
    cb = CRM114::ControlBlock.new(0, ["a", "b", "c"], 8000)
    assert_equal ["a","b","c"], cb.classes
  end

  def test_db
    cb = CRM114::ControlBlock.new(0, ["a", "b", "c"], 8000)
    db = cb.new_db
    assert_equal CRM114::DataBlock, db.class

    
  end
end