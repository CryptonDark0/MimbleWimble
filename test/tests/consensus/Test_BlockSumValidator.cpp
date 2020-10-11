#include <catch.hpp>

#include <mw/Consensus/BlockSumValidator.h>
#include <mw/consensus/Aggregation.h>
#include <mw/crypto/Random.h>

#include <test_framework/models/Tx.h>
#include <test_framework/TxBuilder.h>

// TODO: Add tests for BlockSumValidator::ValidateState
// FUTURE: Create official test vectors for the consensus rules being tested

TEST_CASE("BlockSumValidator::ValidateForBlock")
{
    // Standard transaction - 2 inputs, 2 outputs, 1 kernel
    mw::Transaction::CPtr tx1 = test::TxBuilder()
        .AddInput(5'000'000).AddInput(6'000'000)
        .AddOutput(4'000'000).AddOutput(6'500'000)
        .AddPlainKernel(500'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx1); // Sanity check

    // Pegin transaction - 1 output, 1 kernel
    mw::Transaction::CPtr tx2 = test::TxBuilder()
        .AddOutput(8'000'000, EOutputFeatures::PEGGED_IN)
        .AddPeginKernel(8'000'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx2); // Sanity check

    mw::Transaction::CPtr tx3 = test::TxBuilder()
        .AddInput(1'234'567).AddInput(4'000'000)
        .AddOutput(234'567)
        .AddPegoutKernel(4'500'000, 500'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx3); // Sanity check

    BlindingFactor prev_total_offset = Random::CSPRNG<32>();
    mw::Transaction::CPtr pAggregated = Aggregation::Aggregate({ tx1, tx2, tx3 });
    BlockSumValidator::ValidateForTx(*pAggregated); // Sanity check

    BlindingFactor total_offset = Crypto::AddBlindingFactors({ prev_total_offset, pAggregated->GetOffset() });

    BlockSumValidator::ValidateForBlock(pAggregated->GetBody(), total_offset, prev_total_offset);
}

//
// This tests ValidateForBlock without using the TxBuilder utility, since in theory it could contain bugs.
// In the future, it would be even better to replace this with official test vectors, and avoid relying on Random entirely.
//
TEST_CASE("BlockSumValidator::ValidateForBlock - Without Builder")
{
    mw::Hash prev_total_offset = mw::Hash::FromHex("0123456789abcdef0123456789abcdef00000000000000000000000000000000");

    test::Tx::Builder tx_builder;

    // Add inputs
    BlindingFactor input1_bf = Random::CSPRNG<32>();
    tx_builder.AddInput(Input(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(5'000'000, input1_bf)));

    BlindingFactor input2_bf = Random::CSPRNG<32>();
    tx_builder.AddInput(Input(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(6'000'000, input2_bf)));

    // Add outputs
    BlindingFactor output1_bf = Random::CSPRNG<32>();
    tx_builder.AddOutput(test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, output1_bf, 4'000'000));

    BlindingFactor output2_bf = Random::CSPRNG<32>();
    tx_builder.AddOutput(test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, output2_bf, 6'500'000));

    // Add kernels
    BlindingFactor excess = Crypto::AddBlindingFactors({ output1_bf, output2_bf }, { input1_bf, input2_bf });
    BlindingFactor tx_offset = Random::CSPRNG<32>();
    BigInt<32> excess_minus_offset = Crypto::AddBlindingFactors({ excess }, { tx_offset }).GetBigInt();
    tx_builder.AddPlainKernel(500'000, SecretKey{ std::move(excess_minus_offset) });

    // Set Offset
    tx_builder.SetOffset(tx_offset);

    mw::Transaction::CPtr pTransaction = tx_builder.Build().GetTransaction();

    BlindingFactor total_offset = Crypto::AddBlindingFactors({ prev_total_offset, tx_offset });
    BlockSumValidator::ValidateForBlock(pTransaction->GetBody(), total_offset, prev_total_offset);
}

TEST_CASE("BlockSumValidator::ValidateForTx")
{
    // Standard transaction - 2 inputs, 2 outputs, 1 kernel
    mw::Transaction::CPtr tx1 = test::TxBuilder()
        .AddInput(5'000'000).AddInput(6'000'000)
        .AddOutput(4'000'000).AddOutput(6'500'000)
        .AddPlainKernel(500'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx1);

    // Pegin transaction - 1 output, 1 kernel
    mw::Transaction::CPtr tx2 = test::TxBuilder()
        .AddOutput(8'000'000, EOutputFeatures::PEGGED_IN)
        .AddPeginKernel(8'000'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx2);

    // Pegout transaction - 2 inputs, 1 output, 1 kernel
    mw::Transaction::CPtr tx3 = test::TxBuilder()
        .AddInput(1'234'567).AddInput(4'000'000)
        .AddOutput(234'567)
        .AddPegoutKernel(4'500'000, 500'000)
        .Build();
    BlockSumValidator::ValidateForTx(*tx3);

    // Aggregate all 3
    mw::Transaction::CPtr pAggregated = Aggregation::Aggregate({ tx1, tx2, tx3 });
    BlockSumValidator::ValidateForTx(*pAggregated);
}