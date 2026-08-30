import test from "node:test";
import assert from "node:assert/strict";
import { EngineError, SingleRuntimeSession } from "../../dist/index.js";

test("single runtime session closes before creating the replacement and invalidates requests", async () => {
  const events = [];
  let active = 0;
  const make = (name) => async () => {
    events.push(`create:${name}`);
    active += 1;
    return {
      request: async () => new Promise((resolve) => setTimeout(resolve, 5)),
      close: async () => { events.push(`close:${name}`); active -= 1; },
    };
  };
  const session = new SingleRuntimeSession({ first: make("first"), second: make("second") });
  await session.switchProfile("first");
  const request = session.request("stats", {});
  const switching = session.switchProfile("second");
  await assert.rejects(request, (error) => error instanceof EngineError && error.code === "InvalidPlacementState");
  await switching;
  assert.deepEqual(events, ["create:first", "close:first", "create:second"]);
  assert.equal(active, 1);
  await session.close();
  assert.equal(active, 0);
});

test("failed switch leaves the session closed", async () => {
  let active = 1;
  const oldHandle = { valid: true };
  const runtime = {
    request: async () => 1,
    close: async () => {
      active -= 1;
      oldHandle.valid = false;
    },
  };
  const session = new SingleRuntimeSession({ first: async () => runtime, broken: async () => { throw new Error("load failed"); } });
  await session.switchProfile("first");
  await assert.rejects(session.switchProfile("broken"), /load failed/);
  assert.equal(session.closed, true);
  assert.equal(session.profileId, undefined);
  assert.equal(active, 0);
  assert.equal(oldHandle.valid, false);
  await assert.rejects(session.request("stats"), (error) => error.code === "InvalidPlacementState");
  await assert.rejects(session.switchProfile("first"), /session is closed/);
});
