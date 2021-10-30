#include "canivete.h"

#include <stdio.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "error.h"

static int canivete_init_openssl(struct canivete* ctx)
{
    SSL_library_init();
    // SSL_load_error_strings();
    // OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);

    canivete_log_debug(&ctx->logger, "openssl version: \"%s\"", OPENSSL_VERSION_TEXT);

    // initializes the list of ciphers, the session cache setting, the callbacks, the keys and certificates and the options to their default values.
    ctx->ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (ctx->ssl_ctx == NULL) {
        canivete_log_error(&ctx->logger, "openssl: SSL_CTX_new: ");
        ERR_print_errors_fp(ctx->logger.stream);
        return CANIVETE_ERR_OPENSSL;
    }

    SSL_CTX_set_verify_depth(ctx->ssl_ctx, 8);
    // TODO set verify paths and mode here
    // SSL_CTX_load_verify_locations

    return CANIVETE_OK;
}

void canivete_init_log(struct canivete* ctx)
{
    ctx->logger = canivete_log_default_logger();
}

int canivete_init(struct canivete* ctx)
{
    ctx->config = canivete_config_from_environment(&ctx->logger);

    if (canivete_init_openssl(ctx) != CANIVETE_OK) {
        canivete_log_error(&ctx->logger, "init: failed to init openssl context");
        return CANIVETE_ERR_OPENSSL;
    }

    return CANIVETE_OK;
}

void canivete_free(struct canivete* ctx)
{
    canivete_config_free(&ctx->config);
    SSL_CTX_free(ctx->ssl_ctx);
}

int canivete_get_server_certificates_chain(struct canivete* ctx)
{
    const char* server_addr = "google.com:443";

    // creates a new BIO chain consisting of an SSL BIO (using ctx) followed by a connect BIO.
    BIO* conn = BIO_new_ssl_connect(ctx->ssl_ctx);
    if (conn == NULL) {
        canivete_log_error(&ctx->logger, "openssl: BIO_new_ssl_connect: ");
        goto error;
    }
    BIO_set_conn_hostname(conn, server_addr);
    
    SSL* ssl = NULL;
    BIO_get_ssl(conn, &ssl);
    if (conn == NULL) {
        canivete_log_error(&ctx->logger, "openssl: BIO_get_ssl: ");
        goto error_cleanup_conn;
    }

    // SNI extension hostname
    SSL_set_tlsext_host_name(ssl, server_addr);

    if (BIO_do_handshake(conn) <= 0) {
        canivete_log_error(&ctx->logger, "openssl: BIO_do_handshake: error establishing TLS connection: ");
        goto error_cleanup_conn;
    }

    canivete_log_info(&ctx->logger, "connected to %s", server_addr);

    // The reference count of each certificate in the returned STACK_OF(X509) object is not incremented and
    // the returned stack may be invalidated by renegotiation.
    // If applications wish to use any certificates in the returned chain indefinitely they must
    // increase the reference counts using X509_up_ref() or obtain a copy of the whole chain with X509_chain_up_ref().
    STACK_OF(X509)* cert_chain = SSL_get_peer_cert_chain(ssl);
    if (cert_chain == NULL) {
        canivete_log_error(&ctx->logger, "openssl: SSL_get_peer_cert_chain: no certificate were found.");
        goto error_cleanup_conn;
    }

    int certs_count = sk_X509_num(cert_chain);
    canivete_log_info(&ctx->logger, "openssl: server's certificates chain has %d certificates", certs_count);

    for (int i = 0; i < certs_count; i++) {
        X509* cert = sk_X509_value(cert_chain, i);
        if (PEM_write_X509(stdout, cert) == 0) {
            canivete_log_error(&ctx->logger, "openssl: PEM_write_X509: failed to write certificate number %d", i);
            goto error_cleanup_conn;
        }
    }

    // closes down an SSL connection on BIO chain bio.
    // It does this by locating the SSL BIO in the chain and calling SSL_shutdown() on its internal SSL pointer.
    BIO_ssl_shutdown(conn);
    BIO_free_all(conn);
    return CANIVETE_OK;

error_cleanup_conn:
    BIO_ssl_shutdown(conn);
    BIO_free_all(conn);
error:
    ERR_print_errors_fp(ctx->logger.stream);
    return CANIVETE_ERR_OPENSSL;
}
