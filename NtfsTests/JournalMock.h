#pragma once

#include <gmock\gmock.h>
#include "..\ChangeJournal\Journal.h"

class MockJournal : IJournal {
public:
	MOCK_METHOD0(getHandle, const HANDLE());
	MOCK_METHOD0(getData, const PUSN_JOURNAL_DATA());
	MOCK_METHOD0(getError, int());
};