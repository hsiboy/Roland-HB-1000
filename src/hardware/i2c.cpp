#include "i2c.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <cstring>

namespace pg1000 {
namespace hardware {

bool I2C::init(Bus bus) {
    const Config& config = get_config(bus);
    i2c_inst_t* i2c = get_i2c_inst(bus);
    
    // Initialize I2C peripheral
    i2c_init(i2c, config.frequency);
    
    // Configure pins
    gpio_set_function(config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config.scl_pin, GPIO_FUNC_I2C);
    
    // Enable pull-ups
    gpio_pull_up(config.sda_pin);
    gpio_pull_up(config.scl_pin);
    
    return true;
}

bool I2C::init_all() {
    return init(Bus::BUS0) && init(Bus::BUS1);
}

bool I2C::write_byte(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t data) {
    if (!check_bus(bus)) return false;
    
    uint8_t buf[2] = {reg, data};
    return i2c_write_blocking(get_i2c_inst(bus), device_addr, buf, 2, false) == 2;
}

bool I2C::write_bytes(Bus bus, uint8_t device_addr, uint8_t reg, const uint8_t* data, size_t length) {
    if (!check_bus(bus) || !data || length == 0) return false;
    
    // Allocate temporary buffer for register address + data
    uint8_t* buf = new uint8_t[length + 1];
    if (!buf) return false;
    
    // Prepare buffer
    buf[0] = reg;
    memcpy(buf + 1, data, length);
    
    // Write data
    bool success = i2c_write_blocking(get_i2c_inst(bus), device_addr, buf, length + 1, false) == length + 1;
    
    delete[] buf;
    return success;
}

bool I2C::read_byte(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t& data) {
    if (!check_bus(bus)) return false;
    
    // Write register address
    if (i2c_write_blocking(get_i2c_inst(bus), device_addr, &reg, 1, true) != 1) {
        return false;
    }
    
    // Read data
    return i2c_read_blocking(get_i2c_inst(bus), device_addr, &data, 1, false) == 1;
}

bool I2C::read_bytes(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t* data, size_t length) {
    if (!check_bus(bus) || !data || length == 0) return false;
    
    // Write register address
    if (i2c_write_blocking(get_i2c_inst(bus), device_addr, &reg, 1, true) != 1) {
        return false;
    }
    
    // Read data
    return i2c_read_blocking(get_i2c_inst(bus), device_addr, data, length, false) == length;
}

bool I2C::write_raw(Bus bus, uint8_t device_addr, uint8_t data) {
    if (!check_bus(bus)) return false;
    
    return i2c_write_blocking(get_i2c_inst(bus), device_addr, &data, 1, false) == 1;
}

bool I2C::read_raw(Bus bus, uint8_t device_addr, uint8_t& data) {
    if (!check_bus(bus)) return false;
    
    return i2c_read_blocking(get_i2c_inst(bus), device_addr, &data, 1, false) == 1;
}

bool I2C::device_present(Bus bus, uint8_t device_addr) {
    if (!check_bus(bus)) return false;
    
    uint8_t dummy;
    int result = i2c_read_blocking(get_i2c_inst(bus), device_addr, &dummy, 1, false);
    
    return result >= 0;  // Non-negative result indicates device responded
}

void I2C::scan_bus(Bus bus) {
    if (!check_bus(bus)) return;
    
    printf("Scanning I2C bus %d...\n", static_cast<int>(bus));
    
    for (uint8_t addr = 0x08; addr < 0x77; addr++) {
        if (device_present(bus, addr)) {
            printf("Device found at address 0x%02X\n", addr);
        }
    }
    
    printf("Scan complete.\n");
}

i2c_inst_t* I2C::get_i2c_inst(Bus bus) {
    return (bus == Bus::BUS0) ? i2c0 : i2c1;
}

bool I2C::check_bus(Bus bus) {
    return (bus == Bus::BUS0 || bus == Bus::BUS1);
}

const I2C::Config& I2C::get_config(Bus bus) {
    return (bus == Bus::BUS0) ? BUS0_CONFIG : BUS1_CONFIG;
}

} // namespace hardware
} // namespace pg1000
