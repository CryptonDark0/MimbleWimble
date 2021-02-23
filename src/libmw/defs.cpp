#include <libmw/defs.h>

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/node/INode.h>
#include <mw/consensus/Weight.h>

LIBMW_NAMESPACE

MWEXPORT libmw::BlockHash BlockRef::GetHash() const noexcept
{
    assert(pBlock != nullptr);
    return pBlock->GetHash().ToArray();
}

MWEXPORT libmw::HeaderRef BlockRef::GetHeader() const
{
    if (pBlock == nullptr) {
        return libmw::HeaderRef{ nullptr };
    }

    return libmw::HeaderRef{ pBlock->GetHeader() };
}

MWEXPORT uint64_t BlockRef::GetTotalFee() const noexcept
{
    assert(pBlock != nullptr);
    return pBlock->GetTotalFee();
}

MWEXPORT uint64_t BlockRef::GetWeight() const noexcept
{
    assert(pBlock != nullptr);
    return Weight::Calculate(pBlock->GetTxBody());
}

MWEXPORT std::set<KernelHash> BlockRef::GetKernelHashes() const
{
    assert(pBlock != nullptr);
    std::set<KernelHash> kernelHashes;
    for (const Kernel& kernel : pBlock->GetKernels()) {
        kernelHashes.insert(kernel.GetHash().ToArray());
    }
    return kernelHashes;
}

MWEXPORT std::vector<libmw::Commitment> BlockRef::GetInputCommits() const
{
    assert(pBlock != nullptr);
    std::vector<libmw::Commitment> input_commits;
    for (const Input& input : pBlock->GetInputs()) {
        input_commits.push_back(input.GetCommitment().array());
    }
    return input_commits;
}

MWEXPORT std::vector<libmw::Commitment> BlockRef::GetOutputCommits() const
{
    assert(pBlock != nullptr);
    std::vector<libmw::Commitment> output_commits;
    for (const Output& output : pBlock->GetOutputs()) {
        output_commits.push_back(output.GetCommitment().array());
    }
    return output_commits;
}

MWEXPORT std::vector<PegOut> TxRef::GetPegouts() const noexcept
{
    std::vector<PegOut> pegouts;
    for (const Kernel& kernel : pTransaction->GetKernels()) {
        if (kernel.IsPegOut()) {
            PegOut pegout;
            pegout.amount = kernel.GetPeggedOut();
            pegout.address = kernel.GetAddress().value().ToString();
            pegouts.emplace_back(std::move(pegout));
        }
    }

    return pegouts;
}

MWEXPORT std::vector<PegIn> TxRef::GetPegins() const noexcept
{
    std::vector<PegIn> pegins;
    for (const Kernel& kernel : pTransaction->GetKernels()) {
        if (kernel.IsPegIn()) {
            PegIn pegin;
            pegin.amount = kernel.GetPeggedIn();
            pegin.commitment = kernel.GetCommitment().array();
            pegins.emplace_back(std::move(pegin));
        }
    }

    return pegins;
}

MWEXPORT uint64_t TxRef::GetTotalFee() const noexcept
{
    assert(pTransaction != nullptr);
    return pTransaction->GetTotalFee();
}

MWEXPORT uint64_t TxRef::GetWeight() const noexcept
{
    assert(pTransaction != nullptr);
    return Weight::Calculate(pTransaction->GetBody());
}

MWEXPORT std::set<KernelHash> TxRef::GetKernelHashes() const noexcept
{
    assert(pTransaction != nullptr);
    std::set<KernelHash> kernelHashes;
    for (const Kernel& kernel : pTransaction->GetKernels()) {
        kernelHashes.insert(kernel.GetHash().ToArray());
    }

    return kernelHashes;
}

MWEXPORT std::set<libmw::Commitment> TxRef::GetInputCommits() const noexcept
{
    assert(pTransaction != nullptr);
    std::set<libmw::Commitment> input_commits;
    for (const Input& input : pTransaction->GetInputs()) {
        input_commits.insert(input.GetCommitment().array());
    }

    return input_commits;
}

MWEXPORT std::set<libmw::Commitment> TxRef::GetOutputCommits() const noexcept
{
    assert(pTransaction != nullptr);
    std::set<libmw::Commitment> output_commits;
    for (const Output& output : pTransaction->GetOutputs()) {
        output_commits.insert(output.GetCommitment().array());
    }

    return output_commits;
}

MWEXPORT libmw::CoinsViewRef CoinsViewRef::CreateCache() const
{
    if (pCoinsView == nullptr) {
        return libmw::CoinsViewRef{ nullptr };
    }

    return libmw::CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pCoinsView) };
}

END_NAMESPACE