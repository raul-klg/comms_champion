//
// Copyright 2015 (C). Alex Robenko. All rights reserved.
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

#include <memory>

#include "comms/comms.h"

#include "comms_champion/FieldWidget.h"
#include "comms_champion/field_wrapper/IntValueWrapper.h"
#include "comms_champion/field_wrapper/BitmaskValueWrapper.h"
#include "comms_champion/field_wrapper/EnumValueWrapper.h"
#include "comms_champion/field_wrapper/StringWrapper.h"
#include "comms_champion/field_wrapper/BitfieldWrapper.h"
#include "comms_champion/field_wrapper/OptionalWrapper.h"
#include "comms_champion/field_wrapper/ArrayListRawDataWrapper.h"
#include "comms_champion/field_wrapper/ArrayListWrapper.h"
#include "comms_champion/field_wrapper/FloatValueWrapper.h"
#include "comms_champion/field_wrapper/UnknownValueWrapper.h"

namespace comms_champion
{

namespace details
{

struct IntValueTag {};
struct BitmaskValueTag {};
struct EnumValueTag {};
struct StringTag {};
struct BitfieldTag {};
struct OptionalTag {};
struct BundleTag {};
struct ArrayListTag {};
struct FloatValueTag {};
struct UnknownValueTag {};

template <typename TField>
struct FieldWidgetCreatorTagOf
{
    static_assert(!comms::field::isIntValue<TField>(),
        "IntValue is perceived as unknown type");
    static_assert(!comms::field::isBitmaskValue<TField>(),
        "BitmaskValue is perceived as unknown type");
    static_assert(!comms::field::isEnumValue<TField>(),
        "EnumValue is perceived as unknown type");
    static_assert(!comms::field::isString<TField>(),
        "String is perceived as unknown type");
    static_assert(!comms::field::isBitfield<TField>(),
        "Bitfield is perceived as unknown type");
    static_assert(!comms::field::isOptional<TField>(),
        "Optional is perceived as unknown type");
    static_assert(!comms::field::isArrayList<TField>(),
        "ArrayList is perceived as unknown type");
    static_assert(!comms::field::isFloatValue<TField>(),
        "FloatValue is perceived as unknown type");

    typedef UnknownValueTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::IntValue<TArgs...> >
{
    static_assert(
        comms::field::isIntValue<comms::field::IntValue<TArgs...> >(),
        "isIntValue is supposed to return true");

    typedef IntValueTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::BitmaskValue<TArgs...> >
{
    static_assert(
        comms::field::isBitmaskValue<comms::field::BitmaskValue<TArgs...> >(),
        "isBitmaskValue is supposed to return true");

    typedef BitmaskValueTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::EnumValue<TArgs...> >
{
    static_assert(
        comms::field::isEnumValue<comms::field::EnumValue<TArgs...> >(),
        "isEnumValue is supposed to return true");

    typedef EnumValueTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::String<TArgs...> >
{
    static_assert(
        comms::field::isString<comms::field::String<TArgs...> >(),
        "isString is supposed to return true");

    typedef StringTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::Bitfield<TArgs...> >
{
    static_assert(
        comms::field::isBitfield<comms::field::Bitfield<TArgs...> >(),
        "isBitfield is supposed to return true");

    typedef BitfieldTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::Optional<TArgs...> >
{
    static_assert(
        comms::field::isOptional<comms::field::Optional<TArgs...> >(),
        "isOptional is supposed to return true");

    typedef OptionalTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::Bundle<TArgs...> >
{
    static_assert(
        comms::field::isBundle<comms::field::Bundle<TArgs...> >(),
        "isBundle is supposed to return true");

    typedef BundleTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::ArrayList<TArgs...> >
{
    static_assert(
        comms::field::isArrayList<comms::field::ArrayList<TArgs...> >(),
        "isArrayList is supposed to return true");

    typedef ArrayListTag Type;
};

template <typename... TArgs>
struct FieldWidgetCreatorTagOf<comms::field::FloatValue<TArgs...> >
{
    static_assert(
        comms::field::isFloatValue<comms::field::FloatValue<TArgs...> >(),
        "isFloatValue is supposed to return true");

    typedef FloatValueTag Type;
};


template <typename TField>
using FieldWidgetCreatorTagOfT = typename FieldWidgetCreatorTagOf<TField>::Type;

}  // namespace details

class FieldWidgetCreator
{
public:
    FieldWidgetCreator() = default;

    template <typename TField>
    static FieldWidgetPtr createWidget(TField& field)
    {
        typedef typename std::decay<decltype(field)>::type DecayedField;
        typedef details::FieldWidgetCreatorTagOfT<DecayedField> Tag;
        return createWidgetInternal(field, Tag());
    }

    static void bitfieldWidgetAddMember(
        FieldWidget& bitfieldWidget,
        FieldWidgetPtr memberWidget);

    static void optionalWidgetSetField(
        FieldWidget& optionalWidget,
        FieldWidgetPtr fieldWidget);

    static void bundleWidgetAddMember(
        FieldWidget& bundleWidget,
        FieldWidgetPtr memberWidget);

private:
    using IntValueTag = details::IntValueTag;
    using BitmaskValueTag = details::BitmaskValueTag;
    using EnumValueTag = details::EnumValueTag;
    using StringTag = details::StringTag;
    using BitfieldTag = details::BitfieldTag;
    using OptionalTag = details::OptionalTag;
    using BundleTag = details::BundleTag;
    using ArrayListTag = details::ArrayListTag;
    using FloatValueTag = details::FloatValueTag;
    using UnknownValueTag = details::UnknownValueTag;

    struct RawDataArrayListTag {};
    struct CollectionOfFieldsArrayListTag {};

    class SubfieldsCreateHelper
    {
    public:
        typedef std::function <void (FieldWidgetPtr)> WidgetDispatchFunc;
        SubfieldsCreateHelper(WidgetDispatchFunc&& dispatchOp)
          : m_dispatchOp(std::move(dispatchOp))
        {
        }

        template <typename TField>
        void operator()(TField&& field)
        {
            auto fieldWidget =
                FieldWidgetCreator::createWidget(std::forward<TField>(field));
            m_dispatchOp(std::move(fieldWidget));
        }

    private:
        WidgetDispatchFunc m_dispatchOp;
    };


    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, IntValueTag)
    {
        return createIntValueFieldWidget(field_wrapper::makeIntValueWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, BitmaskValueTag)
    {
        return createBitmaskValueFieldWidget(
            field_wrapper::makeBitmaskValueWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, EnumValueTag)
    {
        return createEnumValueFieldWidget(
            field_wrapper::makeEnumValueWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, StringTag)
    {
        return createStringFieldWidget(
            field_wrapper::makeStringWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, BitfieldTag)
    {
        auto widget = createBitfieldFieldWidget(
            field_wrapper::makeBitfieldWrapper(field));

        auto& memberFields = field.value();
        comms::util::tupleForEach(
            memberFields,
            SubfieldsCreateHelper(
                [&widget](FieldWidgetPtr fieldWidget)
                {
                    FieldWidgetCreator::bitfieldWidgetAddMember(*widget, std::move(fieldWidget));
                }));

        return std::move(widget);
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, OptionalTag)
    {
        auto widget = createOptionalFieldWidget(
            field_wrapper::makeOptionalWrapper(field));

        auto& wrappedField = field.field();
        auto wrappedFieldWidget = createWidget(wrappedField);
        optionalWidgetSetField(*widget, std::move(wrappedFieldWidget));

        return std::move(widget);
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, BundleTag)
    {
        auto widget = createBundleFieldWidget();

        auto& memberFields = field.value();
        comms::util::tupleForEach(
            memberFields,
            SubfieldsCreateHelper(
                [&widget](FieldWidgetPtr fieldWidget)
                {
                    FieldWidgetCreator::bundleWidgetAddMember(*widget, std::move(fieldWidget));
                }));

        return std::move(widget);
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, ArrayListTag)
    {
        typedef typename std::decay<decltype(field)>::type FieldType;
        typedef typename FieldType::ValueType::value_type ElementType;
        typedef typename std::conditional<
            std::is_integral<ElementType>::value,
            RawDataArrayListTag,
            CollectionOfFieldsArrayListTag
        >::type TypeTag;

        return createArrayListFieldWidgetInternal(field, TypeTag());
    }

    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, FloatValueTag)
    {
        return createFloatValueFieldWidget(
            field_wrapper::makeFloatValueWrapper(field));
    }


    template <typename TField>
    static FieldWidgetPtr createWidgetInternal(TField& field, UnknownValueTag)
    {
        return createUnknownValueFieldWidget(
            field_wrapper::makeUnknownValueWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createArrayListFieldWidgetInternal(TField& field, RawDataArrayListTag)
    {
        return createArrayListRawDataFieldWidget(
            field_wrapper::makeArrayListRawDataWrapper(field));
    }

    template <typename TField>
    static FieldWidgetPtr createArrayListFieldWidgetInternal(TField& field, CollectionOfFieldsArrayListTag)
    {
        auto addMissingFieldsFunc =
            [&field](std::size_t from) -> std::vector<FieldWidgetPtr>
            {
                std::vector<FieldWidgetPtr> allFieldsWidgets;

                auto& dataFields = field.value();
                while (from < dataFields.size()) {
                    auto& f = dataFields[from];
                    allFieldsWidgets.push_back(createWidget(f));
                    ++from;
                }

                return allFieldsWidgets;
            };

        auto widget = createArrayListFieldWidget(
            field_wrapper::makeArrayListWrapper(field),
            std::move(addMissingFieldsFunc));

        return std::move(widget);
    }

    static FieldWidgetPtr createIntValueFieldWidget(
        field_wrapper::IntValueWrapperPtr fieldWrapper);

    static FieldWidgetPtr createBitmaskValueFieldWidget(
        field_wrapper::BitmaskValueWrapperPtr fieldWrapper);

    static FieldWidgetPtr createEnumValueFieldWidget(
        field_wrapper::EnumValueWrapperPtr fieldWrapper);

    static FieldWidgetPtr createStringFieldWidget(
        field_wrapper::StringWrapperPtr fieldWrapper);

    static FieldWidgetPtr createBitfieldFieldWidget(
        field_wrapper::BitfieldWrapperPtr fieldWrapper);

    static FieldWidgetPtr createOptionalFieldWidget(
        field_wrapper::OptionalWrapperPtr fieldWrapper);

    static FieldWidgetPtr createBundleFieldWidget();

    static FieldWidgetPtr createArrayListRawDataFieldWidget(
        field_wrapper::ArrayListRawDataWrapperPtr fieldWrapper);

    static FieldWidgetPtr createArrayListFieldWidget(
        field_wrapper::ArrayListWrapperPtr fieldWrapper,
        std::function<std::vector<FieldWidgetPtr> (std::size_t)>&& updateFunc);

    static FieldWidgetPtr createFloatValueFieldWidget(
        field_wrapper::FloatValueWrapperPtr fieldWrapper);

    static FieldWidgetPtr createUnknownValueFieldWidget(
            field_wrapper::UnknownValueWrapperPtr fieldWrapper);

};

}  // namespace comms_champion


