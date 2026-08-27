import type { OrientedFeature, ParametricExpression, ParametricMatrix12, ParametricPoint2, ParametricVec3 } from "./parametric-types.js";
import type { Vec3 } from "./types.js";
import { evaluateExpression } from "./parametric-expressions.js";

export function expressionValue(expression: ParametricExpression, parameters: Readonly<Record<string, number>>): number {
  return evaluateExpression(expression, parameters);
}

export function vectorValue(value: ParametricVec3, parameters: Readonly<Record<string, number>>): Vec3 {
  if (!Array.isArray(value) || value.length !== 3) throw new TypeError("Parametric vector must contain three expressions");
  return [
    expressionValue(value[0], parameters),
    expressionValue(value[1], parameters),
    expressionValue(value[2], parameters),
  ];
}

export function point2Value(value: ParametricPoint2, parameters: Readonly<Record<string, number>>): [number, number] {
  if (!Array.isArray(value) || value.length !== 2) throw new TypeError("Parametric point must contain two expressions");
  return [expressionValue(value[0], parameters), expressionValue(value[1], parameters)];
}

export function matrixValue(value: ParametricMatrix12, parameters: Readonly<Record<string, number>>): [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
] {
  if (!Array.isArray(value) || value.length !== 12) throw new TypeError("Parametric matrix must contain twelve expressions");
  return value.map((entry) => expressionValue(entry, parameters)) as [
    number, number, number, number,
    number, number, number, number,
    number, number, number, number,
  ];
}

export function orientedOptions(value: OrientedFeature, parameters: Readonly<Record<string, number>>): { origin?: Vec3; direction?: Vec3 } {
  return {
    ...(value.origin === undefined ? {} : { origin: vectorValue(value.origin, parameters) }),
    ...(value.direction === undefined ? {} : { direction: vectorValue(value.direction, parameters) }),
  };
}
