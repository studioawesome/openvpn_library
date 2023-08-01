//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication and key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    Copyright (C) 2012-2022 OpenVPN Inc.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License Version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program in the COPYING file.
//    If not, see <http://www.gnu.org/licenses/>.

#pragma once

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
#include <openssl/kdf.h>
#endif

namespace openvpn {
namespace OpenSSLCrypto {

class TLS1PRF
{
  public:
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    static bool PRF(unsigned char *label,
                    const size_t label_len,
                    const unsigned char *sec,
                    const size_t slen,
                    unsigned char *out1,
                    const size_t olen)
    {
        /* TODO use EVP_PKEY_CTX_new_from_name and library context for OpenSSL 3.0 but
         * this needs passing the library context down here.*/
        using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, decltype(&::EVP_PKEY_CTX_free)>;

        EVP_PKEY_CTX_ptr pctx(EVP_PKEY_CTX_new_id(EVP_PKEY_TLS1_PRF, NULL), ::EVP_PKEY_CTX_free);

        if (!pctx.get())
            return false;

        if (!EVP_PKEY_derive_init(pctx.get()))
            return false;

        if (!EVP_PKEY_CTX_set_tls1_prf_md(pctx.get(), EVP_md5_sha1()))
            return false;

        if (!EVP_PKEY_CTX_set1_tls1_prf_secret(pctx.get(), sec, slen))
            return false;

        if (!EVP_PKEY_CTX_add1_tls1_prf_seed(pctx.get(), label, label_len))
            return false;

        size_t out_len = olen;
        if (!EVP_PKEY_derive(pctx.get(), out1, &out_len))
            return false;

        if (out_len != olen)
            return false;

        return true;
    }
#else
    static bool PRF(unsigned char *label,
                    const size_t label_len,
                    const unsigned char *sec,
                    const size_t slen,
                    unsigned char *out1,
                    const size_t olen)
    {
        size_t len, i;
        const unsigned char *S1, *S2;
        unsigned char *out2;

        out2 = new unsigned char[olen];

        len = slen / 2;
        S1 = sec;
        S2 = &(sec[len]);
        len += (slen & 1); /* add for odd, make longer */

        hash(CryptoAlgs::MD5, S1, len, label, label_len, out1, olen);
        hash(CryptoAlgs::SHA1, S2, len, label, label_len, out2, olen);

        for (i = 0; i < olen; i++)
            out1[i] ^= out2[i];

        std::memset(out2, 0, olen);
        delete[] out2;
        return true;
    }

  private:
    /*
     * Use the TLS PRF function for generating data channel keys.
     * This code is adapted from the OpenSSL library.
     *
     * TLS generates keys as such:
     *
     * master_secret[48] = PRF(pre_master_secret[48], "master secret",
     *                         ClientHello.random[32] + ServerHello.random[32])
     *
     * key_block[] = PRF(SecurityParameters.master_secret[48],
     *                 "key expansion",
     *                 SecurityParameters.server_random[32] +
     *                 SecurityParameters.client_random[32]);
     *
     * Notes:
     *
     * (1) key_block contains a full set of 4 keys.
     * (2) The pre-master secret is generated by the client.
     */

    static void hash(const CryptoAlgs::Type md,
                     const unsigned char *sec,
                     const size_t sec_len,
                     const unsigned char *seed,
                     const size_t seed_len,
                     unsigned char *out,
                     size_t olen)
    {
        size_t j;
        unsigned char A1[HMACContext::MAX_HMAC_SIZE];
        size_t A1_len;
        HMACContext ctx;
        HMACContext ctx_tmp;
        const size_t chunk = CryptoAlgs::size(md);

        ctx.init(md, sec, sec_len);
        ctx_tmp.init(md, sec, sec_len);
        ctx.update(seed, seed_len);
        A1_len = ctx.final(A1);

        for (;;)
        {
            ctx.reset();
            ctx_tmp.reset();
            ctx.update(A1, A1_len);
            ctx_tmp.update(A1, A1_len);
            ctx.update(seed, seed_len);

            if (olen > chunk)
            {
                j = ctx.final(out);
                out += j;
                olen -= j;
                A1_len = ctx_tmp.final(A1); /* calc the next A1 value */
            }
            else /* last one */
            {
                A1_len = ctx.final(A1);
                memcpy(out, A1, olen);
                break;
            }
        }
        std::memset(A1, 0, sizeof(A1));
    }
#endif
};

} // namespace OpenSSLCrypto
} // namespace openvpn