#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/BigInteger.h>
#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/ProofMessage.h>
#include <mw/models/crypto/RewoundProof.h>
#include <mw/models/crypto/Hash.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/ScryptParameters.h>
#include <support/allocators/secure.h>

#define CRYPTO_API

//
// Exported class that serves as a lightweight, easy-to-use wrapper for secp256k1-zkp and other crypto dependencies.
//
class CRYPTO_API Crypto
{
public:
    //
    // Creates a pedersen commitment from a value with a zero blinding factor.
    //
    static Commitment CommitTransparent(const uint64_t value);

    //
    // Creates a pedersen commitment from a value with the supplied blinding factor.
    //
    static Commitment CommitBlinded(
        const uint64_t value,
        const BlindingFactor& blindingFactor
    );

    //
    // Adds the homomorphic pedersen commitments together.
    //
    static Commitment AddCommitments(
        const std::vector<Commitment>& positive,
        const std::vector<Commitment>& negative = { }
    );

    //
    // Takes a vector of blinding factors and calculates an additional blinding value that adds to zero.
    //
    static BlindingFactor AddBlindingFactors(
        const std::vector<BlindingFactor>& positive,
        const std::vector<BlindingFactor>& negative = { }
    );

    //
    //
    //
    static RangeProof::CPtr GenerateRangeProof(
        const uint64_t amount,
        const SecretKey& key,
        const SecretKey& privateNonce,
        const SecretKey& rewindNonce,
        const ProofMessage& proofMessage,
        const std::vector<uint8_t>& extraData
    );

    //
    //
    //
    static std::unique_ptr<RewoundProof> RewindRangeProof(
        const Commitment& commitment,
        const RangeProof& rangeProof,
        const std::vector<uint8_t>& extraData,
        const SecretKey& nonce
    );

    //
    //
    //
    static bool VerifyRangeProofs(
        const std::vector<std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>>& rangeProofs
    );

    //
    // Encrypts the input with AES256 using the given key.
    //
    static std::vector<uint8_t> AES256_Encrypt(
        const SecureVector& input,
        const SecretKey& key,
        const BigInt<16>& iv
    );

    //
    // Decrypts the input with AES256 using the given key.
    //
    static SecureVector AES256_Decrypt(
        const std::vector<uint8_t>& ciphertext,
        const SecretKey& key,
        const BigInt<16>& iv
    );

    //
    // Calculates the 33 byte public key from the 32 byte private key using curve secp256k1.
    //
    static PublicKey CalculatePublicKey(const SecretKey& privateKey);

    //
    // Adds a number of public keys together.
    // Returns the combined public key if successful.
    //
    static PublicKey AddPublicKeys(const std::vector<PublicKey>& publicKeys);

    //
    // Converts a commitment to a PublicKey.
    //
    static PublicKey ToPublicKey(const Commitment& commitment);

    //
    // Calculates the blinding factor x' = x + SHA256(xG+vH | xJ), used in the switch commitment x'G+vH.
    //
    static SecretKey BlindSwitch(
        const SecretKey& secretKey,
        const uint64_t amount
    );

    //
    // Adds 2 private keys together.
    //
    static SecretKey AddPrivateKeys(
        const SecretKey& secretKey1,
        const SecretKey& secretKey2
    );
};