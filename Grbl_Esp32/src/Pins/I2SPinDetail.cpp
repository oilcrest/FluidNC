#ifdef ESP32
#    include "I2SPinDetail.h"

#    include "../I2SOut.h"
#    include "../Assert.h"

extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

namespace Pins {
    I2SPinDetail::I2SPinDetail(uint8_t index, const PinOptionsParser& options) :
        _index(index), _capabilities(PinCapabilities::Output | PinCapabilities::I2S), _attributes(Pins::PinAttributes::Undefined),
        _readWriteMask(0) {
        // User defined pin capabilities
        for (auto opt : options) {
            if (opt.is("pu")) {
                _attributes = _attributes | PinAttributes::PullUp;
            } else if (opt.is("pd")) {
                _attributes = _attributes | PinAttributes::PullDown;
            } else if (opt.is("low")) {
                _attributes = _attributes | PinAttributes::ActiveLow;
            } else if (opt.is("high")) {
                // Default: Active HIGH.
            }
        }

        // Update the R/W mask for ActiveLow setting
        if (_attributes.ActiveLow) {
            __digitalWrite(_index, HIGH);
            _readWriteMask = 1;
        } else {
            _readWriteMask = 0;
        }
    }

    PinCapabilities I2SPinDetail::capabilities() const { return PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::I2S; }

    void I2SPinDetail::write(int high) {
        int value = _readWriteMask ^ high;
        i2s_out_write(_index, value);
    }

    int I2SPinDetail::read() {
        auto raw = i2s_out_read(_index);
        return raw ^ _readWriteMask;
    }

    void I2SPinDetail::setAttr(PinAttributes value) {
        // Check the attributes first:
        Assert(!value.validateWith(this->_capabilities), "The requested attributes don't match the pin capabilities");
        Assert(!_attributes.conflictsWith(value), "Attributes on this pin have been set before, and there's a conflict.");

        _attributes = value;

        // I2S out pins cannot be configured, hence there
        // is nothing to do here for them. We basically
        // just check for conflicts above...
    }

    String I2SPinDetail::toString() const { return String("I2S_") + int(_index); }
}

#endif