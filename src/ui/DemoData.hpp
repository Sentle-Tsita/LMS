#pragma once
#include "services/AccountService.hpp"
#include "services/AssignmentService.hpp"

// Creates sample accounts, a course and an assignment so the system can be tried at once.
// Every demo account has the password "Password123".
void seedDemoData(AccountService& accounts, AssignmentService& assignments);
