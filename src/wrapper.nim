import os, strformat, strutils
import nimterop/[cimport, build]

const
  baseDir = getProjectCacheDir("libspatialite")
  dlUrl = &"http://www.gaia-gis.it/gaia-sins/libspatialite-sources/libspatialite-$1.tar.gz"
  fixDefine = """
typedef sqlite3 * PSqlite3;
"""

getHeader("spatialite.h", dlurl = dlUrl, outdir = baseDir)

when spatialiteDL:
  when dirExists(baseDir / "buildcache"):
    cIncludeDir(baseDir / "buildcache")

static:
  # cDebug()
  # cDisableCaching()
  cAddStdDir()
  cSkipSymbol(@["PSqlite3" ,"gaiaGetVectorLayersList"])

  mkDir(baseDir)
  let originContent = readFile(cSearchPath("spatialite.h"))
  let newContent = originContent.replace("sqlite3 *", "PSqlite3")
  writeFile(baseDir/"spatialite_fix.h", fixDefine & newContent)

from std/sqlite3 import PSqlite3

type
  sqlite3_int64 = clonglong
  gaiaGeomColl{.pure, final.} = object
  gaiaGeomCollPtr* = ptr gaiaGeomColl
  gaiaVectorLayersList{.pure, final.} = object
  gaiaVectorLayersListPtr* = ptr gaiaVectorLayersList

when not spatialiteStatic:
  cImport(baseDir/"spatialite_fix.h", recurse = true, dynlib = "spatialiteLPath")
  {.pragma: impspatialite, impspatialite_fixC.} 
else:
  {.passC: "-include sqlite3.h".}
  cImport(baseDir/"spatialite_fix.h", recurse = true)
  {.pragma: impspatialite, impspatialite_fix.} 
  
proc gaiaGetVectorLayersList*(handle: PSqlite3,
                              table: cstring,
                              geometry: cstring,
                              mode: cint):
    gaiaVectorLayersListPtr {.impspatialite.}
