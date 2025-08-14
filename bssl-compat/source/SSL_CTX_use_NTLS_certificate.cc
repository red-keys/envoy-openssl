#include <openssl/ssl.h>
#include <ossl.h>


extern "C" int SSL_CTX_use_NTLS_certificate(SSL_CTX *ctx, X509 *x509, int ntls_enabled, int key_usage) {
  
  if(ntls_enabled) {
    if((key_usage & X509v3_KU_DIGITAL_SIGNATURE) 
      || (key_usage & X509v3_KU_KEY_CERT_SIGN) 
      || (key_usage & X509v3_KU_CRL_SIGN)) {
      if(0 == ossl.ossl_SSL_CTX_use_sign_certificate(ctx, x509)) {
        return 0;
      }
      else {
        return 1;
      }
    }
    if((key_usage & X509v3_KU_KEY_ENCIPHERMENT) 
      || (key_usage & X509v3_KU_DATA_ENCIPHERMENT)) {
      if(0 == ossl.ossl_SSL_CTX_use_enc_certificate(ctx, x509)) {
        return 0;
      }
      else {
        return 1;
      }
    }
  }

  int ret = ossl.ossl_SSL_CTX_use_certificate(ctx, x509);
  return (ret == 1) ? 1 : 0;
}