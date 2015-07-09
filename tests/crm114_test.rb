require 'test/unit'
  
require_relative '../lib/crm114_native'
require_relative './lit_frags'

class CRM114Test < Test::Unit::TestCase
  def setup
    flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
    @cb = CRM114::Classifier.new(flags)
    @cb.config do |config|
      config.datablock_size = 8000000
      config.classes = ["alice","hamlet"]
    end
  end

  def test_config
    classes = {a:1,b:0}

    @cb.config do |config|
      config.datablock_size = 128504
      config.classes = ["a","b"]
    end

    assert_equal 128504, @cb.datablock_size
    assert_equal classes, @cb.classes

    @cb.config do |config|
      config.classes = {"a" => 1, "b" => 0}
    end
    assert_equal classes, @cb.classes

    @cb.config do |config|
      config.classes = classes
    end
    assert_equal classes, @cb.classes
  end

  def test_classify
    assert_nil @cb.learn_text(0, ALICE1)
    assert_nil @cb.learn_text("alice", ALICE2)
    assert_nil @cb.learn_text("alice", ALICE3)
    assert_nil @cb.learn_text(:hamlet, HAMLET1)
    assert_nil @cb.learn_text(:hamlet, HAMLET2)
    assert_nil @cb.learn_text(:hamlet, HAMLET3)
    result = @cb.classify_text(ALICE4)

    assert_equal CRM114::Result, result.class
    assert_nil result.error
    assert_equal :alice, result.best_match
  end

  def test_serialize
    @cb.learn_text(0, ALICE1)
    @cb.learn_text("alice", ALICE2)
    @cb.learn_text("alice", ALICE3)
    @cb.learn_text(:hamlet, HAMLET1)
    @cb.learn_text(:hamlet, HAMLET2)
    @cb.learn_text(:hamlet, HAMLET3)    
    
    dump = @cb.dump_memory

    result = @cb.classify_text(ALICE4)

    flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
    cb2 = CRM114::Classifier.new(flags)
    cb2.config do |config|
      config.datablock_size = 8000000
      config.classes = ["alice","hamlet"]
      config.load_datablock(dump)
    end

    result2 = cb2.classify_text(ALICE4)

    assert_equal result.error, result2.error
    assert_equal result.best_match, result2.best_match

    # TODO
    # would be nice to compare the dumps of both, but we're getting
    # slight differences in the lengths of the binaries (i think bc)
    # when we malloc it pads or something, etc
  end
end
