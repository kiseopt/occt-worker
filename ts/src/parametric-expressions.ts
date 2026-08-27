import { PARAMETRIC_SCHEMA_VERSION } from "./parametric-types.js";
import type { ParametricDefinition, ParametricExpression } from "./parametric-types.js";

type ArithmeticNode =
  | { type: "number"; value: number }
  | { type: "name"; name: string }
  | { type: "unary"; operator: "+" | "-"; value: ArithmeticNode }
  | { type: "binary"; operator: "+" | "-" | "*" | "/" | "^"; left: ArithmeticNode; right: ArithmeticNode };

class ArithmeticParser {
  readonly #source: string;
  #offset = 0;

  constructor(source: string) {
    this.#source = source;
  }

  parse(): ArithmeticNode {
    const result = this.#parseAddition();
    this.#skipWhitespace();
    if (this.#offset !== this.#source.length) this.#fail("Unexpected token");
    return result;
  }

  #parseAddition(): ArithmeticNode {
    let left = this.#parseMultiplication();
    while (true) {
      if (this.#consume("+")) left = { type: "binary", operator: "+", left, right: this.#parseMultiplication() };
      else if (this.#consume("-")) left = { type: "binary", operator: "-", left, right: this.#parseMultiplication() };
      else return left;
    }
  }

  #parseMultiplication(): ArithmeticNode {
    let left = this.#parseUnary();
    while (true) {
      if (this.#consume("*")) left = { type: "binary", operator: "*", left, right: this.#parseUnary() };
      else if (this.#consume("/")) left = { type: "binary", operator: "/", left, right: this.#parseUnary() };
      else return left;
    }
  }

  #parseUnary(): ArithmeticNode {
    if (this.#consume("+")) return { type: "unary", operator: "+", value: this.#parseUnary() };
    if (this.#consume("-")) return { type: "unary", operator: "-", value: this.#parseUnary() };
    return this.#parsePower();
  }

  #parsePower(): ArithmeticNode {
    const left = this.#parsePrimary();
    return this.#consume("^")
      ? { type: "binary", operator: "^", left, right: this.#parseUnary() }
      : left;
  }

  #parsePrimary(): ArithmeticNode {
    this.#skipWhitespace();
    if (this.#consume("(")) {
      const value = this.#parseAddition();
      if (!this.#consume(")")) this.#fail("Expected ')'");
      return value;
    }
    const remaining = this.#source.slice(this.#offset);
    const number = /^(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?/.exec(remaining)?.[0];
    if (number !== undefined) {
      this.#offset += number.length;
      return { type: "number", value: Number(number) };
    }
    const name = /^[A-Za-z_][A-Za-z0-9_]*/.exec(remaining)?.[0];
    if (name !== undefined) {
      this.#offset += name.length;
      return { type: "name", name };
    }
    this.#fail("Expected a number, parameter, or '('");
  }

  #consume(token: string): boolean {
    this.#skipWhitespace();
    if (!this.#source.startsWith(token, this.#offset)) return false;
    this.#offset += token.length;
    return true;
  }

  #skipWhitespace(): void {
    while (/\s/.test(this.#source[this.#offset] ?? "")) this.#offset++;
  }

  #fail(message: string): never {
    throw new SyntaxError(`${message} at offset ${this.#offset} in "${this.#source}"`);
  }
}

function evaluateNode(node: ArithmeticNode, resolve: (name: string) => number): number {
  let value: number;
  if (node.type === "number") value = node.value;
  else if (node.type === "name") {
    if (node.name === "pi") value = Math.PI;
    else if (node.name === "e") value = Math.E;
    else value = resolve(node.name);
  } else if (node.type === "unary") {
    value = node.operator === "-" ? -evaluateNode(node.value, resolve) : evaluateNode(node.value, resolve);
  } else {
    const left = evaluateNode(node.left, resolve);
    const right = evaluateNode(node.right, resolve);
    if (node.operator === "+") value = left + right;
    else if (node.operator === "-") value = left - right;
    else if (node.operator === "*") value = left * right;
    else if (node.operator === "/") {
      if (right === 0) throw new RangeError("Division by zero in parametric expression");
      value = left / right;
    } else value = left ** right;
  }
  if (!Number.isFinite(value)) throw new RangeError("Parametric expression produced a non-finite number");
  return value;
}

function parseArithmetic(expression: ParametricExpression): ArithmeticNode {
  if (typeof expression === "number") {
    if (!Number.isFinite(expression)) throw new RangeError("Parametric values must be finite");
    return { type: "number", value: expression };
  }
  if (expression.trim() === "") throw new SyntaxError("Parametric expression cannot be empty");
  return new ArithmeticParser(expression).parse();
}

export function evaluateExpression(
  expression: ParametricExpression,
  parameters: Readonly<Record<string, number>> = {},
): number {
  return evaluateNode(parseArithmetic(expression), (name) => {
    if (!Object.prototype.hasOwnProperty.call(parameters, name)) throw new ReferenceError(`Unknown parameter "${name}"`);
    const value = parameters[name]!;
    return value;
  });
}

export function assertParameterName(name: string): void {
  if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name) || name === "pi" || name === "e"
      || name === "__proto__" || name === "prototype" || name === "constructor") {
    throw new TypeError(`Invalid parameter name "${name}"`);
  }
}

export function resolveParameters(
  definitions: Readonly<Record<string, ParametricExpression>>,
): Readonly<Record<string, number>> {
  const nodes = new Map<string, ArithmeticNode>();
  for (const [name, expression] of Object.entries(definitions)) {
    assertParameterName(name);
    nodes.set(name, parseArithmetic(expression));
  }
  const values = new Map<string, number>();
  const visiting: string[] = [];
  const resolve = (name: string): number => {
    const known = values.get(name);
    if (known !== undefined) return known;
    const node = nodes.get(name);
    if (node === undefined) {
      const owner = visiting.at(-1);
      throw new ReferenceError(owner === undefined
        ? `Unknown parameter "${name}"`
        : `Unknown parameter "${name}" referenced by "${owner}"`);
    }
    const cycleStart = visiting.indexOf(name);
    if (cycleStart !== -1) {
      throw new Error(`Parameter dependency cycle: ${[...visiting.slice(cycleStart), name].join(" -> ")}`);
    }
    visiting.push(name);
    try {
      const value = evaluateNode(node, resolve);
      values.set(name, value);
      return value;
    } finally {
      visiting.pop();
    }
  };
  for (const name of nodes.keys()) resolve(name);
  return Object.freeze(Object.fromEntries(values));
}

export function cloneDefinition(definition: ParametricDefinition): ParametricDefinition {
  return structuredClone(definition);
}

export function migrateDefinition(definition: ParametricDefinition): ParametricDefinition {
  if (definition.schemaVersion !== undefined
      && definition.schemaVersion !== PARAMETRIC_SCHEMA_VERSION) {
    throw new RangeError(`Unsupported parametric schema version: ${String(definition.schemaVersion)}`);
  }
  return { ...cloneDefinition(definition), schemaVersion: PARAMETRIC_SCHEMA_VERSION };
}
