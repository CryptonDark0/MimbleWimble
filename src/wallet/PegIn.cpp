#include "PegIn.h"

#include <mw/crypto/Blinds.h>
#include <mw/wallet/Wallet.h>
#include <mw/wallet/KernelFactory.h>
#include <mw/wallet/OutputFactory.h>

// TODO: Sort inputs, outputs, kernels, and owner_sigs
mw::Transaction::CPtr PegIn::CreatePegInTx(const uint64_t amount, const StealthAddress& receiver_addr) const
{
    // Create peg-in output.
    // We randomly generate the sender_key and output_blind.
    // Receiver key is generated by OutputFactory using the wallet's stealth address.
    SecretKey sender_key = m_wallet.NewKey();
    BlindingFactor output_blind = Random().CSPRNG<32>();
    Output output = OutputFactory::Create(
        EOutputFeatures::PEGGED_IN,
        output_blind,
        sender_key,
        receiver_addr,
        amount
    );

    // Total kernel offset is split between raw kernel_offset and the kernel's blinding factor.
    // sum(output.blind) - sum(input.blind) = kernel_offset + sum(kernel.blind)
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(output_blind)
        .Sub(kernel_offset)
        .Total();
    Kernel kernel = KernelFactory::CreatePegInKernel(kernel_blind, amount);

    // Total owner offset is split between raw owner_offset and the owner_sig's key.
    // sum(output.sender_key) - sum(input.key) = owner_offset + sum(owner_sig.key)
    BlindingFactor owner_sig_key = Random().CSPRNG<32>();
    SignedMessage owner_sig = Schnorr::SignMessage(owner_sig_key.GetBigInt(), kernel.GetHash());
    BlindingFactor owner_offset = Blinds()
        .Add(sender_key)
        .Sub(owner_sig_key)
        .Total();

    // If peg-in to user's own wallet, rewind output to retrieve and save the spendable coin.
    // This uses the same process that occurs on restore,
    // so a successful rewind means we know it can be restored from seed later.
    if (receiver_addr == m_wallet.GetStealthAddress()) {
        libmw::Coin coin = m_wallet.RewindOutput(output);
        m_wallet.GetInterface()->AddCoins({ std::move(coin) });
    }

    // Build the Transaction
    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(owner_offset),
        TxBody{
            std::vector<Input>{},
            std::vector<Output>{ std::move(output) },
            std::vector<Kernel>{ std::move(kernel) },
            std::vector<SignedMessage>{ std::move(owner_sig) }
        }
    );
}