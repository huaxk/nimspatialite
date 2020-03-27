
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#ifdef SPATIALITE_AMALGAMATION
#include <spatialite/sqlite3.h>
#else
#include <sqlite3.h>
#endif

#ifndef SPATIALITE_EXTENSION
#include <spatialite.h>
#endif

int main(void)
{
  sqlite3 *db_handle;
  sqlite3_stmt *stmt;
  int ret;
  char *err_msg = NULL;
  char sql[2048];
  char sql2[1024];
  int i;
  char **results;
  int rows;
  int columns;
  int cnt;
  const char *type;
  int srid;
  char name[1024];
  char geom[2048];

#ifndef SPATIALITE_EXTENSION
  /*
     * initializing SpatiaLite-Amalgamation
     *
     * any C/C++ source requires this to be performed
     * for each connection before invoking the first
     * SQLite/SpatiaLite call
     */
  spatialite_init(0);
  fprintf(stderr, "\n\n******* hard-linked libspatialite ********\n\n");
#endif

  /* creating/connecting the test_db */
  ret =
      sqlite3_open_v2("test-db.sqlite", &db_handle,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "cannot open 'test-db.sqlite': %s\n",
            sqlite3_errmsg(db_handle));
    sqlite3_close(db_handle);
    db_handle = NULL;
    return -1;
  }

#ifdef SPATIALITE_EXTENSION
  /*
     * loading SpatiaLite as an extension
     */
  sqlite3_enable_load_extension(db_handle, 1);
  strcpy(sql, "SELECT load_extension('libspatialite.so')");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "load_extension() error: %s\n", err_msg);
    sqlite3_free(err_msg);
    return 0;
  }
  fprintf(stderr, "\n\n**** SpatiaLite loaded as an extension ***\n\n");
#endif

/* reporting version infos */
#ifndef SPATIALITE_EXTENSION
  /*
* please note well:
* this process is physically linked to libspatialite
* so we can directly call any SpatiaLite's API function
*/
  fprintf(stderr, "SQLite version: %s\n", sqlite3_libversion());
  fprintf(stderr, "SpatiaLite version: %s\n", spatialite_version());
#else
  /*
* please note well:
* this process isn't physically linked to libspatialite
* because we loaded the library as an extension
*
* so we aren't enabled to directly call any SpatiaLite's API functions
* we simply can access SpatiaLite indirectly via SQL statements
*/
  strcpy(sql, "SELECT sqlite_version()");
  ret =
      sqlite3_get_table(db_handle, sql, &results, &rows, &columns, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  if (rows < 1)
  {
    fprintf(stderr,
            "Unexpected error: sqlite_version() not found ??????\n");
    goto stop;
  }
  else
  {
    for (i = 1; i <= rows; i++)
    {
      fprintf(stderr, "SQLite version: %s\n",
              results[(i * columns) + 0]);
    }
  }
  sqlite3_free_table(results);
  strcpy(sql, "SELECT spatialite_version()");
  ret =
      sqlite3_get_table(db_handle, sql, &results, &rows, &columns, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  if (rows < 1)
  {
    fprintf(stderr,
            "Unexpected error: spatialite_version() not found ??????\n");
    goto stop;
  }
  else
  {
    for (i = 1; i <= rows; i++)
    {
      fprintf(stderr, "SpatiaLite version: %s\n",
              results[(i * columns) + 0]);
    }
  }
  sqlite3_free_table(results);
#endif /* SpatiaLite as an extension */

  /* initializing SpatiaLite's metadata tables */
  strcpy(sql, "SELECT InitSpatialMetadata()");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "InitSpatialMetadata() error: %s\n", err_msg);
    sqlite3_free(err_msg);
    return 0;
  }

  /* creating a POINT table */
  strcpy(sql, "CREATE TABLE test_pt (");
  strcat(sql, "id INTEGER NOT NULL PRIMARY KEY,");
  strcat(sql, "name TEXT NOT NULL)");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  /* creating a POINT Geometry column */
  strcpy(sql, "SELECT AddGeometryColumn('test_pt', ");
  strcat(sql, "'geom', 4326, 'POINT', 'XY')");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /* creating a LINESTRING table */
  strcpy(sql, "CREATE TABLE test_ln (");
  strcat(sql, "id INTEGER NOT NULL PRIMARY KEY,");
  strcat(sql, "name TEXT NOT NULL)");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  /* creating a LINESTRING Geometry column */
  strcpy(sql, "SELECT AddGeometryColumn('test_ln', ");
  strcat(sql, "'geom', 4326, 'LINESTRING', 'XY')");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /* creating a POLYGON table */
  strcpy(sql, "CREATE TABLE test_pg (");
  strcat(sql, "id INTEGER NOT NULL PRIMARY KEY,");
  strcat(sql, "name TEXT NOT NULL)");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  /* creating a POLYGON Geometry column */
  strcpy(sql, "SELECT AddGeometryColumn('test_pg', ");
  strcat(sql, "'geom', 4326, 'POLYGON', 'XY')");
  ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /*
* inserting some POINTs
* please note well: SQLite is ACID and Transactional
* so (to get best performance) the whole insert cycle
* will be handled as a single TRANSACTION
*/
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  for (i = 0; i < 100000; i++)
  {
    /* for POINTs we'll use full text sql statements */
    strcpy(sql, "INSERT INTO test_pt (id, name, geom) VALUES (");
    sprintf(sql2, "%d, 'test POINT #%d'", i + 1, i + 1);
    strcat(sql, sql2);
    sprintf(sql2, ", GeomFromText('POINT(%1.6f %1.6f)'", i / 1000.0,
            i / 1000.0);
    strcat(sql, sql2);
    strcat(sql, ", 4326))");
    ret = sqlite3_exec(db_handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
    {
      fprintf(stderr, "Error: %s\n", err_msg);
      sqlite3_free(err_msg);
      goto stop;
    }
  }
  ret = sqlite3_exec(db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /* checking POINTs */
  strcpy(sql, "SELECT DISTINCT Count(*), ST_GeometryType(geom), ");
  strcat(sql, "ST_Srid(geom) FROM test_pt");
  ret =
      sqlite3_get_table(db_handle, sql, &results, &rows, &columns, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  if (rows < 1)
  {
    fprintf(stderr, "Unexpected error: ZERO POINTs found ??????\n");
    goto stop;
  }
  else
  {
    for (i = 1; i <= rows; i++)
    {
      cnt = atoi(results[(i * columns) + 0]);
      type = results[(i * columns) + 1];
      srid = atoi(results[(i * columns) + 2]);
      fprintf(stderr, "Inserted %d entities of type %s SRID=%d\n",
              cnt, type, srid);
    }
  }
  sqlite3_free_table(results);

  /*
* inserting some LINESTRINGs
* this time we'll use a Prepared Statement
*/
  strcpy(sql, "INSERT INTO test_ln (id, name, geom) ");
  strcat(sql, "VALUES (?, ?, GeomFromText(?, 4326))");
  ret = sqlite3_prepare_v2(db_handle, sql, strlen(sql), &stmt, NULL);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n%s\n", sql,
            sqlite3_errmsg(db_handle));
    goto stop;
  }
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  for (i = 0; i < 100000; i++)
  {
    /* setting up values / binding */
    sprintf(name, "test LINESTRING #%d", i + 1);
    strcpy(geom, "LINESTRING(");
    if ((i % 2) == 1)
    {
      /* odd row: five points */
      strcat(geom, "-180.0 -90.0, ");
      sprintf(sql2, "%1.6f %1.6f, ", -10.0 - (i / 1000.0),
              -10.0 - (i / 1000.0));
      strcat(geom, sql2);
      sprintf(sql2, "%1.6f %1.6f, ", -10.0 - (i / 1000.0),
              10.0 + (i / 1000.0));
      strcat(geom, sql2);
      sprintf(sql2, "%1.6f %1.6f, ", 10.0 + (i / 1000.0),
              10.0 + (i / 1000.0));
      strcat(geom, sql2);
      strcat(geom, "180.0 90.0");
    }
    else
    {
      /* even row: two points */
      sprintf(sql2, "%1.6f %1.6f, ", -10.0 - (i / 1000.0),
              -10.0 - (i / 1000.0));
      strcat(geom, sql2);
      sprintf(sql2, "%1.6f %1.6f, ", 10.0 + (i / 1000.0),
              10.0 + (i / 1000.0));
      strcat(geom, sql2);
    }
    strcat(geom, ")");
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_bind_int(stmt, 1, i + 1);
    sqlite3_bind_text(stmt, 2, name, strlen(name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, geom, strlen(geom), SQLITE_STATIC);
    /* performing INSERT INTO */
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
      continue;
    fprintf(stderr, "sqlite3_step() error: [%s]\n",
            sqlite3_errmsg(db_handle));
    goto stop;
  }
  sqlite3_finalize(stmt);
  ret = sqlite3_exec(db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /* checking LINESTRINGs */
  strcpy(sql, "SELECT DISTINCT Count(*), ST_GeometryType(geom), ");
  strcat(sql, "ST_Srid(geom) FROM test_ln");
  ret =
      sqlite3_get_table(db_handle, sql, &results, &rows, &columns, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  if (rows < 1)
  {
    fprintf(stderr, "Unexpected error: ZERO LINESTRINGs found ??????\n");
    goto stop;
  }
  else
  {
    for (i = 1; i <= rows; i++)
    {
      cnt = atoi(results[(i * columns) + 0]);
      type = results[(i * columns) + 1];
      srid = atoi(results[(i * columns) + 2]);
      fprintf(stderr, "Inserted %d entities of type %s SRID=%d\n",
              cnt, type, srid);
    }
  }
  sqlite3_free_table(results);

  /*
* inserting some POLYGONs
* this time too we'll use a Prepared Statement
*/
  strcpy(sql, "INSERT INTO test_pg (id, name, geom) ");
  strcat(sql, "VALUES (?, ?, GeomFromText(?, 4326))");
  ret = sqlite3_prepare_v2(db_handle, sql, strlen(sql), &stmt, NULL);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n%s\n", sql,
            sqlite3_errmsg(db_handle));
    goto stop;
  }
  ret = sqlite3_exec(db_handle, "BEGIN", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  for (i = 0; i < 100000; i++)
  {
    /* setting up values / binding */
    sprintf(name, "test POLYGON #%d", i + 1);
    strcpy(geom, "POLYGON((");
    sprintf(sql2, "%1.6f %1.6f, ", -10.0 - (i / 1000.0),
            -10.0 - (i / 1000.0));
    strcat(geom, sql2);
    sprintf(sql2, "%1.6f %1.6f, ", 10.0 - (i / 1000.0),
            -10.0 - (i / 1000.0));
    strcat(geom, sql2);
    sprintf(sql2, "%1.6f %1.6f, ", 10.0 + (i / 1000.0),
            10.0 + (i / 1000.0));
    strcat(geom, sql2);
    sprintf(sql2, "%1.6f %1.6f, ", -10.0 - (i / 1000.0),
            10.0 - (i / 1000.0));
    strcat(geom, sql2);
    sprintf(sql2, "%1.6f %1.6f", -10.0 - (i / 1000.0),
            -10.0 - (i / 1000.0));
    strcat(geom, sql2);
    strcat(geom, "))");
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_bind_int(stmt, 1, i + 1);
    sqlite3_bind_text(stmt, 2, name, strlen(name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, geom, strlen(geom), SQLITE_STATIC);
    /* performing INSERT INTO */
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
      continue;
    fprintf(stderr, "sqlite3_step() error: [%s]\n",
            sqlite3_errmsg(db_handle));
    goto stop;
  }
  sqlite3_finalize(stmt);
  ret = sqlite3_exec(db_handle, "COMMIT", NULL, NULL, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }

  /* checking POLYGONs */
  strcpy(sql, "SELECT DISTINCT Count(*), ST_GeometryType(geom), ");
  strcat(sql, "ST_Srid(geom) FROM test_pg");
  ret =
      sqlite3_get_table(db_handle, sql, &results, &rows, &columns, &err_msg);
  if (ret != SQLITE_OK)
  {
    fprintf(stderr, "Error: %s\n", err_msg);
    sqlite3_free(err_msg);
    goto stop;
  }
  if (rows < 1)
  {
    fprintf(stderr, "Unexpected error: ZERO POLYGONs found ??????\n");
    goto stop;
  }
  else
  {
    for (i = 1; i <= rows; i++)
    {
      cnt = atoi(results[(i * columns) + 0]);
      type = results[(i * columns) + 1];
      srid = atoi(results[(i * columns) + 2]);
      fprintf(stderr, "Inserted %d entities of type %s SRID=%d\n",
              cnt, type, srid);
    }
  }
  sqlite3_free_table(results);

  /* closing the DB connection */
stop:
  sqlite3_close(db_handle);
  return 0;
}