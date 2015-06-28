require 'test/unit'

require_relative '../CRM114'

Alice_frag = \
    "So she was considering in her own mind (as well as she could, for the\n" \
    "hot day made her feel very sleepy and stupid), whether the pleasure\n" \
    "of making a daisy-chain would be worth the trouble of getting up and\n" \
    "picking the daisies, when suddenly a White Rabbit with pink eyes ran\n" \
    "close by her.\n"
Hound_frag = \
    "\"Well, Watson, what do you make of it?\"\n" \
    "Holmes was sitting with his back to me, and I had given him no\n" \
    "sign of my occupation.\n" \
    "\"How did you know what I was doing?  I believe you have eyes in\n" \
    "the back of your head.\"\n"
Macbeth_frag = \
"    Double, double, toil and trouble;\n" \
"    Fire, burn; and cauldron, bubble.\n" \
"    \n" \
"    SECOND WITCH.\n" \
"    Fillet of a fenny snake,\n" \
"    In the caldron boil and bake;\n" \
"    Eye of newt, and toe of frog,\n" \
"    Wool of bat, and tongue of dog,\n" \
"    Adder's fork, and blind-worm's sting,\n" \
"    Lizard's leg, and howlet's wing,--\n" \
"    For a charm of powerful trouble,\n" \
"    Like a hell-broth boil and bubble.\n" \

Willows_frag = \
    "'This is fine!' he said to himself. 'This is better than whitewashing!'\n" \
    "The sunshine struck hot on his fur, soft breezes caressed his heated\n" \
    "brow, and after the seclusion of the cellarage he had lived in so long\n" \
    "the carol of happy birds fell on his dulled hearing almost like a shout."

class ControlBlockTest < Test::Unit::TestCase
  def setup
    flags = CRM114::Classifier::SVM | CRM114::Classifier::STRING
    @cb = CRM114::Classifier.new(flags)
    @cb.config do |config|
      config.datablock_size = 8000000
      config.classes = ["a","b"]
    end
  end

  def test_config
    assert_equal 8000000, @cb.datablock_size
    classes = {a:1,b:0}
    assert_equal classes, @cb.classes
  end

  def test_classify
    @cb.learn_text(0, Alice_frag)
    @cb.learn_text(:b, Macbeth_frag)
    result = @cb.classify_text(Willows_frag)

    assert_equal CRM114::Result, result.class
    assert_equal 0, result.bestmatch_index
  end
end
