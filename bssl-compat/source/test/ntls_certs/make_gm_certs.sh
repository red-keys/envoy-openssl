#!/bin/bash
#WORKDIR="./ntls_certs"
#mkdir -p $WORKDIR && cd $WORKDIR || exit 1
env=$OPENSSL_CONF
export OPENSSL_CONF=/usr/local/tongsuo/ssl/openssl.cnf


# 生成根CA证书（SM2算法）
/usr/local/tongsuo/bin/openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:sm2 -out ca.key
/usr/local/tongsuo/bin/openssl req -x509 -new -key ca.key -out ca.crt -subj "/C=CN/ST=ZJ/L=HZ/O=NTLS_CA/CN=Root CA" -sigopt sm2_id:1234567812345678

# 生成服务端证书
## 签名证书
/usr/local/tongsuo/bin/openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:sm2 -out sign_server.key
/usr/local/tongsuo/bin/openssl req -new -key sign_server.key -out server_sign.csr -subj "/C=CN/ST=ZJ/L=HZ/O=Server/CN=localhost"
/usr/local/tongsuo/bin/openssl x509 -req -in server_sign.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out sign_server.crt -days 365

## 加密证书
/usr/local/tongsuo/bin/openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:sm2 -out enc_server.key
/usr/local/tongsuo/bin/openssl req -new -key enc_server.key -out server_enc.csr -subj "/C=CN/ST=ZJ/L=HZ/O=Server/CN=localhost"
/usr/local/tongsuo/bin/openssl x509 -req -in server_enc.csr -CA ca.crt -CAkey ca.key -out enc_server.crt -days 365

# 生成客户端证书
## 签名证书
/usr/local/tongsuo/bin/openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:sm2 -out sign_client.key
/usr/local/tongsuo/bin/openssl req -new -key sign_client.key -out client_sign.csr -subj "/C=CN/ST=ZJ/L=HZ/O=Client/CN=localhost"
/usr/local/tongsuo/bin/openssl x509 -req -in client_sign.csr -CA ca.crt -CAkey ca.key -out sign_client.crt -days 365

## 加密证书
/usr/local/tongsuo/bin/openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:sm2 -out enc_client.key
/usr/local/tongsuo/bin/openssl req -new -key enc_client.key -out client_enc.csr -subj "/C=CN/ST=ZJ/L=HZ/O=Client/CN=localhost"
/usr/local/tongsuo/bin/openssl x509 -req -in client_enc.csr -CA ca.crt -CAkey ca.key -out enc_client.crt -days 365

echo "证书生成完成，目录：$WORKDIR"

export OPENSSL_CONF=$env

# 检查 ca.crt 文件是否存在
if [ ! -f "ca.crt" ]; then
    echo "错误: ca.crt 文件不存在"
    exit 1
fi

# 生成 root_ca_cert.pem.h 文件
echo -n "static const char ntls_root_ca_cert_pem_str[] = R\"\"\"(" > root_ca_cert.pem.h
cat ca.crt >> root_ca_cert.pem.h
echo "" >> root_ca_cert.pem.h  # 添加换行符
echo ")\"\"\";" >> root_ca_cert.pem.h

echo "已生成 root_ca_cert.pem.h 文件"

# 检查 sign_server.crt enc_server.crt 文件是否存在
if [ ! -f "sign_server.crt" ]; then
    echo "错误: sign_server.crt 文件不存在"
    exit 1
fi
if [ ! -f "enc_server.crt" ]; then
    echo "错误: enc_server.crt 文件不存在"
    exit 1
fi
# 生成  server_2_cert_chain.pem.h 文件
echo -n "static const char ntls_server_2_cert_chain_pem_str[] = R\"\"\"(" >  server_2_cert_chain.pem.h
cat sign_server.crt >>  server_2_cert_chain.pem.h
echo "" >>  server_2_cert_chain.pem.h  # 添加换行符
cat enc_server.crt >>  server_2_cert_chain.pem.h
echo "" >>  server_2_cert_chain.pem.h  # 添加换行符
echo ")\"\"\";" >>  server_2_cert_chain.pem.h

echo "已生成  server_2_cert_chain.pem.h 文件"

# 检查 sign_server.key enc_server.key 文件是否存在
if [ ! -f "sign_server.key" ]; then
    echo "错误: sign_server.key 文件不存在"
    exit 1
fi
if [ ! -f "enc_server.key" ]; then
    echo "错误: enc_server.key 文件不存在"
    exit 1
fi
# 生成  server_2_key.pem.h 文件
echo -n "static const char ntls_server_2_key_pem_str[] = R\"\"\"(" >  server_2_key.pem.h
cat sign_server.key >>  server_2_key.pem.h
echo "" >>  server_2_key.pem.h  # 添加换行符
cat enc_server.key >>  server_2_key.pem.h
echo "" >>  server_2_key.pem.h  # 添加换行符
echo ")\"\"\";" >>  server_2_key.pem.h

echo "已生成  server_2_key.pem.h 文件"


# 检查 sign_client.crt enc_client.crt 文件是否存在
if [ ! -f "sign_client.crt" ]; then
    echo "错误: sign_client.crt 文件不存在"
    exit 1
fi
if [ ! -f "enc_client.crt" ]; then
    echo "错误: enc_client.crt 文件不存在"
    exit 1
fi
# 生成  client_2_cert_chain.pem.h 文件
echo -n "static const char ntls_client_2_cert_chain_pem_str[] = R\"\"\"(" >  client_2_cert_chain.pem.h
cat sign_client.crt >>  client_2_cert_chain.pem.h
echo "" >>  client_2_cert_chain.pem.h  # 添加换行符
cat enc_client.crt >>  client_2_cert_chain.pem.h
echo "" >>  client_2_cert_chain.pem.h  # 添加换行符
echo ")\"\"\";" >>  client_2_cert_chain.pem.h

echo "已生成  client_2_cert_chain.pem.h 文件"

# 检查 sign_client.key enc_client.key 文件是否存在
if [ ! -f "sign_client.key" ]; then
    echo "错误: sign_client.key 文件不存在"
    exit 1
fi
if [ ! -f "enc_client.key" ]; then
    echo "错误: enc_client.key 文件不存在"
    exit 1
fi
# 生成  client_2_key.pem.h 文件
echo -n "static const char ntls_client_2_key_pem_str[] = R\"\"\"(" >  client_2_key.pem.h
cat sign_client.key >>  client_2_key.pem.h
echo "" >>  client_2_key.pem.h  # 添加换行符
cat enc_client.key >>  client_2_key.pem.h
echo "" >>  client_2_key.pem.h  # 添加换行符
echo ")\"\"\";" >>  client_2_key.pem.h

echo "已生成  client_2_key.pem.h 文件"
