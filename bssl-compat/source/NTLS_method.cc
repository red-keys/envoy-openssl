#include <openssl/ssl.h>
#include <ossl.h>


extern "C" const SSL_METHOD *NTLS_method(void) {
  return ossl.ossl_NTLS_method();
}

extern "C" const SSL_METHOD *NTLS_server_method(void) {
    return ossl.ossl_NTLS_server_method();
}

extern "C" const SSL_METHOD *NTLS_client_method(void) {
    return ossl.ossl_NTLS_client_method();
}