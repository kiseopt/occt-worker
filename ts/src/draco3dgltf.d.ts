declare module "draco3dgltf" {
  interface DracoModuleFactory {
    createDecoderModule(options?: Record<string, unknown>): Promise<any>;
  }

  const factory: DracoModuleFactory;
  export default factory;
}
