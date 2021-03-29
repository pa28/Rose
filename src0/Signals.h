/** @file Signals.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-13
  * @brief Signals and Slots: used to communicate between objects.
  */

#pragma once

#include <functional>
#include <memory>

namespace rose {

    /**
     * @page signals_slots Signals and Slots
     * @brief Signals and Slots are a communications abstraction.
     *
     * Signals and Slots are an abstraction of a member method call that removes the need
     * for the caller to know the type and structure of the callee. The caller creates
     * a Signal member which is later connected to a Slot member of the intended receiver.
     * The connection is made by an object, possibly a third party, that is aware of the
     * Signal in the Caller and the Slot in the callee. Connected signals and slots must
     * share the same DataType.
     *
     * @section slot Slots
     *
     * A Slot is a many to one receiver. Many Signal objects may connect to a single Slot.
     * When a slot receives data it calls a callback function to pass the data on.
     *
     * @section signal Signals
     *
     * A Signal is a one to many transmitter. One Signal object may connect to many Slot objects.
     * To transmit data simply call the Signal::Transmit method. The data will be sent to all
     * connected, and still valid, Slot objects.
     */

    /**
     * @brief A class to provide unique serial numbers for objects using Signals and Slots.
     */
    class SignalSerialNumber {
    protected:
        uint32_t mSerialNumber;                 ///< This serial number

        static uint32_t sNextSerialNumber;      ///< The next serial number

    public:
        /**
         * @brief Constructor, sets the serial number to the next available serial number.
         */
        SignalSerialNumber() : mSerialNumber(sNextSerialNumber++) {}

        /**
         * @brief Get the serial number
         * @return the serial number
         */
        [[nodiscard]] uint32_t serialNumber() const { return mSerialNumber; }

        /**
         * @brief An alternate method for accessing the serial number.
         * @return The serial number.
         */
        [[nodiscard]] uint32_t operator() () const { return mSerialNumber; }

        /**
         * @brief Test for equality with a received serial number
         * @param other the received serial number
         * @return true if equal
         */
        bool operator==(uint32_t &other) const { return mSerialNumber == other; }

        /**
         * @brief Test for inequality with a received serial number
         * @param other the received serial number
         * @return true if not equal
         */
        bool operator!=(uint32_t &other) const { return mSerialNumber != other; }
    };

    /**
     * @brief A class to act as a data receiver in a signal/slot pair.
     * @details Each class which is using Signals or Slots must have a SignalSerialNumber which is passed
     * to the Signal transmit method, and checked int the Slot callback method to prevent forwarding a
     * signal it originated, thus preventing loops.
     *
     * @tparam DataType the type of data received
     */
    template<typename DataType>
    class Slot : public std::enable_shared_from_this<Slot<DataType>> {

        /**
         * @ingroup signals_slots
         *
         */
    protected:
        std::function<void(uint32_t, DataType)> mCallback{};  //!< The callback called on incoming data

    public:
        /**
         * @brief Set the callback called when data is received
         * @param callback the callback function
         */
        void setCallback(std::function<void(uint32_t, DataType)> callback) { mCallback = callback; }

        /**
         * @brief A function called by an object to send data to the slot.
         * @param serialNumber The serial number of the transmitting signal.
         * @param dataType The type of data.
         */
        void receive(const uint32_t serialNumber, const DataType &dataType) {
            if (mCallback)
                mCallback(serialNumber, dataType);
        }

        /**
         * @brief Get a shared pointer to the slot for holding in the transmitter.
         * @return the shared slot
         */
        std::shared_ptr<Slot<DataType>> connect() { return this->shared_from_this(); }
    };

    /**
     * @brief A class to act as a data transmitter in a signal/slot pair
     * @tparam DataType
     */
    template<typename DataType>
    class Signal {
    protected:
        std::vector<std::shared_ptr<Slot<DataType>>> mSlots{};  //!< A list of slots to send data to

    public:
        /**
         * @brief Add a slot to the list of data recipients
         * @param slot The slot to add, comes from Slot<T>::connect()
         */
        void connect(std::shared_ptr<Slot<DataType>> slot) {
            if (std::find(mSlots.begin(), mSlots.end(), slot) == mSlots.end())
                mSlots.emplace_back(slot);
        }

        /**
         * @brief Disconnect this Signal from all connected Slots
         */
        void disconnectAll() {
            mSlots.clear();
        }

        /**
         * @brief determine of there are any slots to receive the signal.
         * @return True if there is one or more connected Slot objects.
         */
        constexpr explicit operator bool() const noexcept {
            return !mSlots.empty();
        }

        /**
         * @brief Transmit data to the list of recipients
         * @param dataType
         * @param serialNumber the serial number to use, if 0 use the class serial number.
         */
        void transmit(uint32_t serialNumber, const DataType &dataType) {
            for (auto &slot : mSlots) {
                if (slot)
                    slot->receive(serialNumber, dataType);
            }
        }
    };

#if 0
    /**
     * @brief An example implementation using signals and slots.
     */
    class Receiver {
    public:
        std::shared_ptr<Slot<int>> intSlot;
        std::shared_ptr<Slot<std::string>> stringSlot;

        Receiver() {
            intSlot = std::make_shared<Slot<int>>();
            stringSlot = std::make_shared<Slot<std::string>>();

            intSlot->setCallback([](const int s){
                std::cout << __PRETTY_FUNCTION__ << " Received: " << s << '\n';
            });

            stringSlot->setCallback([](const std::string &s){
               std::cout << __PRETTY_FUNCTION__ << " Recieved: " << s << '\n';
            });
        }
    };

    class Transmitter {
    public:
        Signal<int> intSignal{};
        Signal<std::string> stringSignal{};
    };

    Receiver receiver;
    Transmitter transmitter;
    transmitter.intSignal.connect(receiver.intSlot->connect());
    transmitter.stringSignal.connect(receiver.stringSlot->connect());

    transmitter.intSignal.transmit(42);
    transmitter.stringSignal.transmit("Hello World!");

#endif

}

/**
 * @brief Test for equality of signal serial numbers
 * @param serialNumber the serial number value
 * @param signalSerialNumber the serial number object
 * @return boolean
 */
inline bool operator==(uint32_t serialNumber, const rose::SignalSerialNumber &signalSerialNumber) {
    return signalSerialNumber.operator==(serialNumber);
}

/**
 * @brief Test for inequality of signal serial numbers
 * @param serialNumber the serial number value
 * @param signalSerialNumber the serial number object
 * @return boolean
 */
inline bool operator!=(uint32_t serialNumber, const rose::SignalSerialNumber &signalSerialNumber) {
    return signalSerialNumber.operator!=(serialNumber);
}

