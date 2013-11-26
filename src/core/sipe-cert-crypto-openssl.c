/**
 * @file sipe-cert-crypto-openssl.c
 *
 * pidgin-sipe
 *
 * Copyright (C) 2013 SIPE Project <http://sipe.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * Certificate routines implementation based on OpenSSL.
 */
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <glib.h>

#include "sipe-backend.h"
#include "sipe-cert-crypto.h"

struct sipe_cert_crypto {
	RSA *key;
};

/*
 * This data structure is used in two different modes
 *
 *  a) certificate generated by the server from our Certificate Request
 *
 *     key              - reference to client RSA key, don't free!
 *     decoded          - certificate as OpenSSL data structure, must be freed
 *     raw              - certificate as DER encoded binary, must be freed
 *     length           - length of DER binary
 *
 *  b) server certificate
 *
 *     key              - reference to server public key, must be freed
 *     decoded          - certificate as OpenSSL data structure, must be freed
 *     raw              - NULL
 *     length           - modulus length of server public key
 */
struct certificate_openssl {
	RSA *key;
	EVP_PKEY *public;
	X509 *decoded;
	guchar *raw;
	gsize length;
};

struct sipe_cert_crypto *sipe_cert_crypto_init(void)
{
	struct sipe_cert_crypto *scc = g_new0(struct sipe_cert_crypto, 1);

	/* RSA parameters - should those be configurable? */
	SIPE_DEBUG_INFO_NOFORMAT("sipe_cert_crypto_init: generate key pair, this might take a while...");
	scc->key = RSA_generate_key(2048, 65537, NULL, NULL);

	if (scc->key) {
		SIPE_DEBUG_INFO_NOFORMAT("sipe_cert_crypto_init: key pair generated");
		return(scc);
	}

	SIPE_DEBUG_ERROR_NOFORMAT("sipe_cert_crypto_init: key generation failed");
	g_free(scc);
	return(NULL);
}

void sipe_cert_crypto_free(struct sipe_cert_crypto *scc)
{
	if (scc) {
		if (scc->key)
			RSA_free(scc->key);
		g_free(scc);
	}
}


gchar *sipe_cert_crypto_request(struct sipe_cert_crypto *scc,
				const gchar *subject)
{
	gchar *base64                   = NULL;

	/* TBD */
	(void) scc;
	(void) subject;

	return(base64);
}

void sipe_cert_crypto_destroy(gpointer certificate)
{
	struct certificate_openssl *co = certificate;

	if (co) {
		/* imported server certificate - mode (b) */
		if (!co->raw && co->key)
			RSA_free(co->key);
		if (co->decoded)
			X509_free(co->decoded);
		g_free(co->raw);
		g_free(co);
	}
}

/* generates certificate_openssl in mode (a) */
gpointer sipe_cert_crypto_decode(struct sipe_cert_crypto *scc,
				 const gchar *base64)
{
	struct certificate_openssl *co = g_new0(struct certificate_openssl, 1);
	const guchar *tmp;

	/* NOTE: d2i_X509(NULL, &in, len) autoincrements "in" */
	tmp = co->raw = g_base64_decode(base64, &co->length);
	co->decoded = d2i_X509(NULL, &tmp, co->length);

	if (!co->decoded) {
		sipe_cert_crypto_destroy(co);
		return(NULL);
	}

	co->key = scc->key;

	return(co);
}

/* generates certificate_openssl in mode (b) */
gpointer sipe_cert_crypto_import(const guchar *raw,
				 gsize length)
{
	struct certificate_openssl *co = g_new0(struct certificate_openssl, 1);
	EVP_PKEY *pkey;

	/* co->raw not needed as this is a server certificate */
	/* NOTE: d2i_X509(NULL, in, len) autoincrements "in" */
	co->decoded = d2i_X509(NULL, &raw, length);

	if (!co->decoded) {
		sipe_cert_crypto_destroy(co);
		return(NULL);
	}

	pkey = X509_get_pubkey(co->decoded);

	if (!pkey) {
		sipe_cert_crypto_destroy(co);
		return(NULL);
	}

	co->key    = EVP_PKEY_get1_RSA(pkey);
	co->length = EVP_PKEY_bits(pkey);
	EVP_PKEY_free(pkey);

	if (!co->key) {
		sipe_cert_crypto_destroy(co);
		return(NULL);
	}

	return(co);
}

gboolean sipe_cert_crypto_valid(gpointer certificate,
				guint offset)
{
	struct certificate_openssl *co = certificate;

	if (!co)
		return(FALSE);

	/* TBD */
	(void) offset;
	return(FALSE);
}

guint sipe_cert_crypto_expires(gpointer certificate)
{
	/* TBD */
	(void) certificate;
	return(0);
}

gsize sipe_cert_crypto_raw_length(gpointer certificate)
{
	return(((struct certificate_openssl *) certificate)->length);
}

const guchar *sipe_cert_crypto_raw(gpointer certificate)
{
	return(((struct certificate_openssl *) certificate)->raw);
}

gpointer sipe_cert_crypto_public_key(gpointer certificate)
{
	return(((struct certificate_openssl *) certificate)->key);
}

gsize sipe_cert_crypto_modulus_length(gpointer certificate)
{
	return(((struct certificate_openssl *) certificate)->length);
}

gpointer sipe_cert_crypto_private_key(gpointer certificate)
{
	return(((struct certificate_openssl *) certificate)->key);
}

/* Create test certificate for internal key pair (ONLY USE FOR TEST CODE!!!) */
gpointer sipe_cert_crypto_test_certificate(struct sipe_cert_crypto *scc)
{
	struct certificate_openssl *co = NULL;
	EVP_PKEY *pkey;

	if ((pkey = EVP_PKEY_new()) != NULL) {
		X509 *x509;

		if ((x509 = X509_new()) != NULL) {
			X509_NAME *name;

			EVP_PKEY_set1_RSA(pkey, scc->key);

			X509_set_version(x509, 2);
			ASN1_INTEGER_set(X509_get_serialNumber(x509), 0);
			X509_gmtime_adj(X509_get_notBefore(x509), 0);
			X509_gmtime_adj(X509_get_notAfter(x509),  (long) 60*60*24);
			X509_set_pubkey(x509, pkey);

			name = X509_get_subject_name(x509);
			X509_NAME_add_entry_by_txt(name,
						   "CN",
						   MBSTRING_ASC,
						   (guchar *) "test@test.com",
						   -1, -1, 0);
			X509_set_issuer_name(x509, name);

			if (X509_sign(x509, pkey, EVP_sha1())) {
				guchar *buf;

				co = g_new0(struct certificate_openssl, 1);
				co->decoded = x509;
				co->key     = scc->key;

				/*
				 * Encode into DER format
				 *
				 * NOTE: i2d_X509(a, b) autoincrements b!
				 */
				co->length = i2d_X509(x509, NULL);
				co->raw = buf = g_malloc(co->length);
				i2d_X509(x509, &buf);

			} else {
				SIPE_DEBUG_ERROR_NOFORMAT("sipe_cert_crypto_test_certificate: can't sign certificate");
				X509_free(x509);
			}
		} else {
			SIPE_DEBUG_ERROR_NOFORMAT("sipe_cert_crypto_test_certificate: can't create x509 data structure");
		}

		EVP_PKEY_free(pkey);
	} else {
		SIPE_DEBUG_ERROR_NOFORMAT("sipe_cert_crypto_test_certificate: can't create private key data structure");
	}

	return(co);
}

/*
  Local Variables:
  mode: c
  c-file-style: "bsd"
  indent-tabs-mode: t
  tab-width: 8
  End:
*/
