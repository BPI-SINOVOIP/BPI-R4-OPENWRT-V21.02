/*
 * Copyright 2022 Mediatek Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Simple AES GCM authenticated encryption with additional data (AEAD)
 * demonstration program.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>

#define MAX_TEXT_LENGTH		4096
#define MAX_AAD_LENGTH		256
#define MAX_TAG_LENGTH		32

#define ERR_ENC			1
#define ERR_DEC			2
#define ERR_UNK_MOD		3

typedef enum {
	UNK,
	ENCRYPT,
	DECRYPT
} OPERATION;

/*
 * A library context and property query can be used to select & filter
 * algorithm implementations. If they are NULL then the default library
 * context and properties are used.
 */
OSSL_LIB_CTX *libctx = NULL;
const char *propq = NULL;

int aes_gcm_encrypt(uint8_t *key, uint8_t *iv, long iv_len, uint8_t *aad,
		    long aad_len, uint8_t *pt, long pt_len, BIO *out)
{
	int ret = 0;
	EVP_CIPHER_CTX *ctx;
	EVP_CIPHER *cipher = NULL;
	int outlen, tmplen;
	uint8_t *outbuf;
	uint8_t *outtag[16];
	OSSL_PARAM params[2] = {
		OSSL_PARAM_END, OSSL_PARAM_END
	};

	outbuf = calloc(MAX_TEXT_LENGTH, sizeof(uint8_t));
	if (!outbuf)
		return 0;

	/* Create a context for the encrypt operation */
	if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
		goto err;

	/* Fetch the cipher implementation */
	if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL)
		goto err;

	/* Set IV length if default 96 bits is not appropriate */
	params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
						&iv_len);
	/*
	 * Initialise an encrypt operation with the cipher/mode, key, IV and
	 * IV length parameter.
	 * For demonstration purposes the IV is being set here. In a compliant
	 * application the IV would be generated internally so the iv passed in
	 * would be NULL.
	 */
	if (!EVP_EncryptInit_ex2(ctx, cipher, key, iv, params))
		goto err;

	/* Zero or more calls to specify any AAD */
	if (!EVP_EncryptUpdate(ctx, NULL, &outlen, aad, aad_len))
		goto err;

	/* Encrypt plaintext */
	if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, pt, pt_len))
		goto err;

	/* Finalise: note get no output for GCM */
	if (!EVP_EncryptFinal_ex(ctx, outbuf, &tmplen))
		goto err;

	/* Get tag */
	params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG,
						      outtag, 16);

	if (!EVP_CIPHER_CTX_get_params(ctx, params))
		goto err;

	/* Output IV */
	if (BIO_write(out, iv, iv_len) <= 0)
		goto err;

	/* Output tag */
	if (BIO_write(out, outtag, 16) <= 0)
		goto err;

	/* Output encrypted block */
	if (BIO_write(out, outbuf, outlen) <= 0)
		goto err;

	ret = 1;
err:
	if (!ret)
		ERR_print_errors_fp(stderr);

	free(outbuf);
	EVP_CIPHER_free(cipher);
	EVP_CIPHER_CTX_free(ctx);

	return ret;
}

int aes_gcm_decrypt(uint8_t *key, uint8_t *iv, long iv_len,
		    uint8_t *aad, long aad_len, uint8_t *ct, long ct_len,
		    uint8_t *tag, long tag_len, BIO *out)
{
	int ret = 0;
	EVP_CIPHER_CTX *ctx;
	EVP_CIPHER *cipher = NULL;
	int outlen, rv;
	uint8_t *outbuf;
	OSSL_PARAM params[2] = {
		OSSL_PARAM_END, OSSL_PARAM_END
	};

	outbuf = calloc(MAX_TEXT_LENGTH, sizeof(uint8_t));
	if (!outbuf)
		return 0;

	if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
		goto err;

	/* Fetch the cipher implementation */
	if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL)
		goto err;

	/* Set IV length if default 96 bits is not appropriate */
	params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
						&iv_len);

	/*
	 * Initialise an encrypt operation with the cipher/mode, key, IV and
	 * IV length parameter.
	 */
	if (!EVP_DecryptInit_ex2(ctx, cipher, key, iv, params))
		goto err;

	/* Zero or more calls to specify any AAD */
	if (!EVP_DecryptUpdate(ctx, NULL, &outlen, aad, aad_len))
		goto err;

	/* Decrypt plaintext */
	if (!EVP_DecryptUpdate(ctx, outbuf, &outlen, ct, ct_len))
		goto err;

	/* Output decrypted block */
	if (BIO_write(out, outbuf, outlen) <= 0)
		goto err;

	/* Set expected tag value. */
	params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG,
						      (void *)tag, tag_len);

	if (!EVP_CIPHER_CTX_set_params(ctx, params))
		goto err;

	/* Finalise: note get no output for GCM */
	rv = EVP_DecryptFinal_ex(ctx, outbuf, &outlen);
	/*
	 * Print out return value. If this is not successful authentication
	 * failed and plaintext is not trustworthy.
	 */
	printf("Tag Verify %s\n", rv > 0 ? "Successful!" : "Failed!");

	ret = rv > 0 ? 1 : 0;
err:
	if (!ret)
		ERR_print_errors_fp(stderr);

	free(outbuf);
	EVP_CIPHER_free(cipher);
	EVP_CIPHER_CTX_free(ctx);

	return ret;
}

void usage(void)
{
	printf(
		"simple aes-256-gcm tool: \n"
		"Operations:\n"
		"-e 		- encrypt\n"
		"-d 		- decrypt\n"
		"Common requirement parameters:\n"
		"-i infile	- input file\n"
		"-o outfile	- out file\n"
		"-k key(hex) 	- key in hex\n"
		"-n iv(hex)	- initial vector in hex\n"
		"-a aad(hex)	- additional authentication data in hex\n"
		"Decryption requirement paremters:\n"
		"-t intagfile 	- tag file as input\n");
}

int main(int argc, char **argv)
{
	int ret = 0, opt, oper = UNK;
	long key_len = 0, iv_len = 0, aad_len = 0, tag_len = 0, in_len = 0;
	BIO *in = NULL, *out = NULL, *in_tag = NULL;
	uint8_t *in_buf;
	uint8_t key[EVP_MAX_KEY_LENGTH] = {0};
	uint8_t iv[EVP_MAX_IV_LENGTH] = {0};
	uint8_t aad[MAX_AAD_LENGTH] = {0};
	uint8_t tag[MAX_TAG_LENGTH] = {0};

	in_buf = calloc(MAX_TEXT_LENGTH, sizeof(uint8_t));
	if (!in_buf)
		return -ENOMEM;

	while ((opt = getopt(argc, argv, "a:dei:g:k:n:o:t:")) > 0) {
		switch(opt) {
		case 'a':
			ret = OPENSSL_hexstr2buf_ex(aad, MAX_AAD_LENGTH,
						    &aad_len, optarg, '\0');
			if (!ret) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to read aad\n");
				goto end;
			}
			break;
		case 'd':
			if (oper) {
				ret = -EINVAL;
				fprintf(stderr, "Duplicate operations\n");
				goto end;
			}
			oper = DECRYPT;
			break;
		case 'e':
			if (oper) {
				ret = -EINVAL;
				fprintf(stderr, "Duplicate operations\n");
				goto end;
			}
			oper = ENCRYPT;
			break;
		case 'i':
			in = BIO_new_file(optarg, "rb");
			if (!in) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to open input file\n");
				goto end;
			}

			in_len = BIO_read(in, in_buf, MAX_TEXT_LENGTH);
			if (in_len <= 0) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to read input file\n");
				goto end;
			}
			break;
		case 'k':
			ret = OPENSSL_hexstr2buf_ex(key, EVP_MAX_KEY_LENGTH,
				    		    &key_len, optarg, '\0');
			if (!ret) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to read key\n");
				goto end;
			}
			break;
			case 'n':
			ret = OPENSSL_hexstr2buf_ex(iv, EVP_MAX_IV_LENGTH,
				    		    &iv_len, optarg, '\0');
			if (!ret) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to read iv\n");
				goto end;
			}
			break;
		case 'o':
			out = BIO_new_file(optarg, "w");
			if (!out) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to open output file\n");
				goto end;
			}
			break;
		case 't':
			in_tag = BIO_new_file(optarg, "rb");
			if (!in_tag) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to open tag file\n");
				goto end;
			}

			tag_len = BIO_read(in_tag, tag, MAX_TAG_LENGTH);
			if (tag_len <= 0) {
				ret = -EINVAL;
				fprintf(stderr, "Failed to read tag file\n");
				goto end;
			}
			break;
		default:
			break;
		}

	}

	if (!key_len || !iv_len || !aad_len || !in_len || !out) {
		ret = -EINVAL;
		goto end;
	}

	if (oper == ENCRYPT) {
		ret = aes_gcm_encrypt(key, iv, iv_len, aad, aad_len,
				      in_buf, in_len, out);
		if (!ret) {
			ret = -ERR_ENC;
			goto end;
		}
	} else if (oper == DECRYPT) {
		if (!tag_len) {
			ret = -EINVAL;
			goto end;
		}

		ret = aes_gcm_decrypt(key, iv, iv_len, aad, aad_len,
				      in_buf, in_len, tag, tag_len, out);
		if (!ret) {
			ret = -ERR_DEC;
			goto end;
		}
	} else {
		ret = -ERR_UNK_MOD;
		goto end;
	}

end:
	free(in_buf);
	if (in)
		BIO_free(in);
	if (out)
		BIO_free(out);
	if (in_tag)
		BIO_free(in_tag);

	if (ret == -EINVAL)
		usage();

	return ret;
}
