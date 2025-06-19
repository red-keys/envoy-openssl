#include <openssl/ssl.h>
#include <ossl.h>


extern "C" int SSL_CTX_set_ntls_max_proto_version(SSL_CTX *ctx, uint16_t version) {
  ctx->conf_max_version = version;  
  return 1;    
}