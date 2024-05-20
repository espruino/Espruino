DEFINES += -DUSE_CRYPTO

INCLUDE += -I$(ROOT)/libs/crypto
#INCLUDE += -I$(ESP_IDF_PATH)/components/mbedtls
#INCLUDE += -I$(ESP_IDF_PATH)/components/mbedtls/mbedtls/include

#DEFINES += -DMBEDTLS_CIPHER_MODE_CTR \
#-DMBEDTLS_CIPHER_MODE_CBC \
#-DMBEDTLS_CIPHER_MODE_CFB

WRAPPERSOURCES += libs/crypto/jswrap_crypto.c
 
ifdef USE_SHA256
  DEFINES += -DUSE_SHA256
endif

ifdef USE_SHA512
  DEFINES += -DUSE_SHA512
endif

ifdef USE_TLS
  USE_AES=1
  DEFINES += -DUSE_TLS
endif

ifdef USE_AES
  DEFINES += -DUSE_AES
endif
