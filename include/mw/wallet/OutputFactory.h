#pragma once

#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Schnorr.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/tx/Output.h>
#include <mw/models/tx/OwnerData.h>
#include <mw/models/wallet/StealthAddress.h>

class OutputFactory
{
public:
    static Output Create(
        const EOutputFeatures features,
        const BlindingFactor& blinding_factor,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const SecretKey& rewind_nonce, // TODO: Is this still needed?
        const ProofMessage& proofMessage, // TODO: Is this still needed?
        const uint64_t amount)
    {
        Commitment commitment = Crypto::CommitBlinded(
            amount,
            blinding_factor
        );

        OwnerData owner_data = CreateOwnerData(sender_privkey, receiver_addr, blinding_factor, amount);

        RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
            amount,
            SecretKey(blinding_factor.vec()),
            rewind_nonce,
            rewind_nonce,
            proofMessage,
            owner_data.Serialized()
        );

        return Output{ features, std::move(commitment), std::move(owner_data), pRangeProof };
    }

    static OwnerData CreateOwnerData(
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const BlindingFactor& blinding_factor,
        const uint64_t amount)
    {
        PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
        SecretKey r = Random::CSPRNG<32>();
        PublicKey R = Keys::From(r).PubKey();
        PublicKey rA = Keys::From(receiver_addr.A()).Mul(r).PubKey();
        PublicKey receiver_pubkey = Keys::From(Hashed(rA)).Add(receiver_addr.B()).PubKey();

        std::vector<uint8_t> plaintext = Serializer()
            .Append(blinding_factor)
            .Append<uint64_t>(amount)
            .vec();
        SecretKey shared_secret = Hashed(Keys::From(receiver_addr.B()).Mul(sender_privkey).PubKey());
        std::vector<uint8_t> encrypted_data = Crypto::AES256_Encrypt(plaintext, shared_secret, BigInt<16>());

        auto serialized_msg = Serializer()
            .Append(receiver_pubkey)
            .Append(R)
            .Append<uint8_t>((uint8_t)encrypted_data.size())
            .Append(encrypted_data)
            .vec();
        Signature signature = Schnorr::Sign(sender_privkey.data(), Hashed(serialized_msg));

        return OwnerData(
            std::move(sender_pubkey),
            std::move(receiver_pubkey),
            std::move(R),
            std::move(encrypted_data),
            std::move(signature)
        );
    }
};