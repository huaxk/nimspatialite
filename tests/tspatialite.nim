import unittest, db_sqlite, strformat, os
import nimspatialite

# let dataFile = currentSourcePath.parentDir()/"test.db"
# if fileExists(dataFile):
#   removeFile(dataFile)

# abortOnError = true

suite "spatialite":
  let db = open(":memory:", "", "", "")
  let cache = spatialiteAllocConnection()

  spatialiteInitEx(db, cache, 0)

  let s = "select count(*) from sqlite_master "&
          "where type='table' and name='spatial_ref_sys'"
  if db.getValue(s.sql) == $0:
    db.exec(sql"SELECT InitSpatialMetadata(1)")

  test "createTable":
    let s = "CREATE TABLE test_pt ("&
            "id INTEGER NOT NULL PRIMARY KEY,"&
            "name TEXT NOT NULL)"
    db.exec(s.sql)

  test "addColumn":
    db.exec(sql"SELECT AddGeometryColumn('test_pt', 'geom', 4326, 'POINT', 'XY')")

  test "createIndex":
    db.exec(sql"select CreateSpatialIndex('test_pt', 'geom')")

  test "insert":
    let num = 10
    db.exec(sql"begin")
    for i in 1 .. num:
      let s = &"INSERT INTO test_pt (id, name, geom) VALUES ({i}, 'test point {i}', MakePoint({i}, {i}, 4326))"
      db.exec(s.sql)
    db.exec(sql"commit")
    check db.getValue(sql"select count(*) from test_pt") == $num

  test "query":
    let s = sql"select asText(geom) from test_pt where id = 1"
    check db.getValue(s) == "POINT(1 1)"

  test "dropTable":
    check gaiaDropTable(db, "test_pt") != 0

  spatialiteCleanupEx(cache)
  db.close()