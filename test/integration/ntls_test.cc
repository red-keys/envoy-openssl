#include "test/integration/http_integration.h"
#include "test/test_common/utility.h"
#include "gtest/gtest.h"
#include "source/common/tls/server_context_config_impl.h"  
#include "source/common/tls/server_ssl_socket.h"

namespace Envoy {

class NtlsIntegrationTest : public testing::TestWithParam<Network::Address::IpVersion>,
                           public HttpIntegrationTest {
public:
  NtlsIntegrationTest()   
      : HttpIntegrationTest(Http::CodecType::HTTP1, GetParam(), ntlsConfig()) {}  

  void createUpstreams() override {  
    // 创建支持 NTLS 的上游 TLS 上下文  
    addFakeUpstream(createUpstreamNtlsContext(), Http::CodecType::HTTP1, /*autonomous_upstream=*/false);  
  }  

private:  

  Network::DownstreamTransportSocketFactoryPtr createUpstreamNtlsContext() {  
    envoy::extensions::transport_sockets::tls::v3::DownstreamTlsContext tls_context;  
    auto* common_tls_context = tls_context.mutable_common_tls_context();  
      
    // 配置 NTLS 相关设置  
    common_tls_context->set_ntls_enabled(true);  
    common_tls_context->add_alpn_protocols("http/1.1");  
      
    // 添加服务端证书（对应你配置中的客户端证书）  
    auto* tls_cert = common_tls_context->add_tls_certificates();  
    tls_cert->mutable_certificate_chain()->set_filename(  
        TestEnvironment::runfilesPath("test/config/integration/ntls_certs/sign_server.crt"));  
    tls_cert->mutable_private_key()->set_filename(  
        TestEnvironment::runfilesPath("test/config/integration/ntls_certs/sign_server.key"));  
    tls_cert->set_certificate_usage(envoy::extensions::transport_sockets::tls::v3::TlsCertificate::SIGN);  
      
    // 添加加密证书  
    auto* enc_cert = common_tls_context->add_tls_certificates();  
    enc_cert->mutable_certificate_chain()->set_filename(  
        TestEnvironment::runfilesPath("test/config/integration/ntls_certs/enc_server.crt"));  
    enc_cert->mutable_private_key()->set_filename(  
        TestEnvironment::runfilesPath("test/config/integration/ntls_certs/enc_server.key"));  
    enc_cert->set_certificate_usage(envoy::extensions::transport_sockets::tls::v3::TlsCertificate::ENCRYPT);  
      
    // 配置验证上下文  
    auto* validation_context = common_tls_context->mutable_validation_context();  
    validation_context->mutable_trusted_ca()->set_filename(  
        TestEnvironment::runfilesPath("test/config/integration/ntls_certs/ca.crt"));  
      
    // 要求客户端证书  
    tls_context.mutable_require_client_certificate()->set_value(true);  
      
    auto cfg = *Extensions::TransportSockets::Tls::ServerContextConfigImpl::create(  
        tls_context, factory_context_, false);  
        
    static auto* upstream_stats_store = new Stats::TestIsolatedStoreImpl();  
    return *Extensions::TransportSockets::Tls::ServerSslSocketFactory::create(  
        std::move(cfg), context_manager_, *upstream_stats_store->rootScope(),  
        std::vector<std::string>{});  
  }  

  static std::string ntlsConfig() {  
    return R"EOF(  
static_resources:  
  listeners:  
  - name: https  
    address:  
      socket_address:  
        address: 0.0.0.0  
        port_value: 0  
    filter_chains:  
    - filters:  
      - name: envoy.filters.network.http_connection_manager  
        typed_config:  
          "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager  
          codec_type: AUTO  
          stat_prefix: ingress_http  
          route_config:  
            name: local_route  
            virtual_hosts:  
            - name: local_service  
              domains: ["*"]  
              routes:  
              - match:  
                  prefix: "/"  
                route:  
                  cluster: ntls_backend_service  
          http_filters:  
          - name: envoy.filters.http.router  
            typed_config:  
              "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router  
      transport_socket:  
        name: envoy.transport_sockets.tls  
        typed_config:  
          "@type": type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.DownstreamTlsContext  
          common_tls_context:  
            tls_params:  
              cipher_suites:  
              - "ALL"  
            alpn_protocols: ["http/1.1"]  
            ntls_enabled: true  
            tls_certificates:  
            - certificate_chain:  
                filename: "test/config/integration/ntls_certs/sign_server.crt"  
              private_key:  
                filename: "test/config/integration/ntls_certs/sign_server.key"  
              certificate_usage: SIGN  
            - certificate_chain:  
                filename: "test/config/integration/ntls_certs/enc_server.crt"  
              private_key:  
                filename: "test/config/integration/ntls_certs/enc_server.key"  
              certificate_usage: ENCRYPT  
            validation_context:  
              trusted_ca: { filename: "test/config/integration/ntls_certs/ca.crt" }  
          require_client_certificate: true  
  clusters:  
  - name: ntls_backend_service  
    connect_timeout: 5s  
    type: STATIC  
    lb_policy: ROUND_ROBIN  
    load_assignment:  
      cluster_name: ntls_backend_service  
      endpoints:  
      - lb_endpoints:  
        - endpoint:  
            address:  
              socket_address:  
                address: 127.0.0.1  
                port_value: 0  
    transport_socket:  
      name: envoy.transport_sockets.tls  
      typed_config:  
        "@type": type.googleapis.com/envoy.extensions.transport_sockets.tls.v3.UpstreamTlsContext  
        common_tls_context:  
          ntls_enabled: true  
          tls_params:  
            cipher_suites:  
            - "ALL"  
          alpn_protocols: ["http/1.1"]  
          tls_certificates:  
          - certificate_chain:  
              filename: "test/config/integration/ntls_certs/sign_client.crt"  
            private_key:  
              filename: "test/config/integration/ntls_certs/sign_client.key"  
            certificate_usage: SIGN  
          - certificate_chain:  
              filename: "test/config/integration/ntls_certs/enc_client.crt"  
            private_key:  
              filename: "test/config/integration/ntls_certs/enc_client.key"  
            certificate_usage: ENCRYPT  
          validation_context:  
            trusted_ca: { filename: "test/config/integration/ntls_certs/ca.crt" }  
admin:  
  address:  
    socket_address:  
      address: 0.0.0.0  
      port_value: 9901  
)EOF";  
  }
};

TEST_P(NtlsIntegrationTest, BasicNtlsTest) {  

  initialize();
  registerTestServerPorts({"https"});
  
  // 获取上游端口
  const uint32_t upstream_port = fake_upstreams_[0]->localAddress()->ip()->port();
  ENVOY_LOG_MISC(info, "Upstream listening on port: {}", upstream_port);
  
  // 创建客户端连接
  codec_client_ = makeHttpConnection(lookupPort("https"));
  
  // 发送请求
  auto response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "GET"}, {":path", "/"}, {":authority", "test.com"}});
  
  // 等待上游连接
  bool upstream_connected = fake_upstreams_[0]->waitForHttpConnection(
      *dispatcher_, fake_upstream_connection_, std::chrono::seconds(10));
  
  if (!upstream_connected) {
    // 添加详细的诊断信息
    ENVOY_LOG_MISC(critical, "Failed to establish upstream connection. Possible causes:");
    ENVOY_LOG_MISC(critical, "1. Check NTLS handshake logs");
    ENVOY_LOG_MISC(critical, "2. Verify backend cluster configuration");
    ENVOY_LOG_MISC(critical, "3. Ensure certificates are valid and accessible");
    
    // 检查监听器是否激活
    test_server_->waitForCounterGe("listener_manager.listener_create_success", 1);
  }
  
  ASSERT_TRUE(upstream_connected);
  ASSERT_TRUE(fake_upstream_connection_->waitForNewStream(*dispatcher_, upstream_request_));
  ASSERT_TRUE(upstream_request_->waitForEndStream(*dispatcher_));
  
  // 发送上游响应
  upstream_request_->encodeHeaders(Http::TestResponseHeaderMapImpl{{":status", "200"}}, true);
  
  // 等待下游响应
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().getStatusValue());
}

INSTANTIATE_TEST_SUITE_P(IpVersions, NtlsIntegrationTest,
                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                        TestUtility::ipTestParamsToString);

} // namespace Envoy