include config.mk

CFLAGS := -I/usr/local/include
LDFLAGS := -L/usr/local/lib
CFLAGS += -I`pg_config --includedir`
LDFLAGS += -L`pg_config --libdir` -lpq

all: printconfig auth-plugin.so

printconfig:
	@echo "Bulding for $(UNAME)"

auth-plugin.so: auth_plugin.c
	${CROSS_COMPILE}${CC} $(CFLAGS) $(LDFLAGS) -fPIC -shared $< -o $@


clean:
	rm -rf auth-plugin.so

s:
	mosquitto -c mosquitto.conf
