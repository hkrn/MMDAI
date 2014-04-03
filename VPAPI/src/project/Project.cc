/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include <vpvl2/vpvl2.h>
#include <project/Motion.h>
#include <project/Project.h>

#include <QtSql>

namespace {

#include "queries/create_project_tables.h"
#include "queries/drop_project_tables.h"

}

namespace project
{

struct Project::PrivateContext {
    static const int kLatestSchemaVersion = 1;
    struct Migration {
        void (*upgrade)(PrivateContext *ctx);
        void (*downgrade)(PrivateContext *ctx);
    };

    PrivateContext(QSqlDatabase *db)
        : databaseHandle(db)
    {
        Q_ASSERT(databaseHandle);
    }
    ~PrivateContext() {
        databaseHandle->close();
    }

    static void createProjectTables(PrivateContext *ctx) {
        QSqlQuery query(*ctx->databaseHandle);
        query.exec(reinterpret_cast<const char *>(g_create_project_tables_sql));
    }
    static void dropProjectTables(PrivateContext *ctx) {
        QSqlQuery query(*ctx->databaseHandle);
        query.exec(reinterpret_cast<const char *>(g_drop_project_tables_sql));
    }

    void upgrade(int versionTo) {
        static Migration migrations[] = {
            { &PrivateContext::createProjectTables, &PrivateContext::dropProjectTables }
        };
        const int destination = std::min(versionTo, int(sizeof(migrations) / sizeof(migrations[0])));
        databaseHandle->transaction();
        for (int i = version; i < destination; i++) {
            const Migration &m = migrations[i];
            m.upgrade(this);
        }
        databaseHandle->commit();
    }
    void downgrade(int versionTo) {
        static Migration migrations[] = {
            { &PrivateContext::createProjectTables, &PrivateContext::dropProjectTables }
        };
        const int destination = std::max(versionTo, 0);
        databaseHandle->transaction();
        for (int i = version - 1; i >= destination; i--) {
            const Migration &m = migrations[i];
            m.downgrade(this);
        }
        databaseHandle->commit();
    }

    QSqlDatabase *databaseHandle;
    int version;
};

Project::Project(QSqlDatabase *database, bool ownMemory)
    : Scene(ownMemory),
      m_context(new PrivateContext(database))
{
}

Project::~Project()
{
    delete m_context;
    m_context = 0;
}

bool Project::load(const char *filename)
{
    m_context->databaseHandle->setDatabaseName(filename);
    if (m_context->databaseHandle->open()) {
        m_context->upgrade(PrivateContext::kLatestSchemaVersion);
        QSqlQuery query(*m_context->databaseHandle);
        query.exec("pragma user_version;");
        m_context->version = query.value(0).toInt();
        return true;
    }
    return false;
}

bool Project::save(const char * /* filename */)
{
    return true;
}

void Project::clear()
{
}

QSqlDatabase *Project::databaseHandle() const
{
    return m_context->databaseHandle;
}

} /* namespace project */
