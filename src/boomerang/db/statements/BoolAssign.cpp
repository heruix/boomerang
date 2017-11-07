#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BoolAssign.h"


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/StatementHelper.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"

BoolAssign::BoolAssign(int size)
    : Assignment(nullptr)
    , m_jumpType((BranchType)0)
    , m_cond(nullptr)
    , m_isFloat(false)
    , m_size(size)
{
    m_kind = STMT_BOOLASSIGN;
}


BoolAssign::~BoolAssign()
{
}


void BoolAssign::setCondType(BranchType cond, bool usesFloat /*= false*/)
{
    m_jumpType = cond;
    m_isFloat  = usesFloat;
    setCondExpr(Terminal::get(opFlags));
}


void BoolAssign::makeSigned()
{
    // Make this into a signed branch
    switch (m_jumpType)
    {
    case BRANCH_JUL:
        m_jumpType = BRANCH_JSL;
        break;

    case BRANCH_JULE:
        m_jumpType = BRANCH_JSLE;
        break;

    case BRANCH_JUGE:
        m_jumpType = BRANCH_JSGE;
        break;

    case BRANCH_JUG:
        m_jumpType = BRANCH_JSG;
        break;

    default:
        // Do nothing for other cases
        break;
    }
}


SharedExp BoolAssign::getCondExpr() const
{
    return m_cond;
}


void BoolAssign::setCondExpr(SharedExp pss)
{
    if (m_cond) {
        // delete pCond;
    }

    m_cond = pss;
}


void BoolAssign::printCompact(QTextStream& os, bool html) const
{
    os << "BOOL ";
    m_lhs->print(os);
    os << " := CC(";

    switch (m_jumpType)
    {
    case BRANCH_JE:
        os << "equals";
        break;

    case BRANCH_JNE:
        os << "not equals";
        break;

    case BRANCH_JSL:
        os << "signed less";
        break;

    case BRANCH_JSLE:
        os << "signed less or equals";
        break;

    case BRANCH_JSGE:
        os << "signed greater or equals";
        break;

    case BRANCH_JSG:
        os << "signed greater";
        break;

    case BRANCH_JUL:
        os << "unsigned less";
        break;

    case BRANCH_JULE:
        os << "unsigned less or equals";
        break;

    case BRANCH_JUGE:
        os << "unsigned greater or equals";
        break;

    case BRANCH_JUG:
        os << "unsigned greater";
        break;

    case BRANCH_JMI:
        os << "minus";
        break;

    case BRANCH_JPOS:
        os << "plus";
        break;

    case BRANCH_JOF:
        os << "overflow";
        break;

    case BRANCH_JNOF:
        os << "no overflow";
        break;

    case BRANCH_JPAR:
        os << "ev parity";
        break;
    }

    os << ")";

    if (m_isFloat) {
        os << ", float";
    }

    if (html) {
        os << "<br>";
    }

    os << '\n';

    if (m_cond) {
        os << "High level: ";
        m_cond->print(os, html);

        if (html) {
            os << "<br>";
        }

        os << "\n";
    }
}


Statement *BoolAssign::clone() const
{
    BoolAssign *ret = new BoolAssign(m_size);

    ret->m_jumpType = m_jumpType;
    ret->m_cond     = (m_cond) ? m_cond->clone() : nullptr;
    ret->m_isFloat  = m_isFloat;
    ret->m_size     = m_size;
    // Statement members
    ret->m_parent = m_parent;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool BoolAssign::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void BoolAssign::generateCode(ICodeGenerator *gen, const BasicBlock *)
{
    assert(m_lhs);
    assert(m_cond);
    // lhs := (pCond) ? 1 : 0
    Assign as(m_lhs->clone(), std::make_shared<Ternary>(opTern, m_cond->clone(), Const::get(1), Const::get(0)));
    gen->addAssignmentStatement(&as);
}


void BoolAssign::simplify()
{
    if (m_cond) {
        condToRelational(m_cond, m_jumpType);
    }
}


void BoolAssign::getDefinitions(LocationSet& defs) const
{
    defs.insert(getLeft());
}


bool BoolAssign::usesExp(const Exp& e) const
{
    assert(m_lhs && m_cond);
    SharedExp where = nullptr;
    return(m_cond->search(e, where) || (m_lhs->isMemOf() && m_lhs->getSubExp1()->search(e, where)));
}


bool BoolAssign::search(const Exp& pattern, SharedExp& result) const
{
    assert(m_lhs);

    if (m_lhs->search(pattern, result)) {
        return true;
    }

    assert(m_cond);
    return m_cond->search(pattern, result);
}


bool BoolAssign::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    bool ch = false;

    assert(m_lhs);

    if (m_lhs->searchAll(pattern, result)) {
        ch = true;
    }

    assert(m_cond);
    return m_cond->searchAll(pattern, result) || ch;
}


bool BoolAssign::searchAndReplace(const Exp& pattern, SharedExp replace, bool cc)
{
    Q_UNUSED(cc);
    bool chl, chr;
    assert(m_cond);
    assert(m_lhs);
    m_cond = m_cond->searchReplaceAll(pattern, replace, chl);
    m_lhs  = m_lhs->searchReplaceAll(pattern, replace, chr);
    return chl | chr;
}


void BoolAssign::setLeftFromList(std::list<Statement *> *stmts)
{
    assert(stmts->size() == 1);
    Assign *first = (Assign *)stmts->front();
    assert(first->getKind() == STMT_ASSIGN);
    m_lhs = first->getLeft();
}


bool BoolAssign::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_cond) {
        ret = m_cond->accept(v->ev);
    }

    return ret;
}


bool BoolAssign::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (m_cond && visitChildren) {
        m_cond = m_cond->accept(v->m_mod);
    }

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->m_mod));
    }

    return true;
}


bool BoolAssign::accept(StmtPartModifier *v)
{
    bool visitChildren;

    v->visit(this, visitChildren);

    if (m_cond && visitChildren) {
        m_cond = m_cond->accept(v->mod);
    }

    if (m_lhs && visitChildren) {
        m_lhs = m_lhs->accept(v->mod);
    }

    return true;
}


void BoolAssign::dfaTypeAnalysis(bool& ch)
{
    // Not properly implemented yet
    Assignment::dfaTypeAnalysis(ch);
}
