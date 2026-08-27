#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

std::pair<UnitsMethods_LengthUnit, double> stepLengthUnit(const json& theArgs);
DESTEP_Parameters::WriteMode_StepSchema stepWriteSchema(const json& theArgs);
std::tuple<std::string, double, UnitsMethods_LengthUnit> igesLengthUnit(const json& theArgs);

} // namespace occt_worker
