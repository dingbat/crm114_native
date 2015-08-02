require 'test/unit'
  
require_relative '../lib/crm114_native'
require_relative './lit_frags'

class CRM114Test < Test::Unit::TestCase
  def setup
    CRM114.debug!
    
    @flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
    @cb = CRM114::Classifier.new(@flags)
    @cb.config do |config|
      config.datablock_size = 20000
      config.classes = ["alice","hamlet"]
    end
  end

  def test_config
    assert_equal @flags, @cb.flags

    classes = {a:true,b:false}

    @cb.config do |config|
      config.datablock_size = 128504
      config.classes = ["a","b"]
    end

    assert_equal 128504, @cb.datablock_size
    assert_equal classes, @cb.classes

    @cb.config do |config|
      config.classes = {"a" => true, "b" => false}
    end
    assert_equal classes, @cb.classes

    @cb.config do |config|
      config.classes = classes
    end
    assert_equal classes, @cb.classes

    @cb.config_without_db_defaults do |config|
      config.classes = classes
      config.datablock_size = 80000
    end
    assert_equal 80000, @cb.datablock_size
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
    @cb.learn_text(:alice, ALICE1)
    @cb.learn_text(:alice, ALICE2)
    @cb.learn_text(:alice, ALICE3)
    @cb.learn_text(:alice, ALICE4)
    @cb.learn_text(:alice, ALICE5)
    @cb.learn_text(:hamlet, HAMLET1)
    @cb.learn_text(:hamlet, HAMLET2)
    @cb.learn_text(:hamlet, HAMLET3)    
    @cb.learn_text(:hamlet, HAMLET4)    
    
    dump = @cb.datablock_memory

    result = @cb.classify_text(HAMLET5)

    cb2 = CRM114::Classifier.new(@cb.flags)
    cb2.config do |config|
      config.datablock_size = @cb.datablock_size
      config.classes = @cb.classes
      config.load_datablock_memory(dump)
    end

    dump2 = cb2.datablock_memory

    assert_equal dump.length, dump2.length
    assert_equal dump, dump2

    result2 = cb2.classify_text(HAMLET5)

    assert_equal result.error, result2.error
    assert_equal result.best_match, result2.best_match
    assert_equal result[:hamlet], result2[:hamlet]
    assert_equal result[:alice], result2[:alice]

    # try serializing again (the clone)

    cb3 = CRM114::Classifier.new(cb2.flags)
    cb3.config do |config|
      config.datablock_size = cb2.datablock_size
      config.classes = cb2.classes
      config.load_datablock_memory(dump2)
    end

    dump3 = cb3.datablock_memory

    assert_equal dump2.length, dump3.length
    assert_equal dump2, dump3

    result3 = cb3.classify_text(HAMLET5)

    assert_equal result2.error, result3.error
    assert_equal result2.best_match, result3.best_match
    assert_equal result2[:hamlet], result3[:hamlet]
    assert_equal result2[:alice], result3[:alice]
  end

  def test_invalid_db_size
    @cb.learn_text(:alice, ALICE1)
    @cb.learn_text(:alice, ALICE2)
    @cb.learn_text(:alice, ALICE3)
    @cb.learn_text(:alice, ALICE4)
    @cb.learn_text(:alice, ALICE5)
    @cb.learn_text(:hamlet, HAMLET1)
    @cb.learn_text(:hamlet, HAMLET2)
    @cb.learn_text(:hamlet, HAMLET3)    
    @cb.learn_text(:hamlet, HAMLET4)    
    
    dump = @cb.datablock_memory

    result = @cb.classify_text(HAMLET5)

    cb2 = CRM114::Classifier.new(@cb.flags)

    # should be an error that db_size < dump size
    assert_raises(RuntimeError) do
      cb2.config do |config|
        config.datablock_size = 200
        config.load_datablock_memory(dump)
        config.classes = @cb.classes
      end
    end

    # should segfault
    assert_raises(RuntimeError) do
      cb2.classify_text(HAMLET5)
    end
  end

  def test_result_classes
    @cb.learn_text(:alice, ALICE1)
    @cb.learn_text(:alice, ALICE2)
    @cb.learn_text(:hamlet, HAMLET1)
    @cb.learn_text(:hamlet, HAMLET2)
    @cb.learn_text(:hamlet, HAMLET3)

    result = @cb.classify_text(ALICE3)

    assert_nil result.error
    assert_equal result[:alice].documents, 2
    assert_equal result[:hamlet].documents, 3
    assert_not_nil result[:alice]
    assert_not_nil result[:alice].pR
    assert_not_nil result[:alice].probability
    assert_not_nil result[:alice].features
    assert_not_nil result[:alice].hits
    assert_not_nil result[:hamlet]
    
    assert result[:alice].probability > result[:hamlet].probability, "Alice should have greater confidence than hamlet"
  end
end
