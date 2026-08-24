export type SketchPoint = readonly [number, number];

export type SketchEntity =
  | { id: string; type: "line"; start: SketchPoint; end: SketchPoint; construction?: boolean }
  | { id: string; type: "circle"; center: SketchPoint; radius: number; construction?: boolean }
  | {
      id: string;
      type: "arc";
      center: SketchPoint;
      radius: number;
      startAngle: number;
      endAngle: number;
      construction?: boolean;
    }
  | { id: string; type: "spline"; poles: readonly SketchPoint[]; degree?: number; construction?: boolean };

export type SketchPointReference = {
  entity: string;
  point: "start" | "end" | "center";
};

export type SketchConstraint =
  | { type: "coincident"; first: SketchPointReference; second: SketchPointReference }
  | { type: "fixed"; point: SketchPointReference; value: SketchPoint }
  | { type: "horizontal"; entity: string }
  | { type: "vertical"; entity: string }
  | { type: "distance"; first: SketchPointReference; second: SketchPointReference; value: number }
  | { type: "length"; entity: string; value: number }
  | { type: "radius"; entity: string; value: number }
  | { type: "parallel"; first: string; second: string }
  | { type: "perpendicular"; first: string; second: string }
  | { type: "equalLength"; first: string; second: string }
  | { type: "equalRadius"; first: string; second: string }
  | {
      /** Tangency between line, circle, or arc entities; BSpline tangency is not defined. */
      type: "tangent";
      first: string;
      second: string;
    }
  | { type: "concentric"; first: string; second: string }
  | { type: "symmetry"; first: SketchPointReference; second: SketchPointReference; axis: string };

export interface SketchDefinition {
  entities: readonly SketchEntity[];
  constraints?: readonly SketchConstraint[];
}

export interface SketchSolveOptions {
  tolerance?: number;
  maxIterations?: number;
}

export interface SketchSolveDiagnostics {
  converged: boolean;
  iterations: number;
  maxResidual: number;
  degreesOfFreedom: number;
  constraintDiagnostics: readonly SketchConstraintDiagnostic[];
}

export interface SketchConstraintDiagnostic {
  index: number;
  maxResidual: number;
  satisfied: boolean;
}

export interface SolvedSketch {
  entities: SketchEntity[];
  diagnostics: SketchSolveDiagnostics;
}

interface EntityLayout {
  entity: SketchEntity;
  offset: number;
}

function solveLinearSystem(matrix: number[][], vector: number[]): number[] | undefined {
  const size = vector.length;
  const augmented = matrix.map((row, index) => [...row, vector[index]!]);
  for (let column = 0; column < size; column++) {
    let pivot = column;
    for (let row = column + 1; row < size; row++) {
      if (Math.abs(augmented[row]![column]!) > Math.abs(augmented[pivot]![column]!)) pivot = row;
    }
    if (Math.abs(augmented[pivot]![column]!) < 1e-14) return undefined;
    [augmented[column], augmented[pivot]] = [augmented[pivot]!, augmented[column]!];
    const divisor = augmented[column]![column]!;
    for (let entry = column; entry <= size; entry++) augmented[column]![entry] = augmented[column]![entry]! / divisor;
    for (let row = 0; row < size; row++) {
      if (row === column) continue;
      const factor = augmented[row]![column]!;
      if (factor === 0) continue;
      for (let entry = column; entry <= size; entry++) {
        augmented[row]![entry] = augmented[row]![entry]! - factor * augmented[column]![entry]!;
      }
    }
  }
  return augmented.map((row) => row[size]!);
}

function matrixRank(matrix: number[][], tolerance: number): number {
  if (matrix.length === 0) return 0;
  const copy = matrix.map((row) => [...row]);
  const columns = copy[0]!.length;
  let rank = 0;
  for (let column = 0; column < columns && rank < copy.length; column++) {
    let pivot = rank;
    for (let row = rank + 1; row < copy.length; row++) {
      if (Math.abs(copy[row]![column]!) > Math.abs(copy[pivot]![column]!)) pivot = row;
    }
    if (Math.abs(copy[pivot]![column]!) <= tolerance) continue;
    [copy[rank], copy[pivot]] = [copy[pivot]!, copy[rank]!];
    const divisor = copy[rank]![column]!;
    for (let entry = column; entry < columns; entry++) copy[rank]![entry] = copy[rank]![entry]! / divisor;
    for (let row = rank + 1; row < copy.length; row++) {
      const factor = copy[row]![column]!;
      for (let entry = column; entry < columns; entry++) {
        copy[row]![entry] = copy[row]![entry]! - factor * copy[rank]![entry]!;
      }
    }
    rank++;
  }
  return rank;
}

export function solveSketch(definition: SketchDefinition, options: SketchSolveOptions = {}): SolvedSketch {
  if (!Array.isArray(definition.entities) || definition.entities.length === 0) {
    throw new TypeError("Sketch requires at least one entity");
  }
  const tolerance = options.tolerance ?? 1e-8;
  const maxIterations = options.maxIterations ?? 100;
  if (!(tolerance > 0) || !Number.isFinite(tolerance)) throw new RangeError("Sketch tolerance must be positive and finite");
  if (!Number.isInteger(maxIterations) || maxIterations <= 0) throw new RangeError("Sketch maxIterations must be a positive integer");

  const layouts = new Map<string, EntityLayout>();
  const values: number[] = [];
  for (const entity of definition.entities) {
    if (!/^[A-Za-z_][A-Za-z0-9_-]*$/.test(entity.id)) throw new TypeError(`Invalid sketch entity id "${entity.id}"`);
    if (layouts.has(entity.id)) throw new Error(`Duplicate sketch entity id "${entity.id}"`);
    const offset = values.length;
    if (entity.type === "line") values.push(...entity.start, ...entity.end);
    else if (entity.type === "circle") values.push(...entity.center, entity.radius);
    else if (entity.type === "arc") values.push(...entity.center, entity.radius, entity.startAngle, entity.endAngle);
    else if (entity.type === "spline") {
      if (!Number.isInteger(entity.degree ?? Math.min(3, entity.poles.length - 1))
        || (entity.degree ?? Math.min(3, entity.poles.length - 1)) < 1
        || (entity.degree ?? Math.min(3, entity.poles.length - 1)) >= entity.poles.length) {
        throw new RangeError(`Sketch spline "${entity.id}" degree must be between 1 and poleCount - 1`);
      }
      if (entity.poles.length < 2) throw new RangeError(`Sketch spline "${entity.id}" requires at least two poles`);
      for (const pole of entity.poles) values.push(...pole);
    }
    else throw new TypeError(`Unsupported sketch entity type "${String((entity as { type?: unknown }).type)}"`);
    if (values.slice(offset).some((value) => !Number.isFinite(value))) throw new RangeError(`Sketch entity "${entity.id}" must be finite`);
    if ((entity.type === "circle" || entity.type === "arc") && !(entity.radius > 0)) {
      throw new RangeError(`Sketch ${entity.type} "${entity.id}" radius must be positive`);
    }
    layouts.set(entity.id, { entity, offset });
  }

  const layout = (id: string): EntityLayout => {
    const result = layouts.get(id);
    if (result === undefined) throw new ReferenceError(`Unknown sketch entity "${id}"`);
    return result;
  };
  const point = (variables: readonly number[], reference: SketchPointReference): [number, number] => {
    if (reference.point !== "start" && reference.point !== "end" && reference.point !== "center") {
      throw new TypeError(`Unsupported sketch point reference "${String((reference as { point?: unknown }).point)}"`);
    }
    const item = layout(reference.entity);
    if (reference.point === "center") {
      if (item.entity.type !== "circle" && item.entity.type !== "arc") {
        throw new TypeError(`Sketch entity "${reference.entity}" has no center point`);
      }
      return [variables[item.offset]!, variables[item.offset + 1]!];
    }
    if (item.entity.type === "line") {
      const offset = item.offset + (reference.point === "end" ? 2 : 0);
      return [variables[offset]!, variables[offset + 1]!];
    }
    if (item.entity.type === "arc") {
      const angle = variables[item.offset + (reference.point === "start" ? 3 : 4)]!;
      return [
        variables[item.offset]! + variables[item.offset + 2]! * Math.cos(angle),
        variables[item.offset + 1]! + variables[item.offset + 2]! * Math.sin(angle),
      ];
    }
    if (item.entity.type !== "spline") throw new TypeError(`Sketch entity "${reference.entity}" has no ${reference.point} point`);
    const poleOffset = item.offset + (reference.point === "start" ? 0 : (item.entity.poles.length - 1) * 2);
    return [variables[poleOffset]!, variables[poleOffset + 1]!];
  };
  const line = (variables: readonly number[], id: string): [number, number, number, number] => {
    const item = layout(id);
    if (item.entity.type !== "line") throw new TypeError(`Sketch entity "${id}" must be a line`);
    return [variables[item.offset]!, variables[item.offset + 1]!, variables[item.offset + 2]!, variables[item.offset + 3]!];
  };
  const radius = (variables: readonly number[], id: string): number => {
    const item = layout(id);
    if (item.entity.type !== "circle" && item.entity.type !== "arc") throw new TypeError(`Sketch entity "${id}" must be a circle or arc`);
    return variables[item.offset + 2]!;
  };
  const lineLength = (variables: readonly number[], id: string): number => {
    const [x1, y1, x2, y2] = line(variables, id);
    return Math.hypot(x2 - x1, y2 - y1);
  };
  const circleCenter = (variables: readonly number[], id: string): [number, number] => {
    const item = layout(id);
    if (item.entity.type !== "circle" && item.entity.type !== "arc") {
      throw new TypeError(`Sketch entity "${id}" must be a circle or arc`);
    }
    return [variables[item.offset]!, variables[item.offset + 1]!];
  };
  const circularRadius = (variables: readonly number[], id: string): number => radius(variables, id);

  const constraints = definition.constraints ?? [];
  const residuals = (variables: readonly number[]): number[] => {
    const result: number[] = [];
    for (const constraint of constraints) {
      if (constraint.type === "coincident") {
        const first = point(variables, constraint.first);
        const second = point(variables, constraint.second);
        result.push(first[0] - second[0], first[1] - second[1]);
      } else if (constraint.type === "fixed") {
        const current = point(variables, constraint.point);
        if (constraint.value.some((value) => !Number.isFinite(value))) throw new RangeError("Sketch fixed coordinates must be finite");
        result.push(current[0] - constraint.value[0], current[1] - constraint.value[1]);
      } else if (constraint.type === "horizontal" || constraint.type === "vertical") {
        const [x1, y1, x2, y2] = line(variables, constraint.entity);
        result.push(constraint.type === "horizontal" ? y2 - y1 : x2 - x1);
      } else if (constraint.type === "distance") {
        if (!(constraint.value >= 0) || !Number.isFinite(constraint.value)) throw new RangeError("Sketch distance must be finite and non-negative");
        const first = point(variables, constraint.first);
        const second = point(variables, constraint.second);
        result.push(Math.hypot(second[0] - first[0], second[1] - first[1]) - constraint.value);
      } else if (constraint.type === "length") {
        if (!(constraint.value > 0) || !Number.isFinite(constraint.value)) throw new RangeError("Sketch length must be positive and finite");
        result.push(lineLength(variables, constraint.entity) - constraint.value);
      } else if (constraint.type === "radius") {
        if (!(constraint.value > 0) || !Number.isFinite(constraint.value)) throw new RangeError("Sketch radius must be positive and finite");
        result.push(radius(variables, constraint.entity) - constraint.value);
      } else if (constraint.type === "equalLength") {
        result.push(lineLength(variables, constraint.first) - lineLength(variables, constraint.second));
      } else if (constraint.type === "equalRadius") {
        result.push(radius(variables, constraint.first) - radius(variables, constraint.second));
      } else if (constraint.type === "concentric") {
        const first = circleCenter(variables, constraint.first);
        const second = circleCenter(variables, constraint.second);
        result.push(first[0] - second[0], first[1] - second[1]);
      } else if (constraint.type === "tangent") {
        const firstLayout = layout(constraint.first);
        const secondLayout = layout(constraint.second);
        const firstIsLine = firstLayout.entity.type === "line";
        const secondIsLine = secondLayout.entity.type === "line";
        const firstIsCircular = firstLayout.entity.type === "circle" || firstLayout.entity.type === "arc";
        const secondIsCircular = secondLayout.entity.type === "circle" || secondLayout.entity.type === "arc";
        if (firstIsLine && secondIsLine) {
          const first = line(variables, constraint.first);
          const second = line(variables, constraint.second);
          const ax = first[2] - first[0];
          const ay = first[3] - first[1];
          const bx = second[2] - second[0];
          const by = second[3] - second[1];
          const scale = Math.hypot(ax, ay) * Math.hypot(bx, by);
          if (scale <= Number.EPSILON) throw new RangeError("Tangent constraints require non-zero lines");
          result.push((ax * by - ay * bx) / scale);
        } else if ((firstIsLine && secondIsCircular) || (secondIsLine && firstIsCircular)) {
          const lineId = firstIsLine ? constraint.first : constraint.second;
          const circleId = firstIsLine ? constraint.second : constraint.first;
          const edge = line(variables, lineId);
          const center = circleCenter(variables, circleId);
          const dx = edge[2] - edge[0];
          const dy = edge[3] - edge[1];
          const length = Math.hypot(dx, dy);
          if (length <= Number.EPSILON) throw new RangeError("Tangent constraints require a non-zero line");
          result.push(Math.abs((center[0] - edge[0]) * dy - (center[1] - edge[1]) * dx) / length
            - circularRadius(variables, circleId));
        } else if (firstIsCircular && secondIsCircular) {
          const first = circleCenter(variables, constraint.first);
          const second = circleCenter(variables, constraint.second);
          result.push(Math.hypot(second[0] - first[0], second[1] - first[1])
            - circularRadius(variables, constraint.first) - circularRadius(variables, constraint.second));
        } else {
          throw new TypeError("Tangent constraints support line, circle, and arc entities");
        }
      } else if (constraint.type === "symmetry") {
        const first = point(variables, constraint.first);
        const second = point(variables, constraint.second);
        const axis = line(variables, constraint.axis);
        const dx = axis[2] - axis[0];
        const dy = axis[3] - axis[1];
        const length = Math.hypot(dx, dy);
        if (length <= Number.EPSILON) throw new RangeError("Symmetry constraints require a non-zero axis line");
        const midpointX = (first[0] + second[0]) / 2;
        const midpointY = (first[1] + second[1]) / 2;
        result.push(((midpointX - axis[0]) * dy - (midpointY - axis[1]) * dx) / length);
        result.push(((second[0] - first[0]) * dx + (second[1] - first[1]) * dy) / length);
      } else if (constraint.type === "parallel" || constraint.type === "perpendicular") {
        const first = line(variables, constraint.first);
        const second = line(variables, constraint.second);
        const ax = first[2] - first[0];
        const ay = first[3] - first[1];
        const bx = second[2] - second[0];
        const by = second[3] - second[1];
        const scale = Math.hypot(ax, ay) * Math.hypot(bx, by);
        if (scale <= Number.EPSILON) throw new RangeError("Parallel and perpendicular constraints require non-zero lines");
        result.push(constraint.type === "parallel" ? (ax * by - ay * bx) / scale : (ax * bx + ay * by) / scale);
      } else {
        throw new TypeError(`Unsupported sketch constraint type "${String((constraint as { type?: unknown }).type)}"`);
      }
    }
    return result;
  };
  const jacobian = (variables: number[], base: number[]): number[][] => {
    const result = base.map(() => new Array<number>(variables.length).fill(0));
    for (let column = 0; column < variables.length; column++) {
      const original = variables[column]!;
      const step = 1e-6 * Math.max(1, Math.abs(original));
      variables[column] = original + step;
      const shifted = residuals(variables);
      variables[column] = original;
      for (let row = 0; row < base.length; row++) result[row]![column] = (shifted[row]! - base[row]!) / step;
    }
    return result;
  };

  let variables = [...values];
  let lambda = 1e-3;
  let iterations = 0;
  for (; iterations < maxIterations; iterations++) {
    const current = residuals(variables);
    if (current.length === 0 || Math.max(...current.map(Math.abs)) <= tolerance) break;
    const derivatives = jacobian(variables, current);
    const normal = Array.from({ length: variables.length }, () => new Array<number>(variables.length).fill(0));
    const gradient = new Array<number>(variables.length).fill(0);
    for (let row = 0; row < current.length; row++) {
      for (let column = 0; column < variables.length; column++) {
        gradient[column] = gradient[column]! - derivatives[row]![column]! * current[row]!;
        for (let other = 0; other < variables.length; other++) {
          normal[column]![other] = normal[column]![other]!
            + derivatives[row]![column]! * derivatives[row]![other]!;
        }
      }
    }
    for (let column = 0; column < variables.length; column++) {
      normal[column]![column] = normal[column]![column]! + lambda;
    }
    const delta = solveLinearSystem(normal, gradient);
    if (delta === undefined) break;
    const candidate = variables.map((value, index) => value + delta[index]!);
    const currentCost = current.reduce((sum, value) => sum + value * value, 0);
    const hasPositiveRadii = [...layouts.values()].every(({ entity, offset }) =>
      (entity.type !== "circle" && entity.type !== "arc") || candidate[offset + 2]! > 0);
    const candidateResiduals = hasPositiveRadii ? residuals(candidate) : [];
    const candidateCost = hasPositiveRadii
      ? candidateResiduals.reduce((sum, value) => sum + value * value, 0)
      : Number.POSITIVE_INFINITY;
    if (candidateCost < currentCost) {
      variables = candidate;
      lambda = Math.max(lambda / 3, 1e-12);
    } else {
      lambda = Math.min(lambda * 10, 1e12);
    }
  }
  const finalResiduals = residuals(variables);
  const finalJacobian = jacobian(variables, finalResiduals);
  const maxResidual = finalResiduals.length === 0 ? 0 : Math.max(...finalResiduals.map(Math.abs));
  let residualOffset = 0;
  const constraintDiagnostics = constraints.map((constraint, index) => {
    const residualCount = constraint.type === "coincident" || constraint.type === "fixed"
      || constraint.type === "concentric" || constraint.type === "symmetry" ? 2 : 1;
    const valuesForConstraint = finalResiduals.slice(residualOffset, residualOffset + residualCount);
    residualOffset += residualCount;
    const constraintResidual = valuesForConstraint.length === 0 ? 0 : Math.max(...valuesForConstraint.map(Math.abs));
    return { index, maxResidual: constraintResidual, satisfied: constraintResidual <= tolerance };
  });
  const entities = definition.entities.map((entity): SketchEntity => {
    const item = layout(entity.id);
    if (entity.type === "line") {
      return {
        id: entity.id,
        type: "line",
        start: [variables[item.offset]!, variables[item.offset + 1]!],
        end: [variables[item.offset + 2]!, variables[item.offset + 3]!],
        ...(entity.construction === undefined ? {} : { construction: entity.construction }),
      };
    }
    if (entity.type === "circle") {
      return {
        id: entity.id,
        type: "circle",
        center: [variables[item.offset]!, variables[item.offset + 1]!],
        radius: variables[item.offset + 2]!,
        ...(entity.construction === undefined ? {} : { construction: entity.construction }),
      };
    }
    if (entity.type === "arc") {
      return {
        id: entity.id,
        type: "arc",
        center: [variables[item.offset]!, variables[item.offset + 1]!],
        radius: variables[item.offset + 2]!,
        startAngle: variables[item.offset + 3]!,
        endAngle: variables[item.offset + 4]!,
        ...(entity.construction === undefined ? {} : { construction: entity.construction }),
      };
    }
    return {
      id: entity.id,
      type: "spline",
      poles: entity.poles.map((_: SketchPoint, index: number) => [variables[item.offset + index * 2]!, variables[item.offset + index * 2 + 1]!]),
      ...(entity.degree === undefined ? {} : { degree: entity.degree }),
      ...(entity.construction === undefined ? {} : { construction: entity.construction }),
    };
  });
  return {
    entities,
    diagnostics: {
      converged: maxResidual <= tolerance,
      iterations,
      maxResidual,
      degreesOfFreedom: Math.max(0, variables.length - matrixRank(finalJacobian, tolerance * 10)),
      constraintDiagnostics,
    },
  };
}
