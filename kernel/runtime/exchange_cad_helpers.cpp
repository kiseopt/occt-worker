#include "kernel_exchange_cad_helpers.hpp"

namespace occt_worker {

std::pair<UnitsMethods_LengthUnit, double> stepLengthUnit(const json& theArgs)
{
  const std::string aName = theArgs.value("unit", "mm");
  if (aName == "mm") return {UnitsMethods_LengthUnit_Millimeter, 1.0};
  if (aName == "cm") return {UnitsMethods_LengthUnit_Centimeter, 10.0};
  if (aName == "m") return {UnitsMethods_LengthUnit_Meter, 1000.0};
  if (aName == "inch") return {UnitsMethods_LengthUnit_Inch, 25.4};
  if (aName == "foot") return {UnitsMethods_LengthUnit_Foot, 304.8};
  throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported STEP length unit: " + aName);
}

DESTEP_Parameters::WriteMode_StepSchema stepWriteSchema(const json& theArgs)
{
  const std::string aName = theArgs.value("schema", "AP242");
  if (aName == "AP203") return DESTEP_Parameters::WriteMode_StepSchema_AP203;
  if (aName == "AP214") return DESTEP_Parameters::WriteMode_StepSchema_AP214IS;
  if (aName == "AP242") return DESTEP_Parameters::WriteMode_StepSchema_AP242DIS;
  throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported STEP schema: " + aName);
}

std::tuple<std::string, double, UnitsMethods_LengthUnit> igesLengthUnit(const json& theArgs)
{
  const std::string aName = theArgs.value("unit", "mm");
  if (aName == "mm") return {"MM", 1.0, UnitsMethods_LengthUnit_Millimeter};
  if (aName == "cm") return {"CM", 10.0, UnitsMethods_LengthUnit_Centimeter};
  if (aName == "m") return {"M", 1000.0, UnitsMethods_LengthUnit_Meter};
  if (aName == "inch") return {"INCH", 25.4, UnitsMethods_LengthUnit_Inch};
  if (aName == "foot") return {"FT", 304.8, UnitsMethods_LengthUnit_Foot};
  throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported IGES length unit: " + aName);
}

} // namespace occt_worker
