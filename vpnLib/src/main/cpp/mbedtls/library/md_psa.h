/**
 * Translation between MD and PSA identifiers (algorithms, errors).
 *
 *  Note: this internal module will go away when everything becomes based on
 *  PSA Crypto; it is a helper for the transition period.
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef MBEDTLS_MD_PSA_H
#define MBEDTLS_MD_PSA_H

#include "common.h"

#include "mbedtls/md.h"
#include "psa/crypto.h"

/**
 * \brief           This function returns the PSA algorithm identifier
 *                  associated with the given digest type.
 *
 * \param md_type   The type of digest to search for.
 *
 * \return          The PSA algorithm identifier associated with \p md_type.
 * \return          PSA_ALG_NONE if the algorithm is not supported.
 */
psa_algorithm_t mbedtls_md_psa_alg_from_type(mbedtls_md_type_t md_type);

/**
 * \brief           This function returns the given digest type
 *                  associated with the PSA algorithm identifier.
 *
 * \param psa_alg   The PSA algorithm identifier to search for.
 *
 * \return          The MD type associated with \p psa_alg.
 * \return          MBEDTLS_MD_NONE if the algorithm is not supported.
 */
mbedtls_md_type_t mbedtls_md_type_from_psa_alg(psa_algorithm_t psa_alg);

/** Convert PSA status to MD error code.
 *
 * \param status    PSA status.
 *
 * \return          The corresponding MD error code,
 */
int mbedtls_md_error_from_psa(psa_status_t status);

#endif /* MBEDTLS_MD_PSA_H */
