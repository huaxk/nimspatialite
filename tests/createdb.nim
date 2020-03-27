import db_sqlite, strformat
import wrapper

var
  db: DbConn
  str: string

echo "spatialiter version: " & $spatialiteVersion()
echo "spatialite target cpu: " & $spatialiteTargetCpu()

db = open("test.db", "", "", "")

let cache = spatialiteAllocConnection()
spatialiteInitEx(db, cache, 0)

str = "SELECT InitSpatialMetadata()"
db.exec(str.sql)

spatialiteCleanupEx(cache)
db.close