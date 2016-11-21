/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-07-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#include <maxscale/query_classifier.h>
#include <maxscale/log_manager.h>
#include <maxscale/modules.h>
#include <maxscale/modutil.h>
#include <maxscale/alloc.h>
#include <maxscale/utils.h>

//#define QC_TRACE_ENABLED
#undef QC_TRACE_ENABLED

#if defined(QC_TRACE_ENABLED)
#define QC_TRACE() MXS_NOTICE(__func__)
#else
#define QC_TRACE()
#endif

struct type_name_info
{
    const char* name;
    size_t name_len;
};

static const char default_qc_name[] = "qc_sqlite";

static QUERY_CLASSIFIER* classifier;


bool qc_init(const char* plugin_name, const char* plugin_args)
{
    QC_TRACE();
    ss_dassert(!classifier);

    if (!plugin_name || (*plugin_name == 0))
    {
        MXS_NOTICE("No query classifier specified, using default '%s'.", default_qc_name);
        plugin_name = default_qc_name;
    }

    bool success = false;
    classifier = qc_load(plugin_name);

    if (classifier)
    {
        success = classifier->qc_init(plugin_args);
    }

    return success;
}

void qc_end(void)
{
    QC_TRACE();
    ss_dassert(classifier);

    classifier->qc_end();
    classifier = NULL;
}

QUERY_CLASSIFIER* qc_load(const char* plugin_name)
{
    void* module = load_module(plugin_name, MODULE_QUERY_CLASSIFIER);

    if (module)
    {
        MXS_INFO("%s loaded.", plugin_name);
    }
    else
    {
        MXS_ERROR("Could not load %s.", plugin_name);
    }

    return (QUERY_CLASSIFIER*) module;
}

void qc_unload(QUERY_CLASSIFIER* classifier)
{
    // TODO: The module loading/unloading needs an overhaul before we
    // TODO: actually can unload something.
}

bool qc_thread_init(void)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_thread_init();
}

void qc_thread_end(void)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_thread_end();
}

qc_parse_result_t qc_parse(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_parse(query);
}

uint32_t qc_get_type(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_type(query);
}

qc_query_op_t qc_get_operation(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_operation(query);
}

char* qc_get_created_table_name(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_created_table_name(query);
}

bool qc_is_drop_table_query(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_is_drop_table_query(query);
}

bool qc_is_real_query(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_is_real_query(query);
}

char** qc_get_table_names(GWBUF* query, int* tblsize, bool fullnames)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_table_names(query, tblsize, fullnames);
}

char* qc_get_canonical(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    char *rval;

    if (classifier->qc_get_canonical)
    {
        rval = classifier->qc_get_canonical(query);
    }
    else
    {
        rval = modutil_get_canonical(query);
    }

    if (rval)
    {
        squeeze_whitespace(rval);
    }

    return rval;
}

bool qc_query_has_clause(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_query_has_clause(query);
}

void qc_get_field_info(GWBUF* query, const QC_FIELD_INFO** infos, size_t* n_infos)
{
    QC_TRACE();
    ss_dassert(classifier);

    classifier->qc_get_field_info(query, infos, n_infos);
}

char** qc_get_database_names(GWBUF* query, int* sizep)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_database_names(query, sizep);
}

char* qc_get_prepare_name(GWBUF* query)
{
    QC_TRACE();
    ss_dassert(classifier);

    return classifier->qc_get_prepare_name(query);
}

struct type_name_info field_usage_to_type_name_info(qc_field_usage_t usage)
{
    struct type_name_info info;

    switch (usage)
    {
    case QC_USED_IN_SELECT:
        {
            static const char name[] = "QC_USED_IN_SELECT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QC_USED_IN_SUBSELECT:
        {
            static const char name[] = "QC_USED_IN_SUBSELECT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QC_USED_IN_WHERE:
        {
            static const char name[] = "QC_USED_IN_WHERE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QC_USED_IN_SET:
        {
            static const char name[] = "QC_USED_IN_SET";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QC_USED_IN_GROUP_BY:
        {
            static const char name[] = "QC_USED_IN_GROUP_BY";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    default:
        {
            static const char name[] = "UNKNOWN_FIELD_USAGE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;
    }

    return info;
}



const char* qc_field_usage_to_string(qc_field_usage_t usage)
{
    return field_usage_to_type_name_info(usage).name;
}

static const qc_field_usage_t FIELD_USAGE_VALUES[] =
{
    QC_USED_IN_SELECT,
    QC_USED_IN_SUBSELECT,
    QC_USED_IN_WHERE,
    QC_USED_IN_SET,
    QC_USED_IN_GROUP_BY,
};

static const int N_FIELD_USAGE_VALUES =
    sizeof(FIELD_USAGE_VALUES) / sizeof(FIELD_USAGE_VALUES[0]);
static const int FIELD_USAGE_MAX_LEN = 20; // strlen("QC_USED_IN_SUBSELECT");

char* qc_field_usage_mask_to_string(uint32_t mask)
{
    size_t len = 0;

    // First calculate how much space will be needed.
    for (int i = 0; i < N_FIELD_USAGE_VALUES; ++i)
    {
        if (mask & FIELD_USAGE_VALUES[i])
        {
            if (len != 0)
            {
                ++len; // strlen("|");
            }

            len += FIELD_USAGE_MAX_LEN;
        }
    }

    ++len;

    // Then make one allocation and build the string.
    char* s = (char*) MXS_MALLOC(len);

    if (s)
    {
        if (len > 1)
        {
            char* p = s;

            for (int i = 0; i < N_FIELD_USAGE_VALUES; ++i)
            {
                qc_field_usage_t value = FIELD_USAGE_VALUES[i];

                if (mask & value)
                {
                    if (p != s)
                    {
                        strcpy(p, "|");
                        ++p;
                    }

                    struct type_name_info info = field_usage_to_type_name_info(value);

                    strcpy(p, info.name);
                    p += info.name_len;
                }
            }
        }
        else
        {
            *s = 0;
        }
    }

    return s;
}

const char* qc_op_to_string(qc_query_op_t op)
{
    switch (op)
    {
    case QUERY_OP_UNDEFINED:
	return "QUERY_OP_UNDEFINED";

    case QUERY_OP_SELECT:
	return "QUERY_OP_SELECT";

    case QUERY_OP_UPDATE:
	return "QUERY_OP_UPDATE";

    case QUERY_OP_INSERT:
	return "QUERY_OP_INSERT";

    case QUERY_OP_DELETE:
	return "QUERY_OP_DELETE";

    case QUERY_OP_TRUNCATE:
	return "QUERY_OP_TRUNCATE";

    case QUERY_OP_ALTER:
	return "QUERY_OP_ALTER";

    case QUERY_OP_CREATE:
	return "QUERY_OP_CREATE";

    case QUERY_OP_DROP:
	return "QUERY_OP_DROP";

    case QUERY_OP_CHANGE_DB:
	return "QUERY_OP_CHANGE_DB";

    case QUERY_OP_LOAD:
	return "QUERY_OP_LOAD";

    case QUERY_OP_GRANT:
	return "QUERY_OP_GRANT";

    case QUERY_OP_REVOKE:
	return "QUERY_OP_REVOKE";

    default:
        return "UNKNOWN_QUERY_OP";
    }
}

struct type_name_info type_to_type_name_info(qc_query_type_t type)
{
    struct type_name_info info;

    switch (type)
    {
    case QUERY_TYPE_UNKNOWN:
        {
            static const char name[] = "QUERY_TYPE_UNKNOWN";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_LOCAL_READ:
        {
            static const char name[] = "QUERY_TYPE_LOCAL_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_READ:
        {
            static const char name[] = "QUERY_TYPE_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_WRITE:
        {
            static const char name[] = "QUERY_TYPE_WRITE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_MASTER_READ:
        {
            static const char name[] = "QUERY_TYPE_MASTER_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_SESSION_WRITE:
        {
            static const char name[] = "QUERY_TYPE_SESSION_WRITE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_USERVAR_WRITE:
        {
            static const char name[] = "QUERY_TYPE_USERVAR_WRITE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
        }
        break;

    case QUERY_TYPE_USERVAR_READ:
        {
            static const char name[] = "QUERY_TYPE_USERVAR_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_SYSVAR_READ:
        {
            static const char name[] = "QUERY_TYPE_SYSVAR_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

        /** Not implemented yet */
        //case QUERY_TYPE_SYSVAR_WRITE:
    case QUERY_TYPE_GSYSVAR_READ:
        {
            static const char name[] = "QUERY_TYPE_GSYSVAR_READ";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_GSYSVAR_WRITE:
        {
            static const char name[] = "QUERY_TYPE_GSYSVAR_WRITE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_BEGIN_TRX:
        {
            static const char name[] = "QUERY_TYPE_BEGIN_TRX";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_ENABLE_AUTOCOMMIT:
        {
            static const char name[] = "QUERY_TYPE_ENABLE_AUTOCOMMIT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_DISABLE_AUTOCOMMIT:
        {
            static const char name[] = "QUERY_TYPE_DISABLE_AUTOCOMMIT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_ROLLBACK:
        {
            static const char name[] = "QUERY_TYPE_ROLLBACK";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_COMMIT:
        {
            static const char name[] = "QUERY_TYPE_COMMIT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_PREPARE_NAMED_STMT:
        {
            static const char name[] = "QUERY_TYPE_PREPARE_NAMED_STMT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_PREPARE_STMT:
        {
            static const char name[] = "QUERY_TYPE_PREPARE_STMT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_EXEC_STMT:
        {
            static const char name[] = "QUERY_TYPE_EXEC_STMT";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_CREATE_TMP_TABLE:
        {
            static const char name[] = "QUERY_TYPE_CREATE_TMP_TABLE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_READ_TMP_TABLE:
        {
            static const char name[] = "QUERY_TYPE_READ_TMP_TABLE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_SHOW_DATABASES:
        {
            static const char name[] = "QUERY_TYPE_SHOW_DATABASES";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    case QUERY_TYPE_SHOW_TABLES:
        {
            static const char name[] = "QUERY_TYPE_SHOW_TABLES";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;

    default:
        {
            static const char name[] = "UNKNOWN_QUERY_TYPE";
            info.name = name;
            info.name_len = sizeof(name) - 1;
	}
	break;
    }

    return info;
}


const char* qc_type_to_string(qc_query_type_t type)
{
    return type_to_type_name_info(type).name;
}

static const qc_query_type_t QUERY_TYPES[] =
{
    /* Excluded by design */
    //QUERY_TYPE_UNKNOWN,
    QUERY_TYPE_LOCAL_READ,
    QUERY_TYPE_READ,
    QUERY_TYPE_WRITE,
    QUERY_TYPE_MASTER_READ,
    QUERY_TYPE_SESSION_WRITE,
    QUERY_TYPE_USERVAR_WRITE,
    QUERY_TYPE_USERVAR_READ,
    QUERY_TYPE_SYSVAR_READ,
    /** Not implemented yet */
    //QUERY_TYPE_SYSVAR_WRITE,
    QUERY_TYPE_GSYSVAR_READ,
    QUERY_TYPE_GSYSVAR_WRITE,
    QUERY_TYPE_BEGIN_TRX,
    QUERY_TYPE_ENABLE_AUTOCOMMIT,
    QUERY_TYPE_DISABLE_AUTOCOMMIT,
    QUERY_TYPE_ROLLBACK,
    QUERY_TYPE_COMMIT,
    QUERY_TYPE_PREPARE_NAMED_STMT,
    QUERY_TYPE_PREPARE_STMT,
    QUERY_TYPE_EXEC_STMT,
    QUERY_TYPE_CREATE_TMP_TABLE,
    QUERY_TYPE_READ_TMP_TABLE,
    QUERY_TYPE_SHOW_DATABASES,
    QUERY_TYPE_SHOW_TABLES,
};

static const int N_QUERY_TYPES = sizeof(QUERY_TYPES) / sizeof(QUERY_TYPES[0]);
static const int QUERY_TYPE_MAX_LEN = 29; // strlen("QUERY_TYPE_PREPARE_NAMED_STMT");

char* qc_typemask_to_string(uint32_t types)
{
    int len = 0;

    // First calculate how much space will be needed.
    for (int i = 0; i < N_QUERY_TYPES; ++i)
    {
        if (types & QUERY_TYPES[i])
        {
            if (len != 0)
            {
                ++len; // strlen("|");
            }

            len += QUERY_TYPE_MAX_LEN;
        }
    }

    ++len;

    // Then make one allocation and build the string.
    char* s = (char*) MXS_MALLOC(len);

    if (s)
    {
        if (len > 1)
        {
            char* p = s;

            for (int i = 0; i < N_QUERY_TYPES; ++i)
            {
                qc_query_type_t type = QUERY_TYPES[i];

                if (types & type)
                {
                    if (p != s)
                    {
                        strcpy(p, "|");
                        ++p;
                    }

                    struct type_name_info info = type_to_type_name_info(type);

                    strcpy(p, info.name);
                    p += info.name_len;
                }
            }
        }
        else
        {
            *s = 0;
        }
    }

    return s;
}
