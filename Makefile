CC = gcc
RPCGEN = rpcgen

SRCDIR = src

# Intentamos ser compatibles tanto con entornos clásicos como con tirpc
RPC_INC =
RPC_LIBS = -lnsl

ifeq ($(wildcard /usr/include/tirpc/rpc/rpc.h),/usr/include/tirpc/rpc/rpc.h)
	RPC_INC = -I/usr/include/tirpc
	RPC_LIBS = -ltirpc -lnsl
endif

CFLAGS = -Wall -Werror -fPIC -g -pthread $(RPC_INC) -I$(SRCDIR)
CFLAGS_RPC = -Wall -fPIC -g -pthread $(RPC_INC) -I$(SRCDIR)
LDFLAGS = -L. -Wl,-rpath,.
LDLIBS = $(RPC_LIBS) -pthread

LIB = libclaves.so
CLIENT = app-cliente
SERVER = clavesRPC_server

RPC_X = $(SRCDIR)/clavesRPC.x
RPC_HDR = $(SRCDIR)/clavesRPC.h
RPC_CLNT_C = $(SRCDIR)/clavesRPC_clnt.c
RPC_SVC_C = $(SRCDIR)/clavesRPC_svc.c
RPC_XDR_C = $(SRCDIR)/clavesRPC_xdr.c

APP_CLIENT_O = $(SRCDIR)/app-cliente.o
CLAVES_O = $(SRCDIR)/claves.o
PROXY_O = $(SRCDIR)/proxy-rpc.o
RPC_CLNT_O = $(SRCDIR)/clavesRPC_clnt.o
RPC_SVC_O = $(SRCDIR)/clavesRPC_svc.o
RPC_XDR_O = $(SRCDIR)/clavesRPC_xdr.o
RPC_SERVER_O = $(SRCDIR)/clavesRPC_server.o

all: $(LIB) $(CLIENT) $(SERVER)

# ==================================================
# 1. GENERACIÓN RPC (CAMBIO 1: no sobreescribe clavesRPC_server.c)
# ==================================================

rpcgen:
	cd $(SRCDIR) && $(RPCGEN) -NM -c clavesRPC.x -o clavesRPC_xdr.c
	cd $(SRCDIR) && $(RPCGEN) -NM -h clavesRPC.x -o clavesRPC.h
	cd $(SRCDIR) && $(RPCGEN) -NM -l clavesRPC.x -o clavesRPC_clnt.c
	cd $(SRCDIR) && $(RPCGEN) -NM -s tcp clavesRPC.x -o clavesRPC_svc.c

# ==================================================
# 2. BIBLIOTECA CLIENTE
# ==================================================

$(LIB): $(PROXY_O) $(RPC_CLNT_O) $(RPC_XDR_O)
	$(CC) -shared -o $@ $^ $(LDLIBS)

# ==================================================
# 3. OBJETOS
# ==================================================

$(APP_CLIENT_O): $(SRCDIR)/app-cliente.c $(SRCDIR)/claves.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLAVES_O): $(SRCDIR)/claves.c $(SRCDIR)/claves.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROXY_O): $(SRCDIR)/proxy-rpc.c $(SRCDIR)/claves.h $(RPC_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RPC_CLNT_O): $(RPC_CLNT_C) $(RPC_HDR)
	$(CC) $(CFLAGS_RPC) -c $< -o $@

$(RPC_SVC_O): $(RPC_SVC_C) $(RPC_HDR)
	$(CC) $(CFLAGS_RPC) -c $< -o $@

$(RPC_XDR_O): $(RPC_XDR_C) $(RPC_HDR)
	$(CC) $(CFLAGS_RPC) -c $< -o $@

$(RPC_SERVER_O): $(SRCDIR)/clavesRPC_server.c $(SRCDIR)/claves.h $(RPC_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# ==================================================
# 4. EJECUTABLES
# ==================================================

$(CLIENT): $(APP_CLIENT_O) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(APP_CLIENT_O) -lclaves $(LDLIBS)

$(SERVER): $(RPC_SERVER_O) $(CLAVES_O) $(RPC_SVC_O) $(RPC_XDR_O)
	$(CC) -o $@ $^ $(LDLIBS)

# ==================================================
# 5. LIMPIEZA
# ==================================================

clean:
	rm -f $(SRCDIR)/*.o
	rm -f $(LIB) $(CLIENT) $(SERVER)

distclean: clean
	rm -f $(SRCDIR)/clavesRPC.h
	rm -f $(SRCDIR)/clavesRPC_clnt.c
	rm -f $(SRCDIR)/clavesRPC_svc.c
	rm -f $(SRCDIR)/clavesRPC_xdr.c
	rm -f $(SRCDIR)/clavesRPC_client.c
	rm -f $(SRCDIR)/Makefile.clavesRPC

.PHONY: all clean distclean rpcgen
