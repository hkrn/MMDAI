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
#include <vpvl2/extensions/vpdb/Motion.h>
#include <vpvl2/extensions/vpdb/Project.h>

#include "sqlite3.h"

namespace {

#include "queries/create_project_tables.h"
#include "queries/drop_project_tables.h"

}

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace extensions
{
namespace vpdb
{

struct Project::PrivateContext {
    static const int kLatestSchemaVersion = 1;
    struct Migration {
        void (*upgrade)(PrivateContext *ctx);
        void (*downgrade)(PrivateContext *ctx);
    };

    PrivateContext()
        : db(0)
    {
    }
    ~PrivateContext() {
        sqlite3_close(db);
        db = 0;
    }

    static int handleVersion(void *self, int /* argc */, char **argv, char ** /* columns */) {
        PrivateContext *ctx = static_cast<PrivateContext *>(self);
        ctx->version = int(strtol(argv[0], 0, 10));
        return 0;
    }
    static void createProjectTables(PrivateContext *ctx) {
        ctx->executeQuery(reinterpret_cast<const char *>(g_create_project_tables_sql), 0);
    }
    static void dropProjectTables(PrivateContext *ctx) {
        ctx->executeQuery(reinterpret_cast<const char *>(g_drop_project_tables_sql), 0);
    }

    bool executeQuery(const char *query, sqlite3_callback callback) {
        char *errmsg = 0;
        int rc = sqlite3_exec(db, query, callback, this, &errmsg);
        if (rc != SQLITE_OK) {
            VPVL2_LOG(WARNING, "Cannot execute query: " << errmsg);
            sqlite3_free(errmsg);
            return false;
        }
        return true;
    }
    bool beginTransaction() {
        return executeQuery("begin transaction;", 0);
    }
    bool commitTransaction() {
        return executeQuery("commit transaction;", 0);
    }
    bool open(const char *filename) {
        int rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_PRIVATECACHE, 0);
        if (rc != SQLITE_OK) {
            VPVL2_LOG(WARNING, "Cannot open database: " << sqlite3_errmsg(db));
            return false;
        }
        if (!executeQuery("pragma user_version;", &PrivateContext::handleVersion)) {
            return false;
        }
        upgrade(kLatestSchemaVersion);
        return true;
    }
    void upgrade(int versionTo) {
        static Migration migrations[] = {
            { &PrivateContext::createProjectTables, &PrivateContext::dropProjectTables }
        };
        const int destination = std::min(versionTo, int(sizeof(migrations) / sizeof(migrations[0])));
        beginTransaction();
        for (int i = version; i < destination; i++) {
            const Migration &m = migrations[i];
            m.upgrade(this);
        }
        commitTransaction();
    }
    void downgrade(int versionTo) {
        static Migration migrations[] = {
            { &PrivateContext::createProjectTables, &PrivateContext::dropProjectTables }
        };
        const int destination = std::max(versionTo, 0);
        beginTransaction();
        for (int i = version - 1; i >= destination; i--) {
            const Migration &m = migrations[i];
            m.downgrade(this);
        }
        commitTransaction();
    }

    sqlite3 *db;
    int version;
};

Project::Project(bool ownMemory)
    : Scene(ownMemory),
      m_context(new PrivateContext())
{
}

Project::~Project()
{
    delete m_context;
    m_context = 0;
}

bool Project::load(const char *filename)
{
    return m_context->open(filename);
}

bool Project::save(const char * /* filename */)
{
    return true;
}

void Project::clear()
{
}

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
