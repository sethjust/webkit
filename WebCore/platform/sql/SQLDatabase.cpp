/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "SQLDatabase.h"

#include "Logging.h"
#include "SQLAuthorizer.h"
#include "SQLStatement.h"

#include <sqlite3.h>

namespace WebCore {

const int SQLResultDone = SQLITE_DONE;
const int SQLResultError = SQLITE_ERROR;
const int SQLResultOk = SQLITE_OK;
const int SQLResultRow = SQLITE_ROW;
const int SQLResultSchema = SQLITE_SCHEMA;


SQLDatabase::SQLDatabase()
    : m_db(0)
    , m_transactionInProgress(false)
{
#ifndef NDEBUG
    memset(&m_openingThread, 0, sizeof(pthread_t));
#endif
}

SQLDatabase::~SQLDatabase()
{
    close();
}

bool SQLDatabase::open(const String& filename)
{
    close();
    
    //SQLite expects a null terminator on its UTF16 strings
    m_path = filename.copy();
    
    m_lastError = sqlite3_open16(m_path.charactersWithNullTermination(), &m_db);
    if (m_lastError != SQLITE_OK) {
        LOG_ERROR("SQLite database failed to load from %s\nCause - %s", filename.ascii().data(),
            sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        m_db = 0;
        return false;
    }

#ifndef NDEBUG
    if (isOpen())
        m_openingThread = pthread_self();
#endif
    
    if (!SQLStatement(*this, "PRAGMA temp_store = MEMORY;").executeCommand())
        LOG_ERROR("SQLite database could not set temp_store to memory");

    return isOpen();
}

void SQLDatabase::close()
{
    if (m_db) {
        sqlite3_close(m_db);
        m_path.truncate(0);
        m_db = 0;
    }
#ifndef NDEBUG
    memset(&m_openingThread, 0, sizeof(pthread_t));
#endif
}

void SQLDatabase::setFullsync(bool fsync) 
{
    if (fsync) 
        executeCommand("PRAGMA fullfsync = 1;");
    else
        executeCommand("PRAGMA fullfsync = 0;");
}

void SQLDatabase::setSynchronous(SynchronousPragma sync)
{
    executeCommand(String::format("PRAGMA synchronous = %i", sync));
}

void SQLDatabase::setBusyTimeout(int ms)
{
    if (m_db)
        sqlite3_busy_timeout(m_db, ms);
    else
        LOG(SQLDatabase, "BusyTimeout set on non-open database");
}

void SQLDatabase::setBusyHandler(int(*handler)(void*, int))
{
    if (m_db)
        sqlite3_busy_handler(m_db, handler, NULL);
    else
        LOG(SQLDatabase, "Busy handler set on non-open database");
}

bool SQLDatabase::executeCommand(const String& sql)
{
    return SQLStatement(*this, sql).executeCommand();
}

bool SQLDatabase::returnsAtLeastOneResult(const String& sql)
{
    return SQLStatement(*this, sql).returnsAtLeastOneResult();
}

bool SQLDatabase::tableExists(const String& tablename)
{
    if (!isOpen())
        return false;
        
    String statement = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = '" + tablename + "';";
    
    SQLStatement sql(*this, statement);
    sql.prepare();
    return sql.step() == SQLITE_ROW;
}

void SQLDatabase::clearAllTables()
{
    String query = "SELECT name FROM sqlite_master WHERE type='table';";
    Vector<String> tables;
    if (!SQLStatement(*this, query).returnTextResults16(0, tables)) {
        LOG(SQLDatabase, "Unable to retrieve list of tables from database");
        return;
    }
    
    for (Vector<String>::iterator table = tables.begin(); table != tables.end(); ++table ) {
        if (*table == "sqlite_sequence")
            continue;
        if (!executeCommand("DROP TABLE " + *table))
            LOG(SQLDatabase, "Unable to drop table %s", (*table).ascii().data());
    }
}

void SQLDatabase::runVacuumCommand()
{
    if (!executeCommand("VACUUM;"))
        LOG(SQLDatabase, "Unable to vacuum database - %s", lastErrorMsg());
}

int64_t SQLDatabase::lastInsertRowID()
{
    if (!m_db)
        return 0;
    return sqlite3_last_insert_rowid(m_db);
}

int SQLDatabase::lastChanges()
{
    if (!m_db)
        return 0;
    return sqlite3_changes(m_db);
}

int SQLDatabase::lastError()
{
    return m_db ? sqlite3_errcode(m_db) : SQLITE_ERROR;
}

const char* SQLDatabase::lastErrorMsg()
{ 
    return sqlite3_errmsg(m_db);
}

int SQLDatabase::authorizerFunction(void* userData, int actionCode, const char* parameter1, const char* parameter2, const char* /*databaseName*/, const char* /*trigger_or_view*/)
{
    SQLAuthorizer* auth = static_cast<SQLAuthorizer*>(userData);
    ASSERT(auth);

    switch (actionCode) {
        case SQLITE_CREATE_INDEX:
            return auth->createIndex(parameter1, parameter2);
        case SQLITE_CREATE_TABLE:
            return auth->createTable(parameter1);
        case SQLITE_CREATE_TEMP_INDEX:
            return auth->createTempIndex(parameter1, parameter2);
        case SQLITE_CREATE_TEMP_TABLE:
            return auth->createTempTable(parameter1);
        case SQLITE_CREATE_TEMP_TRIGGER:
            return auth->createTempTrigger(parameter1, parameter2);
        case SQLITE_CREATE_TEMP_VIEW:
            return auth->createTempView(parameter1);
        case SQLITE_CREATE_TRIGGER:
            return auth->createTrigger(parameter1, parameter2);
        case SQLITE_CREATE_VIEW:
            return auth->createView(parameter1);
        case SQLITE_DELETE:
            return auth->allowDelete(parameter1);
        case SQLITE_DROP_INDEX:
            return auth->dropIndex(parameter1, parameter2);
        case SQLITE_DROP_TABLE:
            return auth->dropTable(parameter1);
        case SQLITE_DROP_TEMP_INDEX:
            return auth->dropTempIndex(parameter1, parameter2);
        case SQLITE_DROP_TEMP_TABLE:
            return auth->dropTempTable(parameter1);
        case SQLITE_DROP_TEMP_TRIGGER:
            return auth->dropTempTrigger(parameter1, parameter2);
        case SQLITE_DROP_TEMP_VIEW:
            return auth->dropTempView(parameter1);
        case SQLITE_DROP_TRIGGER:
            return auth->dropTrigger(parameter1, parameter2);
        case SQLITE_DROP_VIEW:
            return auth->dropView(parameter1);
        case SQLITE_INSERT:
            return auth->allowInsert(parameter1);
        case SQLITE_PRAGMA:
            return auth->allowPragma(parameter1, parameter2);
        case SQLITE_READ:
            return auth->allowRead(parameter1, parameter2);
        case SQLITE_SELECT:
            return auth->allowSelect();
        case SQLITE_TRANSACTION:
            return auth->allowTransaction();
        case SQLITE_UPDATE:
            return auth->allowUpdate(parameter1, parameter2);
        case SQLITE_ATTACH:
            return auth->allowAttach(parameter1);
        case SQLITE_DETACH:
            return auth->allowDetach(parameter1);
        case SQLITE_ALTER_TABLE:
            return auth->allowAlterTable(parameter1, parameter2);
        case SQLITE_REINDEX:
            return auth->allowReindex(parameter1);
#if SQLITE_VERSION_NUMBER >= 3003013 
        case SQLITE_ANALYZE:
            return auth->allowAnalyze(parameter1);
        case SQLITE_CREATE_VTABLE:
            return auth->createVTable(parameter1, parameter2);
        case SQLITE_DROP_VTABLE:
            return auth->dropVTable(parameter1, parameter2);
        case SQLITE_FUNCTION:
            return auth->allowFunction(parameter1);
#endif
        default:
            ASSERT_NOT_REACHED();
            return SQLAuthDeny;
    }
}

void SQLDatabase::setAuthorizer(PassRefPtr<SQLAuthorizer> auth)
{
    if (!m_db) {
        LOG_ERROR("Attempt to set an authorizer on a non-open SQL database");
        ASSERT_NOT_REACHED();
        return;
    }

    MutexLocker locker(m_authorizerLock);

    m_authorizer = auth;
    if (m_authorizer)
        sqlite3_set_authorizer(m_db, SQLDatabase::authorizerFunction, m_authorizer.get());
    else
        sqlite3_set_authorizer(m_db, NULL, 0);
}

void SQLDatabase::lock()
{
    m_lockingMutex.lock();
}

void SQLDatabase::unlock()
{
    m_lockingMutex.unlock();
}

} // namespace WebCore


