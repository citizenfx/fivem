#pragma once

uint64_t ROSGetDummyAccountID();

// note: don't *ever* use a real account ID here
#define ROS_DUMMY_ACCOUNT_ID ROSGetDummyAccountID()