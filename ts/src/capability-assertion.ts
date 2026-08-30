import type { OperationName } from "./generated.js";

export function assertOperationSet(
  context: string,
  declared: readonly OperationName[],
  registered: readonly OperationName[],
): void {
  const declaredSet = new Set(declared);
  const registeredSet = new Set(registered);
  const missing = [...declaredSet].filter((operation) => !registeredSet.has(operation)).sort();
  const extra = [...registeredSet].filter((operation) => !declaredSet.has(operation)).sort();
  if (missing.length === 0 && extra.length === 0) return;
  const list = (operations: readonly OperationName[]): string => (
    operations.length === 0 ? "(none)" : operations.join(", ")
  );
  throw new Error(
    `${context} capability mismatch; declared but not registered: ${list(missing)}; registered but not declared: ${list(extra)}`,
  );
}
