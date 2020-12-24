#include "../src/core/rcuList.h"
#include "../src/core/types.h"
#include <catch2/catch.hpp>


using namespace giada;
using namespace giada::m;


TEST_CASE("RCUList")
{
	struct Object
	{
		Object(ID id) : id(id) {}
		ID id;
	};

	RCUList<Object> list;

    REQUIRE(list.size() == 0);
    REQUIRE(list.changed == false);

	SECTION("test push")
  	{
		list.push(std::make_unique<Object>(1));
		list.push(std::make_unique<Object>(2));
		list.push(std::make_unique<Object>(3));

    	REQUIRE(list.size() == 3);
    	REQUIRE(list.changed == true);

		SECTION("test retrieval")
		{
			RCUList<Object>::Lock l(list);

			REQUIRE(list.get(0)->id == 1);
			REQUIRE(list.get(1)->id == 2);
			REQUIRE(list.get(2)->id == 3);
			REQUIRE(list[0]->id == 1);
			REQUIRE(list[1]->id == 2);
			REQUIRE(list[2]->id == 3);
			REQUIRE(list.back()->id == 3);
		}

		SECTION("test iterator")
		{
			RCUList<Object>::Lock l(list);

			ID id = 1;
			for (Object* o : list)
				REQUIRE(o->id == id++);
		}
		
		SECTION("test const iterator")
		{
			RCUList<Object>::Lock l(list);

			ID id = 1;
			for (const Object* o : list)
				REQUIRE(o->id == id++);
		}

		SECTION("test pop")
		{
			list.pop(0);

			REQUIRE(list.size() == 2);
			REQUIRE(list.changed == true);
		}

		SECTION("test clear")
		{
			list.clear();

			REQUIRE(list.size() == 0);
			REQUIRE(list.changed == true);
		}
	}

	SECTION("test swap")
	{
		list.push(std::make_unique<Object>(1));

		list.swap(std::make_unique<Object>(16));

		REQUIRE(list.size() == 1);
		REQUIRE(list.changed == true);

		RCUList<Object>::Lock l(list);
		
		REQUIRE(list.get(0)->id == 16);
	}
}
