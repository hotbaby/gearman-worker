/*************************************************************************
> File Name: gworker.c
> Author: yy
> Mail: mengyy_linux@163.com
************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgearman/gearman.h>

#include <lauxlib.h>
#include <lua.h>

#define TRUE            (1)
#define FALSE           (0)
#define BUF_SIZE        (32*1024)   /* 32k */
#define MODULE_NAME     ("worker")
#define WORKER_META     ("worker.meta")

/*
* Abandon this code.
* Do not use hybrid enconding with C and Lua any more, and realize a gearman lua lib, instead.
* Avoid the efficiency problem of process creation and distruction.
*/
typedef struct _worker_ctx_t
{
    bool              initialized;
    const char        *module_name;
    gearman_worker_st worker;           /* worker ctx */
    const char        *host;            /* server ip address */
    in_port_t         port;             /* server port */
    bool              ssl;
    int               timeout;
    const char        *function;
    char              buf[BUF_SIZE];    /* message buffer */
}worker_ctx_t;


static void stackDump (lua_State *L);
static void* worker_cb(gearman_job_st *job, void *context,
                       size_t *result_size, gearman_return_t *ret_ptr)
{
    int ret;
    size_t buf_size;
    char  *workload = NULL;
    const char *outbuf = NULL;
    int nargs;
    int nresults;

    lua_State *L = (lua_State*)context;
    if(NULL == L) {
        *ret_ptr = GEARMAN_INVALID_ARGUMENT;
        return NULL;
    }

    buf_size = gearman_job_workload_size(job);
    workload = (char*)malloc(buf_size + 1);
    if(NULL == workload) {
        fprintf(stderr, "can not alloc work load\n");
        *ret_ptr = GEARMAN_WORK_ERROR;
        return NULL;
    }
    memcpy(workload, gearman_job_workload(job), buf_size);
    workload[buf_size] = '\0';

    if (!workload || buf_size <= 0) {
        *ret_ptr = GEARMAN_INVALID_ARGUMENT;
        return NULL;
    }

    if (lua_type(L, 2) != LUA_TFUNCTION) {
        *ret_ptr = GEARMAN_INVALID_ARGUMENT;
        return NULL;
    }

    lua_pushvalue(L,2);
    lua_pushnumber(L, buf_size + 1);
    lua_pushstring(L, workload);
    if (workload) {
        free(workload);
    }

    nargs = 2;
    nresults = 1;
    ret = lua_pcall(L, nargs, nresults, 0);
    if (ret != 0) {
        switch (ret) {
            case LUA_ERRRUN:
            fprintf(stderr, "a runtime error, %s.\n", lua_tostring(L, -1));
            break;

            case LUA_ERRMEM:
            fprintf(stderr, "memory allocation error, %s.\n", lua_tostring(L, -1));
            break;

            case LUA_ERRERR:
            fprintf(stderr, "error while running the error handler function,\
                    %s.\n", lua_tostring(L, -1));
            break;

            default:
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            break;
        }
        goto call_error;
    }

    if (lua_isstring(L, -1)) {
        outbuf = strdup(lua_tostring(L, -1));
        *result_size = strlen(outbuf); /* just support string result */
    }
    else {
        fprintf(stderr, "function return result error.\n");
        goto call_error;
    }

    lua_pop(L, 1); /* pop result value */

    *ret_ptr = GEARMAN_SUCCESS;
    return (void*)outbuf;

call_error:
    *ret_ptr = GEARMAN_INVALID_COMMAND; /* TODO error code ? */
    return NULL;
}

static void stackDump (lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
                                int t = lua_type(L, i);
                                switch (t) {

                                    case LUA_TSTRING:  /* strings */
                                    fprintf(stderr, "`%s'", lua_tostring(L, i));
                                    break;

                                    case LUA_TBOOLEAN:  /* booleans */
                                    fprintf(stderr,lua_toboolean(L, i) ? "true" : "false");
                                    break;

                                    case LUA_TNUMBER:  /* numbers */
                                    fprintf(stderr, "%g", lua_tonumber(L, i));
                                    break;

                                    default:  /* other values */
                                    fprintf(stderr, "%s", lua_typename(L, t));
                                    break;

                                }
                                fprintf(stderr, "  ");	/* put a separator */
                               }
    fprintf(stderr, "\n");  /* put a separator */
}

static int worker_lua_gc(lua_State *L)
{
    worker_ctx_t *router_worker = (worker_ctx_t*)lua_touserdata(L, 1);
    if (router_worker != NULL)
    {
        gearman_worker_free(&router_worker->worker);
    }

    return 0;
}

static int worker_lua_initialize(lua_State *L)
{
    int argc;
    int ret;
    worker_ctx_t* router_worker = NULL;

    argc = lua_gettop(L);
    if (argc != 5) {
        lua_pushboolean(L, FALSE);
        return 1;
    }

    router_worker = (worker_ctx_t*)lua_newuserdata(L, sizeof(worker_ctx_t));
    if (NULL == router_worker) {
        fprintf(stderr, "can not alloc user data for worker ctx\n");
        lua_pushboolean(L, FALSE);
        return 1;
    }

    luaL_getmetatable(L, WORKER_META);
    lua_setmetatable(L, -2);

    router_worker->module_name = MODULE_NAME;
    router_worker->host = strdup(lua_tostring(L, 1));
    router_worker->port = (in_port_t)lua_tointeger(L, 2);
    router_worker->ssl  = lua_toboolean(L, 3);
    router_worker->timeout = lua_tointeger(L, 4);
    router_worker->function = strdup(lua_tostring(L, 5));

    /* debug g_router_worker */
    fprintf(stderr, "host:%s, port:%d, ssl:%d, timeout:%d\n",
            router_worker->host,
            router_worker->port,
            router_worker->ssl,
            router_worker->timeout);

    if (NULL == gearman_worker_create(&router_worker->worker)) {
        fprintf(stderr, "gearman worker create failed\n");
        lua_pushboolean(L, FALSE);
        return 1;
    }

    if (router_worker->timeout > 0) {
        gearman_worker_set_timeout(&router_worker->worker, router_worker->timeout);
    }

    if (router_worker->ssl) {
        gearman_worker_add_options(&router_worker->worker, GEARMAN_WORKER_SSL);
    }

    if (gearman_worker_add_server(&router_worker->worker, router_worker->host, router_worker->port) != GEARMAN_SUCCESS) {
        fprintf(stderr, "gearman worker add server failed\n");
        lua_pushboolean(L, FALSE);
        return 1;
    }

    if (gearman_worker_add_function(&router_worker->worker, router_worker->function, 0, worker_cb, (void*)L) != GEARMAN_SUCCESS) {
        fprintf(stderr, "gearman worker add function failed\n");
        lua_pushboolean(L, FALSE);
        return 1;
    }
    router_worker->initialized = TRUE;
    lua_pushboolean(L, TRUE);

    stackDump(L);
    return 2; /* number of results */
}

static int worker_lua_work_loop(lua_State *L)
{
    gearman_return_t rc;

    int argc;

    argc = lua_gettop(L);

    stackDump(L);
    worker_ctx_t* router_worker = (worker_ctx_t *)lua_touserdata(L, 1);
    if (!router_worker->initialized) {
        fprintf(stderr, "router worker not initialize.\n");
        lua_pushboolean(L, FALSE);
        return 1;
    }


    rc = gearman_worker_work(&router_worker->worker);
    if (gearman_failed(rc)) {
        fprintf(stderr, "%s\n", gearman_strerror(rc));
        fprintf(stderr, "Gearman router worker work error.\n");
    }

    lua_pushboolean(L, TRUE);
    return 1;
}

static const luaL_Reg worker[] = {
    {"__gc",       worker_lua_gc},
    {"initialize", worker_lua_initialize},
    {"loop",       worker_lua_work_loop},
    {NULL, NULL}
};

int luaopen_worker(lua_State *L)
{
    luaL_newmetatable(L, WORKER_META);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, worker);
    lua_pop(L, 1);

    luaL_register(L, MODULE_NAME, worker);

    return 0;
}
