#pragma once

#include "../include/doctest.h"
#include "../src/lookup_tables.hpp"
#include "../src/zobrist.hpp"

struct CustomTestFixture {
	CustomTestFixture() {
		LookupTables::init();
		Zobrist::init();
	}
};

#define CUSTOM_TEST_CASE(name) TEST_CASE_FIXTURE(CustomTestFixture, name)
