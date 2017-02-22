/*
 * MRustC - Rust Compiler
 * - By John Hodge (Mutabah/thePowersGang)
 *
 * mir/mir.cpp
 * - MIR (Middle Intermediate Representation) definitions
 */
#include <mir/mir.hpp>

namespace MIR {
    ::std::ostream& operator<<(::std::ostream& os, const Constant& v) {
        TU_MATCHA( (v), (e),
        (Int,
            os << (e < 0 ? "-" : "+");
            os << (e < 0 ? -e : e);
            ),
        (Uint,
            os << e;
            ),
        (Float,
            os << e;
            ),
        (Bool,
            os << (e ? "true" : "false");
            ),
        (Bytes,
            os << "[";
            os << ::std::hex;
            for(auto v : e)
                os << static_cast<unsigned int>(v) << " ";
            os << ::std::dec;
            os << "]";
            ),
        (StaticString,
            os << "\"";
            for(auto v : e) {
                if( ' ' <= v && v < 0x7F && v != '"' && v != '\\' )
                    os << v;
                else
                    os << "\\u{" << FMT(::std::hex << (unsigned int)v) << "}";
            }
            os << "\"";
            ),
        (Const,
            os << e.p;
            ),
        (ItemAddr,
            os << "&" << e;
            )
        )
        return os;
    }
    bool Constant::operator==(const Constant& b) const
    {
        if( this->tag() != b.tag() )
            return false;
        TU_MATCHA( (*this,b), (ae,be),
        (Int,
            return ae == be;
            ),
        (Uint,
            return ae == be;
            ),
        (Float,
            return ae == be;
            ),
        (Bool,
            return ae == be;
            ),
        (Bytes,
            return ae == be;
            ),
        (StaticString,
            return ae == be;
            ),
        (Const,
            return ae.p == be.p;
            ),
        (ItemAddr,
            return ae == be;
            )
        )
        throw "";
    }

    ::std::ostream& operator<<(::std::ostream& os, const LValue& x)
    {
        TU_MATCHA( (x), (e),
        (Variable,
            os << "Variable(" << e << ")";
            ),
        (Temporary,
            os << "Temporary(" << e.idx << ")";
            ),
        (Argument,
            os << "Argument(" << e.idx << ")";
            ),
        (Static,
            os << "Static(" << e << ")";
            ),
        (Return,
            os << "Return";
            ),
        (Field,
            os << "Field(" << e.field_index << ", " << *e.val << ")";
            ),
        (Deref,
            os << "Deref(" << *e.val << ")";
            ),
        (Index,
            os << "Index(" << *e.val << ", " << *e.idx << ")";
            ),
        (Downcast,
            os << "Downcast(" << e.variant_index << ", " << *e.val << ")";
            )
        )
        return os;
    }
    bool operator<(const LValue& a, const LValue& b)
    {
        if( a.tag() != b.tag() )
            return a.tag() < b.tag();
        TU_MATCHA( (a, b), (ea, eb),
        (Variable,
            return ea < eb;
            ),
        (Temporary,
            return ea.idx < eb.idx;
            ),
        (Argument,
            return ea.idx < eb.idx;
            ),
        (Static,
            return ea < eb;
            ),
        (Return,
            return false;
            ),
        (Field,
            if( *ea.val != *eb.val )
                return *ea.val < *eb.val;
            if( ea.field_index != eb.field_index )
                return ea.field_index < eb.field_index;
            return true;
            ),
        (Deref,
            return *ea.val < *eb.val;
            ),
        (Index,
            if( *ea.val != *eb.val )
                return *ea.val < *eb.val;
            return *ea.idx < *eb.idx;
            ),
        (Downcast,
            if( *ea.val != *eb.val )
                return *ea.val < *eb.val;
            return ea.variant_index < eb.variant_index;
            )
        )
        throw "";
    }
    bool operator==(const LValue& a, const LValue& b)
    {
        if( a.tag() != b.tag() )
            return false;
        TU_MATCHA( (a, b), (ea, eb),
        (Variable,
            return ea == eb;
            ),
        (Temporary,
            return ea.idx == eb.idx;
            ),
        (Argument,
            return ea.idx == eb.idx;
            ),
        (Static,
            return ea == eb;
            ),
        (Return,
            return true;
            ),
        (Field,
            if( *ea.val != *eb.val )
                return false;
            if( ea.field_index != eb.field_index )
                return false;
            return true;
            ),
        (Deref,
            return *ea.val == *eb.val;
            ),
        (Index,
            if( *ea.val != *eb.val )
                return false;
            if( *ea.idx != *eb.idx )
                return false;
            return true;
            ),
        (Downcast,
            if( *ea.val != *eb.val )
                return false;
            if( ea.variant_index != eb.variant_index )
                return false;
            return true;
            )
        )
        throw "";
    }

    ::std::ostream& operator<<(::std::ostream& os, const Param& x)
    {
        TU_MATCHA( (x), (e),
        (LValue,
            os << e;
            ),
        (Constant,
            os << e;
            )
        )
        return os;
    }
    bool Param::operator==(const Param& x) const
    {
        if( this->tag() != x.tag() )
            return false;
        TU_MATCHA( (*this, x), (ea, eb),
        (LValue,
            return ea == eb;
            ),
        (Constant,
            return ea == eb;
            )
        )
        throw "";
    }

    ::std::ostream& operator<<(::std::ostream& os, const RValue& x)
    {
        TU_MATCHA( (x), (e),
        (Use,
            os << "Use(" << e << ")";
            ),
        (Constant,
            os << "Constant(" << e << ")";
            ),
        (SizedArray,
            os << "SizedArray(" << e.val << "; " << e.count << ")";
            ),
        (Borrow,
            os << "Borrow(" << e.region << ", " << e.type << ", " << e.val << ")";
            ),
        (Cast,
            os << "Cast(" << e.val << " as " << e.type << ")";
            ),
        (BinOp,
            os << "BinOp(" << e.val_l << " ";
            switch(e.op)
            {
            case ::MIR::eBinOp::ADD:    os << "ADD";    break;
            case ::MIR::eBinOp::SUB:    os << "SUB";    break;
            case ::MIR::eBinOp::MUL:    os << "MUL";    break;
            case ::MIR::eBinOp::DIV:    os << "DIV";    break;
            case ::MIR::eBinOp::MOD:    os << "MOD";    break;
            case ::MIR::eBinOp::ADD_OV: os << "ADD_OV"; break;
            case ::MIR::eBinOp::SUB_OV: os << "SUB_OV"; break;
            case ::MIR::eBinOp::MUL_OV: os << "MUL_OV"; break;
            case ::MIR::eBinOp::DIV_OV: os << "DIV_OV"; break;

            case ::MIR::eBinOp::BIT_OR : os << "BIT_OR" ; break;
            case ::MIR::eBinOp::BIT_AND: os << "BIT_AND"; break;
            case ::MIR::eBinOp::BIT_XOR: os << "BIT_XOR"; break;
            case ::MIR::eBinOp::BIT_SHL: os << "BIT_SHL"; break;
            case ::MIR::eBinOp::BIT_SHR: os << "BIT_SHR"; break;

            case ::MIR::eBinOp::EQ: os << "EQ"; break;
            case ::MIR::eBinOp::NE: os << "NE"; break;
            case ::MIR::eBinOp::GT: os << "GT"; break;
            case ::MIR::eBinOp::GE: os << "GE"; break;
            case ::MIR::eBinOp::LT: os << "LT"; break;
            case ::MIR::eBinOp::LE: os << "LE"; break;
            }
            os << " " << e.val_r << ")";
            ),
        (UniOp,
            os << "UniOp(" << e.val << " " << static_cast<int>(e.op) << ")";
            ),
        (DstMeta,
            os << "DstMeta(" << e.val << ")";
            ),
        (DstPtr,
            os << "DstPtr(" << e.val << ")";
            ),
        (MakeDst,
            os << "MakeDst(" << e.ptr_val << ", " << e.meta_val << ")";
            ),
        (Tuple,
            os << "Tuple(" << e.vals << ")";
            ),
        (Array,
            os << "Array(" << e.vals << ")";
            ),
        (Variant,
            os << "Variant(" << e.path << " #" << e.index << ", " << e.val << ")";
            ),
        (Struct,
            os << "Struct(" << e.path << ", {" << e.vals << "})";
            )
        )
        return os;
    }
    bool operator==(const RValue& a, const RValue& b)
    {
        if( a.tag() != b.tag() )
            return false;
        TU_MATCHA( (a, b), (are, bre),
        (Use,
            return are == bre;
            ),
        (Constant,
            return are == bre;
            ),
        (SizedArray,
            if( are.val != bre.val )
                return false;
            if( are.count != bre.count )
                return false;
            return true;
            ),
        (Borrow,
            if( are.region != bre.region )
                return false;
            if( are.type != bre.type )
                return false;
            if( are.val != bre.val )
                return false;
            return true;
            ),
        (Cast,
            if( are.type != bre.type )
                return false;
            if( are.val != bre.val )
                return false;
            return true;
            ),
        (BinOp,
            if( are.val_l != bre.val_l )
                return false;
            if( are.op != bre.op )
                return false;
            if( are.val_r != bre.val_r )
                return false;
            return true;
            ),
        (UniOp,
            if( are.op != bre.op )
                return false;
            if( are.val != bre.val )
                return false;
            return true;
            ),
        (DstPtr,
            return are.val == bre.val;
            ),
        (DstMeta,
            return are.val == bre.val;
            ),
        (MakeDst,
            if( are.meta_val != bre.meta_val )
                return false;
            if( are.ptr_val != bre.ptr_val )
                return false;
            return true;
            ),
        (Tuple,
            return are.vals == bre.vals;
            ),
        (Array,
            return are.vals == bre.vals;
            ),
        (Variant,
            if( are.path != bre.path )
                return false;
            if( are.index != bre.index )
                return false;
            return are.val == bre.val;
            ),
        (Struct,
            if( are.path != bre.path )
                return false;
            if( are.variant_idx != bre.variant_idx )
                return false;
            return are.vals == bre.vals;
            )
        )
        throw "";
    }

    ::std::ostream& operator<<(::std::ostream& os, const Terminator& x)
    {
        TU_MATCHA( (x), (e),
        (Incomplete,
            os << "Invalid";
            ),
        (Return,
            os << "Return";
            ),
        (Diverge,
            os << "Diverge";
            ),
        (Goto,
            os << "Goto(" << e << ")";
            ),
        (Panic,
            os << "Panic(" << e.dst << ";)";
            ),
        (If,
            os << "If( " << e.cond << " : " << e.bb0 << ", " << e.bb1 << ")";
            ),
        (Switch,
            os << "Switch( " << e.val << " : ";
            for(unsigned int j = 0; j < e.targets.size(); j ++)
                os << j << " => bb" << e.targets[j] << ", ";
            os << ")";
            ),
        (Call,
            os << "Call( " << e.ret_val << " = ";
            TU_MATCHA( (e.fcn), (e2),
            (Value,
                os << "(" << e2 << ")";
                ),
            (Path,
                os << e2;
                ),
            (Intrinsic,
                os << "\"" << e2.name << "\"::" << e2.params;
                )
            )
            os << "( ";
            for(const auto& arg : e.args)
                os << arg << ", ";
            os << "), bb" << e.ret_block << ", bb" << e.panic_block << ")";
            )
        )

        return os;
    }
}

::MIR::LValue MIR::LValue::clone() const
{
    TU_MATCHA( (*this), (e),
    (Variable, return LValue(e); ),
    (Temporary, return LValue(e); ),
    (Argument, return LValue(e); ),
    (Static, return LValue(e.clone()); ),
    (Return, return LValue(e); ),
    (Field, return LValue::make_Field({
        box$( e.val->clone() ),
        e.field_index
        }); ),
    (Deref, return LValue::make_Deref({
        box$( e.val->clone() )
        }); ),
    (Index, return LValue::make_Index({
        box$( e.val->clone() ),
        box$( e.idx->clone() )
        }); ),
    (Downcast, return LValue::make_Downcast({
        box$( e.val->clone() ),
        e.variant_index
        }); )
    )
    throw "";
}
::MIR::Constant MIR::Constant::clone() const
{
    TU_MATCHA( (*this), (e2),
    (Int, return ::MIR::Constant(e2); ),
    (Uint, return ::MIR::Constant(e2); ),
    (Float, return ::MIR::Constant(e2); ),
    (Bool, return ::MIR::Constant(e2); ),
    (Bytes, return ::MIR::Constant(e2); ),
    (StaticString, return ::MIR::Constant(e2); ),
    (Const, return ::MIR::Constant::make_Const({e2.p.clone()}); ),
    (ItemAddr, return ::MIR::Constant(e2.clone()); )
    )
    throw "";
}

::MIR::Param MIR::Param::clone() const
{
    TU_MATCHA( (*this), (e),
    (LValue,
        return e.clone();
        ),
    (Constant,
        return e.clone();
        )
    )
    throw "";
}

::MIR::RValue MIR::RValue::clone() const
{
    TU_MATCHA( (*this), (e),
    (Use,
        return ::MIR::RValue(e.clone());
        ),
    (Constant,
        return e.clone();
        ),
    (SizedArray,
        return ::MIR::RValue::make_SizedArray({ e.val.clone(), e.count });
        ),
    (Borrow,
        return ::MIR::RValue::make_Borrow({ e.region, e.type, e.val.clone() });
        ),
    (Cast,
        return ::MIR::RValue::make_Cast({ e.val.clone(), e.type.clone() });
        ),
    (BinOp,
        return ::MIR::RValue::make_BinOp({ e.val_l.clone(), e.op, e.val_r.clone() });
        ),
    (UniOp,
        return ::MIR::RValue::make_UniOp({ e.val.clone(), e.op });
        ),
    (DstMeta,
        return ::MIR::RValue::make_DstMeta({ e.val.clone() });
        ),
    (DstPtr,
        return ::MIR::RValue::make_DstPtr({ e.val.clone() });
        ),
    // Construct a DST pointer from a thin pointer and metadata
    (MakeDst,
        return ::MIR::RValue::make_MakeDst({ e.ptr_val.clone(), e.meta_val.clone() });
        ),
    (Tuple,
        decltype(e.vals)    ret;
        ret.reserve(e.vals.size());
        for(const auto& v : e.vals)
            ret.push_back( v.clone() );
        return ::MIR::RValue::make_Tuple({ mv$(ret) });
        ),
    // Array literal
    (Array,
        decltype(e.vals)    ret;
        ret.reserve(e.vals.size());
        for(const auto& v : e.vals)
            ret.push_back( v.clone() );
        return ::MIR::RValue::make_Array({ mv$(ret) });
        ),
    // Create a new instance of a union (and eventually enum)
    (Variant,
        return ::MIR::RValue::make_Variant({ e.path.clone(), e.index, e.val.clone() });
        ),
    // Create a new instance of a struct (or enum)
    (Struct,
        decltype(e.vals)    ret;
        ret.reserve(e.vals.size());
        for(const auto& v : e.vals)
            ret.push_back( v.clone() );
        return ::MIR::RValue::make_Struct({ e.path.clone(), e.variant_idx, mv$(ret) });
        )
    )
    throw "";
}

