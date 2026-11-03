#pragma once
#include <memory>
#include "data/Repository.hpp"
#include "domain/Assignment.hpp"
#include "domain/AuditLogEntry.hpp"
#include "domain/Department.hpp"
#include "domain/Extension.hpp"
#include "domain/User.hpp"

using UserRepository = Repository<std::unique_ptr<User>, int>;
using DepartmentRepository = Repository<Department, int>;
using AssignmentRepository = Repository<Assignment, int>;
using ExtensionRepository = Repository<Extension, int>;
using AuditLogRepository = Repository<AuditLogEntry, int>;
