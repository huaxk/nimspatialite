import nimterop/build

setDefines(["spatialiteStd"])
when not defined(spatialiteSetVer):
  setDefines(["spatialiteSetVer=4.3.0a"])
  
import ./wrapper
export wrapper