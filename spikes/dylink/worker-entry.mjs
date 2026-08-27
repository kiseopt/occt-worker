// occt-worker dylink spike - Worker entry shared by browser Workers and Node
// worker_threads. Protocol:
//   -> {type:"init", epoch, mainJsUrl, mainWasmUrl, sideUrl}
//   <- {type:"ready", epoch, info}
//   -> {type:"call", callId, op:"identity"|"eh_roundtrip"|"trap"|"host_pages", input}
//   <- {type:"response", callId, ok, value?} | {type:"fatal", callId, error}
//   -> {type:"grow", callId, pages}          <- {type:"response", callId, ok, value}
//   -> {type:"reload-side", callId, sideUrl} <- {type:"response", callId, ok, registrations}
//   -> {type:"bad-load", callId, sideUrl}    <- {type:"response", callId, ok:false, error}
//   -> {type:"dispose"}                      <- worker terminates itself

const isBrowserWorker = typeof importScripts === "function";

let post;
let moduleInstance = undefined;
let pluginId = -1;

function respond(message) {
  post(message);
}

async function handleInit(message) {
  const factory = (await import(message.mainJsUrl)).default;
  moduleInstance = await factory({
    locateFile: (path) => {
      if (path === "dylink-main.wasm") return message.mainWasmUrl;
      return path;
    },
  });
  // Side ctor auto-registration runs during loadDynamicLibrary.
  await moduleInstance.loadDynamicLibrary(message.sideUrl, {
    loadAsync: true,
    global: true,
    nodelete: true,
  });
  pluginId = moduleInstance._plugin_registration_count() - 1;
  if (pluginId < 0) throw new Error(`plugin registration failed: ${moduleInstance._host_last_error()}`);
  const namePointer = moduleInstance._plugin_descriptor_name(pluginId);
  const descriptorName = moduleInstance.UTF8ToString(namePointer);
  const buildFamily = moduleInstance.UTF8ToString(moduleInstance._host_build_family());
  respond({
    type: "ready",
    epoch: message.epoch,
    info: {
      pluginId,
      descriptorName,
      abiVersion: moduleInstance._plugin_descriptor_abi_version(pluginId),
      buildFamilyMatches: buildFamily === "spike-dylink-v1",
      registrations: moduleInstance._plugin_registration_count(),
      hostPages: moduleInstance._host_memory_pages(),
    },
  });
}

function dispatch(op, input) {
  switch (op) {
    case "identity": return moduleInstance._host_dispatch(pluginId, 0, input);
    case "eh_roundtrip": return moduleInstance._host_dispatch(pluginId, 1, input);
    case "trap": return moduleInstance._host_dispatch(pluginId, 2, input);
    case "host_pages": return moduleInstance._host_dispatch(pluginId, 3, input);
    default: throw new Error(`unknown op ${op}`);
  }
}

async function handleMessage(message) {
  try {
    switch (message.type) {
      case "init": {
        await handleInit(message);
        break;
      }
      case "call": {
        const value = dispatch(message.op, message.input ?? 0);
        respond({ type: "response", callId: message.callId, ok: true, value });
        break;
      }
      case "grow": {
        const before = moduleInstance._host_memory_pages();
        const result = moduleInstance._host_grow_pages(message.pages);
        const after = moduleInstance._host_memory_pages();
        respond({
          type: "response",
          callId: message.callId,
          ok: result !== -1 && after > before,
          value: { before, after },
        });
        break;
      }
      case "reload-side": {
        await moduleInstance.loadDynamicLibrary(message.sideUrl, {
          loadAsync: true,
          global: true,
          nodelete: true,
        });
        respond({
          type: "response",
          callId: message.callId,
          ok: true,
          registrations: moduleInstance._plugin_registration_count(),
        });
        break;
      }
      case "bad-load": {
        try {
          await moduleInstance.loadDynamicLibrary(message.sideUrl, {
            loadAsync: true,
            global: true,
            nodelete: true,
          });
          respond({ type: "response", callId: message.callId, ok: false, error: "bad load unexpectedly succeeded" });
        } catch (error) {
          respond({
            type: "response",
            callId: message.callId,
            ok: true,
            errorKind: error instanceof Error ? error.constructor.name : typeof error,
            errorMessage: String(error && error.message !== undefined ? error.message : error).slice(0, 200),
          });
        }
        break;
      }
      default:
        respond({ type: "error", error: `unknown message type ${message.type}` });
    }
  } catch (error) {
    if (error instanceof WebAssembly.RuntimeError) {
      respond({ type: "fatal", callId: message.callId ?? null, error: String(error.message || error) });
    } else {
      respond({
        type: "response",
        callId: message.callId ?? null,
        ok: false,
        error: String(error && error.message !== undefined ? error.message : error).slice(0, 300),
      });
    }
  }
}

if (isBrowserWorker) {
  post = (message) => self.postMessage(message);
  self.addEventListener("message", (event) => handleMessage(event.data));
} else {
  const { parentPort } = await import("node:worker_threads");
  post = (message) => parentPort.postMessage(message);
  parentPort.on("message", (message) => handleMessage(message));
}
