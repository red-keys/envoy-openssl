#include <openssl/ssl.h>  
#include "bssl-compat/third_party/boringssl/src/ssl/internal.h"  


extern "C" int SSL_CTX_set_ntls_max_proto_version(SSL_CTX *ctx, uint16_t version) {
  ctx->conf_max_version = version;  
  return 1;    
}