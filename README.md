# mosquitto-pg-auth-plug
mosquitto PostgreSQL auth plugin

# Version support
- PostgreSQL 12
- mosquitto 2.0.6

# Build Tool
- GNU Make

# Compile

Just clone to local dir, get in `mosquitto-pg-auth-plug` dir, exec `gmake`!

After that,the `auth-plugin.so` file will create in dir.

# Configure `mosquitto.conf`
import this library in `mosquitto.conf` file. like:
```
auth_plugin /path/to/your/dir/auth-plugin.so
```
## plugin options
- `auth_opt_dsn` Data Source Name to connect PostgreSQL.look's like : `postgresql://postgres@192.168.1.2/data`
- `auth_opt_pwd_query` password query string
	- must have 1 param
	- suject add `limit 1` to suffix
	- i.e. : `auth_opt_pwd_query select pwd from mqtt.users where user_name=$1 and expire_at>now() limit 1`
- `auth_opt_acl_query` todo
