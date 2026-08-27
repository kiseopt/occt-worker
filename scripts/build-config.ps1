function Get-OcctWasmBuildConfig {
  $repo = Split-Path -Parent $PSScriptRoot
  $repoPosix = $repo.Replace('\', '/')
  $identity = Get-Content -Raw (Join-Path $repo 'protocol/artifacts.json') | ConvertFrom-Json
  return @{
    EmsdkVersion = $identity.sources.emsdk.version
    OcctCxxFlags = @(
      '-fwasm-exceptions'
      '-ffp-contract=off'
      '-UOCC_CONVERT_SIGNALS'
      "-ffile-prefix-map=$repoPosix=."
    )
    OcctCMakeArguments = @(
      '-DBUILD_LIBRARY_TYPE=Static'
      '-DBUILD_RELEASE_DISABLE_EXCEPTIONS=OFF'
      '-DBUILD_ADDITIONAL_TOOLKITS=TKPrim;TKBO;TKBool;TKMesh;TKFillet;TKOffset;TKXSBase;TKDESTEP;TKDEIGES;TKDESTL;TKDEVRML;TKBinXCAF;TKXmlXCAF'
      '-DBUILD_DOC_Overview=OFF'
      '-DBUILD_DOC_RefMan=OFF'
      '-DBUILD_GTEST=OFF'
      '-DUSE_TBB=OFF'
      '-DUSE_OPENGL=OFF'
      '-DUSE_GLES2=OFF'
      '-DUSE_FREETYPE=OFF'
      '-DBUILD_MODULE_FoundationClasses=OFF'
      '-DBUILD_MODULE_ModelingData=OFF'
      '-DBUILD_MODULE_ModelingAlgorithms=ON'
      '-DBUILD_MODULE_Visualization=OFF'
      '-DBUILD_MODULE_ApplicationFramework=OFF'
      '-DBUILD_MODULE_DataExchange=OFF'
      '-DBUILD_MODULE_Draw=OFF'
    )
  }
}
