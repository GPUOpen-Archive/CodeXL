/// -----------------------------------------------------------------------------------------------
/// \class Name:
/// \brief Description:  Branch Unit Scheduler Class.
/// -----------------------------------------------------------------------------------------------

#include <vector>
#include "../Parser/Instruction.h"
#include "../Parser/SOPPInstruction.h"
#include "../Parser/SOP1Instruction.h"
#include "../Parser/SOP2Instruction.h"
#include "../Parser/SOPCInstruction.h"
#include "../Parser/SOPKInstruction.h"
/// -----------------------------------------------------------------------------------------------
/// \class Name:
/// \brief Description:  Branch Unit Scheduler Class
/// -----------------------------------------------------------------------------------------------
class BranchUnitScheduler
{
public:
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        BranchUnitScheduler
    /// \brief Description: c`tor
    /// \param[in]          branchTakenRate
    /// \return
    /// -----------------------------------------------------------------------------------------------
    explicit BranchUnitScheduler(double branchTakenRate): m_branchInstNum(0), m_branchTakenRate(branchTakenRate) {}

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~BranchUnitScheduler
    /// \brief Description: d`tor
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~BranchUnitScheduler() {}

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetBranchInstructionsNum
    /// \brief Description: Get branch instruction number.
    ///                     The instructions ,which are classified as branches are:
    ///                     1)S_BRANCH – Unconditional branch
    ///                     2)S_CBRANCH_(test) - Conditional branch. Branch only if (test) is true.
    ///                     3)S_SETPC – Directly set the PC from an SGPR pair.
    ///                     4)S_SWAPPC – Swap the current PC with an address in an SGPR pair.
    ///                     5)S_GETPC – Retrieve the current PC value (does not cause a branch).
    ///                     6)S_CBRANCH_FORK and S_CBRANCH_JOIN – Conditional branch for complex.
    ///                     7)S_SETVSKIP – Set a bit that causes all vector instructions to be ignored.
    /// \param[in]          instructions
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetBranchInstructionsNum(const std::vector<Instruction*>* instructions);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsInstructionBranch
    /// \brief Description: Checks if instruction is branch instruction
    /// \param[in]          inst
    /// \return True :      If yes.
    /// \return False:      If no.
    /// -----------------------------------------------------------------------------------------------
    bool IsInstructionBranch(const Instruction* inst) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsBranchTaken
    /// \brief Description: Checks if branch instruction is taken.
    /// \param[in]          branchInstIdx
    /// \return True :      If yes.
    /// \return False:      If no.
    /// -----------------------------------------------------------------------------------------------
    bool IsBranchTaken(size_t branchInstIdx)const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        SetUpBranchUnitScheduler
    /// \brief Description: Setup Branch Unit Scheduler - initializes m_branchPredictor.
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void SetUpBranchUnitScheduler();


private:
    /// Branch Instruction Number.
    size_t m_branchInstNum;

    /// Bool map ,which saves the branch prediction information foe every branch instruction./
    std::vector<bool> m_branchPredictor;

    /// Branch Prediction Rate.(Should be < 1)
    double m_branchTakenRate;
};

