  DEFINES += -DUSE_CRYPTO
  INCLUDE += -I$(ROOT)/libs/crypto
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls
  INCLUDE += -I$(ROOT)/libs/crypto/mbedtls/include
  WRAPPERSOURCES += libs/crypto/jswrap_crypto.c
  SOURCES += libs/crypto/mbedtls/library/sha1.c 
ifdef USE_SHA256
  DEFINES += -DUSE_SHA256
  SOURCES += libs/crypto/mbedtls/library/sha256.c
endif
ifdef USE_SHA512
  DEFINES += -DUSE_SHA512
  SOURCES += libs/crypto/mbedtls/library/sha512.c
endif

ifdef USE_TLS
  USE_AES=1
  DEFINES += -DUSE_TLS
  SOURCES += \
libs/crypto/mbedtls/library/bignum.c \
libs/crypto/mbedtls/library/ctr_drbg.c \
libs/crypto/mbedtls/library/debug.c \
libs/crypto/mbedtls/library/ecp.c \
libs/crypto/mbedtls/library/ecp_curves.c \
libs/crypto/mbedtls/library/entropy.c \
libs/crypto/mbedtls/library/entropy_poll.c \
libs/crypto/mbedtls/library/md5.c \
libs/crypto/mbedtls/library/pk.c \
libs/crypto/mbedtls/library/pkparse.c \
libs/crypto/mbedtls/library/pk_wrap.c \
libs/crypto/mbedtls/library/rsa.c \
libs/crypto/mbedtls/library/ssl_ciphersuites.c \
libs/crypto/mbedtls/library/ssl_cli.c \
libs/crypto/mbedtls/library/ssl_tls.c \
libs/crypto/mbedtls/library/ssl_srv.c \
libs/crypto/mbedtls/library/x509.c \
libs/crypto/mbedtls/library/x509_crt.c
endif
ifdef USE_AES
  DEFINES += -DUSE_AES
  SOURCES += \
libs/crypto/mbedtls/library/aes.c \
libs/crypto/mbedtls/library/asn1parse.c \
libs/crypto/mbedtls/library/cipher.c \
libs/crypto/mbedtls/library/cipher_wrap.c \
libs/crypto/mbedtls/library/md.c \
libs/crypto/mbedtls/library/md_wrap.c \
libs/crypto/mbedtls/library/oid.c \
libs/crypto/mbedtls/library/pkcs5.c
endif
