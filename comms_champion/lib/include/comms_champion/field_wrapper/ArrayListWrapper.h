//
// Copyright 2015 - 2019 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once


#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <type_traits>
#include <functional>
#include <utility>

#include "comms/comms.h"

#include "FieldWrapper.h"

namespace comms_champion
{

namespace field_wrapper
{

class CC_API ArrayListWrapper : public FieldWrapper
{
public:

    typedef std::vector<FieldWrapperPtr> Members;
    typedef std::unique_ptr<ArrayListWrapper> Ptr;

    ArrayListWrapper();
    ArrayListWrapper(const ArrayListWrapper&) = delete;
    ArrayListWrapper& operator=(const ArrayListWrapper&) = delete;

    virtual ~ArrayListWrapper() noexcept;

    void addField();

    void removeField(int idx);

    unsigned size() const;

    bool hasFixedSize() const;

    void adjustFixedSize();

    Members& getMembers();

    const Members& getMembers() const;

    void setMembers(Members&& members);

    Ptr clone();

    void refreshMembers();

    using PrefixFieldInfo = std::pair<int, SerialisedSeq>;

    PrefixFieldInfo getPrefixFieldInfo() const;

protected:
    virtual void addFieldImpl() = 0;
    virtual void removeFieldImpl(int idx) = 0;
    virtual unsigned sizeImpl() const = 0;
    virtual bool hasFixedSizeImpl() const = 0;
    virtual void adjustFixedSizeImpl() = 0;
    virtual Ptr cloneImpl() = 0;
    virtual void refreshMembersImpl() = 0;
    virtual PrefixFieldInfo getPrefixFieldInfoImpl() const = 0;

    void dispatchImpl(FieldWrapperHandler& handler);

private:
    Members m_members;
};

template <typename TField>
class ArrayListWrapperT : public FieldWrapperT<ArrayListWrapper, TField>
{
    using Base = FieldWrapperT<ArrayListWrapper, TField>;
    using Field = TField;
    using ValueType = typename Field::ValueType;
    using ElementType = typename ValueType::value_type;

public:
    using SerialisedSeq = typename Base::SerialisedSeq;
    using Ptr = typename Base::Ptr;
    using PrefixFieldInfo = typename Base::PrefixFieldInfo;

    typedef std::function<FieldWrapperPtr (ElementType&)> WrapFieldCallbackFunc;

    explicit ArrayListWrapperT(Field& fieldRef)
      : Base(fieldRef)
    {
    }

    ArrayListWrapperT(const ArrayListWrapperT&) = default;
    ArrayListWrapperT(ArrayListWrapperT&&) = default;
    virtual ~ArrayListWrapperT() noexcept = default;

    ArrayListWrapperT& operator=(const ArrayListWrapperT&) = delete;

    void setWrapFieldCallback(WrapFieldCallbackFunc&& func)
    {
        assert(func);
        m_wrapFieldFunc = std::move(func);
    }

protected:

    virtual void addFieldImpl() override
    {
        auto& col = Base::field().value();

        auto& mems = Base::getMembers();

        decltype(&col[0]) firstElemPtr = nullptr;
        if (!col.empty()) {
            firstElemPtr = &col[0];
        }

        col.push_back(ElementType());
        if (!m_wrapFieldFunc) {
            assert(!"The callback is expected to be set");
            mems.clear();
            return;
        }

        if (firstElemPtr == &col[0]) {
            mems.push_back(m_wrapFieldFunc(col.back()));
            assert(col.size() == mems.size());
            return;
        }

        mems.clear();
        mems.reserve(col.size());
        for (auto& f : col) {
            mems.push_back(m_wrapFieldFunc(f));
        }

        assert(col.size() == mems.size());
    }

    virtual void removeFieldImpl(int idx) override
    {
        auto& storage = Base::field().value();
        if (static_cast<decltype(idx)>(storage.size()) <= idx) {
            return;
        }

        storage.erase(storage.begin() + idx);
        auto& mems = Base::getMembers();
        mems.clear();
        mems.reserve(storage.size());
        for (auto& f : storage) {
            mems.push_back(m_wrapFieldFunc(f));
        }
    }

    virtual bool setSerialisedValueImpl(const SerialisedSeq& value) override
    {
        static_cast<void>(value);
        assert(!"Mustn't be called");
        return false;
    }

    virtual unsigned sizeImpl() const override
    {
        return Base::field().value().size();
    }

    virtual bool hasFixedSizeImpl() const override
    {
        return Field::ParsedOptions::HasSequenceFixedSize;
    }

    virtual void adjustFixedSizeImpl() override
    {
        using Tag =
            typename std::conditional<
                Field::ParsedOptions::HasSequenceFixedSize,
                HasFixedSizeTag,
                HasVarSizeTag
            >::type;
        adjustFixedSizeInternal(Tag());
    }

    virtual Ptr cloneImpl() override
    {
        std::unique_ptr<ArrayListWrapperT<TField> > ptr(new ArrayListWrapperT(Base::field()));
        ptr->m_wrapFieldFunc = m_wrapFieldFunc;
        return ptr;
    }

    virtual void refreshMembersImpl() override
    {
        if (!m_wrapFieldFunc) {
            assert(!"Expected to have callback");
        }

        auto& storage = Base::field().value();
        auto& mems = Base::getMembers();
        mems.clear();
        mems.reserve(storage.size());
        for (auto& f : storage) {
            mems.push_back(m_wrapFieldFunc(f));
            if (!mems.back()->canWrite()) {
                mems.back()->reset();
                assert(mems.back()->canWrite());
            }
        }
    }

    virtual PrefixFieldInfo getPrefixFieldInfoImpl() const override
    {
        using Tag =
            typename std::conditional<
                Field::ParsedOptions::HasSequenceSizeFieldPrefix,
                ElemCountFieldTag,
                typename std::conditional<
                    Field::ParsedOptions::HasSequenceSerLengthFieldPrefix,
                    SerLengthFieldTag,
                    NoPrefixFieldTag
                >::type
            >::type;

        return getPrefixFieldInfoInternal(Tag());
    }

private:
    struct NoPrefixFieldTag {};
    struct ElemCountFieldTag {};
    struct SerLengthFieldTag {};
    struct SerLengthFieldFixedTag {};
    struct SerLengthFieldVarTag {};
    struct HasFixedSizeTag {};
    struct HasVarSizeTag {};

    PrefixFieldInfo getPrefixFieldInfoInternal(NoPrefixFieldTag) const
    {
        return std::make_pair(0, SerialisedSeq());
    }

    PrefixFieldInfo getPrefixFieldInfoInternal(ElemCountFieldTag) const
    {
        using SizeField = typename Field::ParsedOptions::SequenceSizeFieldPrefix;
        static_assert(sizeof(typename SizeField::ValueType) <= sizeof(int), "Prefix field is too big");
        static_assert((sizeof(typename SizeField::ValueType) < sizeof(int)) ||
                      (std::is_signed<typename SizeField::ValueType>::value),
                      "Prefix field is too big");

        SizeField sizeField;
        sizeField.value() = Base::field().value().size();
        return std::make_pair(sizeField.value(), getPrefixFieldSerialised(sizeField));
    }

    PrefixFieldInfo getPrefixFieldInfoInternal(SerLengthFieldTag) const
    {
        using LengthField = typename Field::ParsedOptions::SequenceSerLengthFieldPrefix;
        using Tag =
            typename std::conditional<
                LengthField::ParsedOptions::HasVarLengthLimits,
                SerLengthFieldVarTag,
                SerLengthFieldFixedTag
            >::type;

        return getPrefixFieldInfoInternal(Tag());
    }

    PrefixFieldInfo getPrefixFieldInfoInternal(SerLengthFieldFixedTag) const
    {
        using LengthField = typename Field::ParsedOptions::SequenceSerLengthFieldPrefix;
        static_assert(sizeof(typename LengthField::ValueType) <= sizeof(int), "Prefix field is too big");
        static_assert((sizeof(typename LengthField::ValueType) < sizeof(int)) ||
                      (std::is_signed<typename LengthField::ValueType>::value),
                      "Prefix field is too big");

        LengthField lenField;
        lenField.value() = Base::field().length() - LengthField::maxLength();
        return std::make_pair(lenField.value(), getPrefixFieldSerialised(lenField));
    }

    PrefixFieldInfo getPrefixFieldInfoInternal(SerLengthFieldVarTag) const
    {
        using LengthField = typename Field::ParsedOptions::SequenceSerLengthFieldPrefix;

        auto fullLen = Base::field().length();
        LengthField lenFieldTmp;
        lenFieldTmp.value() = fullLen;
        auto tmpLen = lenFieldTmp.length();
        LengthField lenField;
        lenField.value() = (fullLen - tmpLen);
        if (lenField.length() == tmpLen) {
            assert(static_cast<int>(lenField.value()) <= std::numeric_limits<int>::max());
            return std::make_pair(lenField.value(), getPrefixFieldSerialised(lenField));
        }

        lenField.value() = fullLen - lenField.length();
        assert(static_cast<int>(lenField.value()) <= std::numeric_limits<int>::max());
        return std::make_pair(lenField.value(), getPrefixFieldSerialised(lenField));
    }

    template <typename TPrefixField>
    SerialisedSeq getPrefixFieldSerialised(const TPrefixField& prefixField) const
    {
        SerialisedSeq serData;
        serData.reserve(prefixField.length());
        auto writeIter = std::back_inserter(serData);
        auto es = prefixField.write(writeIter, serData.max_size());
        static_cast<void>(es);
        assert(es == comms::ErrorStatus::Success);
        return serData;
    }

    void adjustFixedSizeInternal(HasVarSizeTag)
    {
        assert(!"Should not be called");
    }

    void adjustFixedSizeInternal(HasFixedSizeTag)
    {
        Base::field().value().resize(Field::ParsedOptions::SequenceFixedSize);
    }


    WrapFieldCallbackFunc m_wrapFieldFunc;
};

using ArrayListWrapperPtr = ArrayListWrapper::Ptr;

template <typename TField>
ArrayListWrapperPtr
makeArrayListWrapper(TField& field)
{
    return
        ArrayListWrapperPtr(
            new ArrayListWrapperT<TField>(field));
}

template <typename TField>
std::unique_ptr<ArrayListWrapperT<TField> >
makeDowncastedArrayListWrapper(TField& field)
{
    return
        std::unique_ptr<ArrayListWrapperT<TField> >(
            new ArrayListWrapperT<TField>(field));
}

}  // namespace field_wrapper

}  // namespace comms_champion



