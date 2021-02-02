#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <mosquitto_broker.h>
#include <mosquitto.h>
#include <mosquitto_plugin.h>
#include <libpq-fe.h>

static struct pg_obj {
    PGconn *conn;
    char *dsn;
    char *password_query;
} pg;


void auth_log(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(va);
}

int pg_connect() {
    pg.conn = PQconnectdb(pg.dsn);
    if (PQstatus(pg.conn) != CONNECTION_OK) {
        auth_log("connect db fail: %s", PQerrorMessage(pg.conn));
        return MOSQ_ERR_NO_CONN;
    }

    // 准备语句
    PGresult *rs = PQprepare(pg.conn, "pwd_query", pg.password_query, 1, NULL);
    if (PQresultStatus(rs) != PGRES_COMMAND_OK) {
        auth_log("prepare pwd_query fail: %s", PQerrorMessage(pg.conn));
        return MOSQ_ERR_CONN_LOST;
    }
    PQclear(rs);

    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_plugin_version(void) {
    auth_log("mosquitto_auth_plugin_version");
    return MOSQ_AUTH_PLUGIN_VERSION;
}

int mosquitto_auth_plugin_init(void **user_data, struct mosquitto_opt *opts, int opt_count) {
    auth_log("mosquitto_auth_plugin_init");
    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count) {
    auth_log("mosquitto_auth_plugin_cleanup");
    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_init(void *user_data, struct mosquitto_opt *opts, int opt_count, bool reload) {
    auth_log("mosquitto_auth_security_init");
    for (int i = 0; i < opt_count; ++i) {
        if (strcmp(opts->key, "dsn") == 0) {
            // 数据库参数，立即连接
            pg.dsn = opts->value;
        } else if (strcmp(opts->key, "pwd_query") == 0) {
            pg.password_query = opts->value;
        }
        opts++;
    }
    if (pg_connect() != MOSQ_ERR_SUCCESS) {
        return MOSQ_ERR_NO_CONN;
    }
    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count, bool reload) {
    auth_log("mosquitto_auth_security_cleanup");
    PQfinish(pg.conn);
    return MOSQ_ERR_SUCCESS;
}

int
mosquitto_auth_acl_check(void *user_data, int access, struct mosquitto *client, const struct mosquitto_acl_msg *msg) {
    auth_log("mosquitto_auth_acl_check");
    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_unpwd_check(void *user_data, struct mosquitto *client, const char *username, const char *password) {
    auth_log("mosquitto_auth_unpwd_check");
    if (username == NULL || password == NULL) {
        return MOSQ_ERR_ACL_DENIED;
    }
    const char *paramValues[6];
    paramValues[0] = username;
    PGresult *rs = PQexecPrepared(pg.conn, "pwd_query", 1, paramValues, NULL, NULL, 0);
    ExecStatusType rst = PQresultStatus(rs);
    auth_log("%d", rst);
    if (rst != PGRES_TUPLES_OK) {
        if (rst == PGRES_FATAL_ERROR) {
            if (pg_connect() == 0) {
                PQclear(rs);
                rs = PQexecPrepared(pg.conn, "pwd_query", 1, paramValues, NULL, NULL, 0);
                if (PQresultStatus(rs) != PGRES_TUPLES_OK) {
                    auth_log("db reconnect fail: %s", PQerrorMessage(pg.conn));
                    return MOSQ_ERR_AUTH;
                }
            } else {
                auth_log("db reconnect fail: %s", PQerrorMessage(pg.conn));
                return MOSQ_ERR_AUTH;
            }
        } else {
            auth_log("db query pwd_query fail: %x, %s", PQresultStatus(rs), PQerrorMessage(pg.conn));
            return MOSQ_ERR_AUTH;
        }
    }
    if (PQgetisnull(rs, 0, 0)) {
        auth_log("db query pwd_query fail: %s not found", username);
        return MOSQ_ERR_AUTH;
    }
    char *pwd = PQgetvalue(rs, 0, 0);
    auth_log("%s,%s,%s", username, password, pwd);
    if (strcmp(password, pwd) != 0) {
        return MOSQ_ERR_AUTH;
    }
    PQclear(rs);

    return MOSQ_ERR_SUCCESS;
}

int
mosquitto_auth_psk_key_get(void *user_data, struct mosquitto *client, const char *hint, const char *identity, char *key,
                           int max_key_len) {
    auth_log("mosquitto_auth_psk_key_get");
    return MOSQ_ERR_SUCCESS;
}

int
mosquitto_auth_start(void *user_data, struct mosquitto *client, const char *method, bool reauth, const void *data_in,
                     uint16_t data_in_len, void **data_out, uint16_t *data_out_len) {
    auth_log("mosquitto_auth_start");
    return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_continue(void *user_data, struct mosquitto *client, const char *method, const void *data_in,
                            uint16_t data_in_len, void **data_out, uint16_t *data_out_len) {
    auth_log("mosquitto_auth_continue");
    return MOSQ_ERR_SUCCESS;
}

