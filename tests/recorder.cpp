#include "../src/core/recorder.h"
#include "../src/core/const.h"
#include "catch/single_include/catch.hpp"


using std::string;
using namespace giada;


TEST_CASE("Test Recorder")
{
  /* Each SECTION the TEST_CASE is executed from the start. The following
  code is exectuted before each SECTION. */

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, nullptr);

  recorder::init();
  REQUIRE(recorder::frames.size() == 0);
  REQUIRE(recorder::global.size() == 0);

  SECTION("Test record single action")
  {
    recorder::rec(0, ACTION_KEYPRESS, 50, 1, 0.5f);

    REQUIRE(recorder::frames.size() == 1);
    REQUIRE(recorder::frames.at(0) == 50);
    REQUIRE(recorder::global.at(0).size() == 1);  // 1 action on frame #0
    REQUIRE(recorder::global.at(0).at(0)->chan == 0);
    REQUIRE(recorder::global.at(0).at(0)->type == ACTION_KEYPRESS);
    REQUIRE(recorder::global.at(0).at(0)->frame == 50);
    REQUIRE(recorder::global.at(0).at(0)->iValue == 1);
    REQUIRE(recorder::global.at(0).at(0)->fValue == 0.5f);
  }

  SECTION("Test record, two actions on same frame")
  {
    recorder::rec(0, ACTION_KEYPRESS, 50, 6, 0.3f);
    recorder::rec(0, ACTION_KEYREL,   50, 1, 0.5f);

    REQUIRE(recorder::frames.size() == 1);    // same frame, frames.size must stay 1
    REQUIRE(recorder::frames.at(0) == 50);
    REQUIRE(recorder::global.at(0).size() == 2);  // 2 actions on frame #0

    REQUIRE(recorder::global.at(0).at(0)->chan == 0);
    REQUIRE(recorder::global.at(0).at(0)->type == ACTION_KEYPRESS);
    REQUIRE(recorder::global.at(0).at(0)->frame == 50);
    REQUIRE(recorder::global.at(0).at(0)->iValue == 6);
    REQUIRE(recorder::global.at(0).at(0)->fValue == 0.3f);

    REQUIRE(recorder::global.at(0).at(1)->chan == 0);
    REQUIRE(recorder::global.at(0).at(1)->type == ACTION_KEYREL);
    REQUIRE(recorder::global.at(0).at(1)->frame == 50);
    REQUIRE(recorder::global.at(0).at(1)->iValue == 1);
    REQUIRE(recorder::global.at(0).at(1)->fValue == 0.5f);

    SECTION("Test record, another action on a different frame")
    {
      recorder::rec(0, ACTION_KEYPRESS, 70, 1, 0.5f);

      REQUIRE(recorder::frames.size() == 2);
      REQUIRE(recorder::frames.at(1) == 70);
      REQUIRE(recorder::global.at(0).size() == 2);  // 2 actions on frame #0
      REQUIRE(recorder::global.at(1).size() == 1);  // 1 actions on frame #1
      REQUIRE(recorder::global.at(1).at(0)->chan == 0);
      REQUIRE(recorder::global.at(1).at(0)->type == ACTION_KEYPRESS);
      REQUIRE(recorder::global.at(1).at(0)->frame == 70);
      REQUIRE(recorder::global.at(1).at(0)->iValue == 1);
      REQUIRE(recorder::global.at(1).at(0)->fValue == 0.5f);
    }
  }

  SECTION("Test retrieval")
  {
    recorder::rec(0, ACTION_KEYPRESS, 50, 6, 0.3f);
    recorder::rec(0, ACTION_KEYREL,   70, 1, 0.5f);
    recorder::rec(1, ACTION_KEYPRESS, 50, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   70, 1, 0.5f);
    recorder::rec(2, ACTION_KEYPRESS, 100, 6, 0.3f);
    recorder::rec(2, ACTION_KEYREL,   120, 1, 0.5f);

    /* Give me action on chan 1, type ACTION_KEYREL, frame 70. */
    recorder::action *action = nullptr;
    REQUIRE(recorder::getAction(1, ACTION_KEYREL, 70, &action) == 1);

    REQUIRE(action != nullptr);
    REQUIRE(action->chan == 1);
    REQUIRE(action->type == ACTION_KEYREL);
    REQUIRE(action->frame == 70);
    REQUIRE(action->iValue == 1);
    REQUIRE(action->fValue == 0.5f);

    /* Give me *next* action on chan 0, type ACTION_KEYREL, starting from frame 20.
    Must be action #2 */

    REQUIRE(recorder::getNextAction(0, ACTION_KEYREL, 20, &action) == 1);
    REQUIRE(action != nullptr);
    REQUIRE(action->chan == 0);
    REQUIRE(action->type == ACTION_KEYREL);
    REQUIRE(action->frame == 70);

    /* Give me *next* action on chan 2, type ACTION_KEYPRESS, starting from
    frame 200. You are requesting frame outside boundaries. */

    REQUIRE(recorder::getNextAction(2, ACTION_KEYPRESS, 200, &action) == -1);

    /* Give me *next* action on chan 2, type ACTION_KEYPRESS, starting from
    frame 100. That action does not exist. */

    REQUIRE(recorder::getNextAction(2, ACTION_KEYPRESS, 100, &action) == -2);
  }

  SECTION("Test deletion, single action")
  {
    recorder::rec(0, ACTION_KEYPRESS, 50, 6, 0.3f);
    recorder::rec(0, ACTION_KEYREL,   60, 1, 0.5f);
    recorder::rec(1, ACTION_KEYPRESS, 70, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   80, 1, 0.5f);

    /* Delete action #0, don't check values. */
    recorder::deleteAction(0, 50, ACTION_KEYPRESS, false, &mutex);

    REQUIRE(recorder::frames.size() == 3);
    REQUIRE(recorder::global.size() == 3);

    SECTION("Test deletion checked")
    {
      /* Delete action #1, check values. */
      recorder::deleteAction(1, 70, ACTION_KEYPRESS, true, &mutex, 6, 0.3f);

      REQUIRE(recorder::frames.size() == 2);
      REQUIRE(recorder::global.size() == 2);
    }
  }

  SECTION("Test deletion, range of actions")
  {
    recorder::rec(0, ACTION_KEYPRESS, 50, 6, 0.3f);
    recorder::rec(0, ACTION_KEYREL,   60, 1, 0.5f);
    recorder::rec(0, ACTION_KEYPRESS, 70, 6, 0.3f);
    recorder::rec(0, ACTION_KEYREL,   80, 1, 0.5f);
    recorder::rec(1, ACTION_KEYPRESS, 100, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   120, 1, 0.5f);

    /* Delete any action on channel 0 of types KEYPRESS | KEYREL between
    frames 0 and 200. */

    recorder::deleteActions(0, 0, 200, ACTION_KEYPRESS | ACTION_KEYREL, &mutex);

    REQUIRE(recorder::frames.size() == 2);
    REQUIRE(recorder::global.size() == 2);
    REQUIRE(recorder::global.at(0).size() == 1);
    REQUIRE(recorder::global.at(1).size() == 1);

    REQUIRE(recorder::global.at(0).at(0)->chan == 1);
    REQUIRE(recorder::global.at(0).at(0)->type == ACTION_KEYPRESS);
    REQUIRE(recorder::global.at(0).at(0)->frame == 100);
    REQUIRE(recorder::global.at(0).at(0)->iValue == 6);
    REQUIRE(recorder::global.at(0).at(0)->fValue == 0.3f);

    REQUIRE(recorder::global.at(1).at(0)->chan == 1);
    REQUIRE(recorder::global.at(1).at(0)->type == ACTION_KEYREL);
    REQUIRE(recorder::global.at(1).at(0)->frame == 120);
    REQUIRE(recorder::global.at(1).at(0)->iValue == 1);
    REQUIRE(recorder::global.at(1).at(0)->fValue == 0.5f);
  }

  SECTION("Test action presence")
  {
    recorder::rec(0, ACTION_KEYREL,   80, 1, 0.5f);
    recorder::rec(1, ACTION_KEYPRESS, 100, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   120, 1, 0.5f);

    recorder::deleteAction(0, 80, ACTION_KEYREL, false, &mutex);

    REQUIRE(recorder::hasActions(0) == false);
    REQUIRE(recorder::hasActions(1) == true);
  }

  SECTION("Test clear actions by channel")
  {
    recorder::rec(0, ACTION_KEYREL,   80, 1, 0.5f);
    recorder::rec(1, ACTION_KEYPRESS, 100, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   120, 1, 0.5f);

    recorder::clearChan(1);

    REQUIRE(recorder::hasActions(0) == true);
    REQUIRE(recorder::hasActions(1) == false);
    REQUIRE(recorder::frames.size() == 1);
    REQUIRE(recorder::global.size() == 1);
    REQUIRE(recorder::global.at(0).size() == 1);
  }

  SECTION("Test clear actions by type")
  {
    recorder::rec(1, ACTION_KEYREL,   80, 1, 0.5f);
    recorder::rec(0, ACTION_KEYPRESS, 100, 6, 0.3f);
    recorder::rec(1, ACTION_KEYREL,   120, 1, 0.5f);

    /* Clear all actions of type KEYREL from channel 1. */

    recorder::clearAction(1, ACTION_KEYREL);

    REQUIRE(recorder::hasActions(0) == true);
    REQUIRE(recorder::hasActions(1) == false);
    REQUIRE(recorder::frames.size() == 1);
    REQUIRE(recorder::global.size() == 1);
    REQUIRE(recorder::global.at(0).size() == 1);
  }


  // TODO clearAll
  // TODO optimize
  // TODO sortActions
  // TODO updateBpm
  // TODO updateSamplerate
  // TODO expand
  // TODO shrink
  // TODO startOverdub
  // TODO stopOverdub

}
