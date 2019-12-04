//
// Copyright 2016 - 2018 (C). Alex Robenko. All rights reserved.
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

/// @file
/// @brief Contains common interface for all the messages of demo binary protocol.

#pragma once

#include <cstdint>

#include "comms/Message.h"
#include "comms/field/IntValue.h"
#include "comms/Field.h"

#include "MsgId.h"

namespace demo
{

/// @brief Endian option for the protocol
using ProtocolEndian = comms::option::BigEndian;

/// @brief Field containing version information
using VersionField =
    comms::field::IntValue<
        comms::Field<ProtocolEndian>,
        std::uint8_t,
        comms::option::DefaultNumValue<1>,
        comms::option::ValidNumValueRange<0, 1>
    >;

/// @brief Extra transport fields that every message object will contain
using ExtraTransportFields =
    std::tuple<
        VersionField
    >;

/// @brief Interface class of all the Demo binary protocol messages.
/// @details Defined as alias to @b comms::Message
///     class while providing the following default options:
///     @li @b comms::option::MsgIdType<MsgId> > - use @ref MsgId as the type of message ID.
///     @li @b comms::option::BigEndian - use big endian for serialisation.
///
///     All other options provided with TOptions template parameter will also be passed
///     to @b comms::Message to define the interface.
/// @tparam TOptions Zero or more extra options to be passed to @b comms::Message
///     to define the interface.
template <typename... TOptions>
struct Message : public
    comms::Message<
        ProtocolEndian,
        comms::option::MsgIdType<MsgId>,
        comms::option::ExtraTransportFields<ExtraTransportFields>,
        comms::option::VersionInExtraTransportFields<0>,
        TOptions...
    >
{
    using Base =
        comms::Message<
            ProtocolEndian,
            comms::option::MsgIdType<MsgId>,
            comms::option::ExtraTransportFields<ExtraTransportFields>,
            comms::option::VersionInExtraTransportFields<0>,
            TOptions...
        >;
public:

    /// @brief Allow access to extra transport fields.
    /// @details See definition of @b COMMS_MSG_TRANSPORT_FIELDS_NAMES macro
    ///     related to @b comms::Message class from COMMS library
    ///     for details.
    ///
    COMMS_MSG_TRANSPORT_FIELDS_NAMES(version);
};

}  // namespace demo


