/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl.
 *
 * Change Date: 2019-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

/**
 *
 * @verbatim
 * Revision History
 *
 * Date         Who                 Description
 * 08-09-2014   Martin Brampton     Initial implementation
 *
 * @endverbatim
 */

// To ensure that ss_info_assert asserts also when builing in non-debug mode.
#if !defined(SS_DEBUG)
#define SS_DEBUG
#endif
#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maxscale_test.h>
#include <test_utils.h>
#include <service.h>
#include <gwdirs.h>

/**
 * test1    Allocate a service and do lots of other things
 *
  */
static int
test1()
{
    SERVICE     *service;
    SESSION     *session;
    DCB     *dcb;
    int     result;
    int     argc = 3;

    init_test_env(NULL);

    /* Service tests */
    ss_dfprintf(stderr,
                "testservice : creating service called MyService with router nonexistent");
    service = service_alloc("MyService", "non-existent");
    mxs_log_flush_sync();
    ss_info_dassert(NULL == service, "New service with invalid router should be null");
    ss_info_dassert(0 == service_isvalid(service), "Service must not be valid after incorrect creation");
    ss_dfprintf(stderr, "\t..done\nValid service creation, router testroute.");
    set_libdir(strdup("../../modules/routing/"));
    service = service_alloc("MyService", "testroute");
    mxs_log_flush_sync();
    ss_info_dassert(NULL != service, "New service with valid router must not be null");
    ss_info_dassert(0 != service_isvalid(service), "Service must be valid after creation");
    ss_info_dassert(0 == strcmp("MyService", service_get_name(service)), "Service must have given name");
    ss_dfprintf(stderr, "\t..done\nAdding protocol testprotocol.");
    ss_info_dassert(0 != serviceAddProtocol(service, "testprotocol", "localhost", 9876, "MySQL", NULL),
                    "Add Protocol should succeed");
    ss_info_dassert(0 != serviceHasProtocol(service, "testprotocol", "localhost", 9876),
                    "Service should have new protocol as requested");
    set_libdir(strdup("../../modules/protocol/"));
    serviceStartProtocol(service, "testprotocol", 9876);
    mxs_log_flush_sync();
    ss_dfprintf(stderr, "\t..done\nStarting Service.");
    result = serviceStart(service);
    mxs_log_flush_sync();
    ss_info_dassert(0 != result, "Start should succeed");
    serviceStop(service);
    mxs_log_flush_sync();
    ss_info_dassert(service->state == SERVICE_STATE_STOPPED, "Stop should succeed");
    result = serviceStartAll();
    mxs_log_flush_sync();
    ss_info_dassert(0 != result, "Start all should succeed");
    ss_dfprintf(stderr, "\t..done\nStopping Service.");
    serviceStop(service);
    ss_info_dassert(service->state == SERVICE_STATE_STOPPED, "Stop should succeed");
    ss_dfprintf(stderr, "\t..done\n");

    return 0;

}

int main(int argc, char **argv)
{
    int result = 0;

    result += test1();

    exit(result);
}
