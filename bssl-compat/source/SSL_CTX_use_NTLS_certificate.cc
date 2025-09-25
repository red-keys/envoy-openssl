#include <openssl/ssl.h>
#include <ossl.h>

#define X509v3_KU_DIGITAL_SIGNATURE 0x0080
#define X509v3_KU_KEY_CERT_SIGN 0x0004
#define X509v3_KU_CRL_SIGN 0x0002
#define X509v3_KU_KEY_ENCIPHERMENT 0x0020
#define X509v3_KU_DATA_ENCIPHERMENT 0x0010

extern "C" int SSL_CTX_use_NTLS_certificate(SSL_CTX *ctx, X509 *x509, int ntls_enabled, uint32_t key_usage) {  
  if(ntls_enabled) {  
    int sign_set = 0, enc_set = 0;  
      
    if((key_usage & X509v3_KU_DIGITAL_SIGNATURE)   
      || (key_usage & X509v3_KU_KEY_CERT_SIGN)   
      || (key_usage & X509v3_KU_CRL_SIGN)) {  
      if(0 == ossl.ossl_SSL_CTX_use_sign_certificate(ctx, x509)) {  
        return 0;  
      }  
      sign_set = 1;  
    }  
       
    if((key_usage & X509v3_KU_KEY_ENCIPHERMENT)   
      || (key_usage & X509v3_KU_DATA_ENCIPHERMENT)) {  
      if(0 == ossl.ossl_SSL_CTX_use_enc_certificate(ctx, x509)) {  
        return 0;  
      }  
      enc_set = 1;  
    }  
    
    if(sign_set || enc_set) {  
      return 1;  
    }  
  }  
  
  int ret = ossl.ossl_SSL_CTX_use_certificate(ctx, x509);  
  return (ret == 1) ? 1 : 0;  
}