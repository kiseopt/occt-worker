import {
  createProfileClient,
  type WorkerProfileHostOptions,
} from "../../ts/src/index.js";

declare const options: WorkerProfileHostOptions;

async function assertProfileClients(): Promise<void> {
  const previewClient = await createProfileClient("preview", options);
  const fullClient = await createProfileClient("full-profile", options);
  const previewScope = await previewClient.beginScope();
  const fullScope = await fullClient.beginScope();

  // @ts-expect-error The preview profile does not include modeling operations.
  previewScope.fillet;
  fullScope.fillet;
  previewClient.tessellate;
  previewClient.bbox;

  // @ts-expect-error The preview profile does not include IGES document exchange.
  previewClient.exportIGES;
  fullClient.exportIGES;
}

void assertProfileClients;
