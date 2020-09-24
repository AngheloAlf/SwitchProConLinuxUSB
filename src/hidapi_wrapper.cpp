#include "hidapi_wrapper.hpp"


#include <hidapi.h>
#include <cstdlib>
#include <stdexcept>
#include <vector>


HidApi::Enumerate::Enumerate(uint16_t vendor_id, uint16_t product_id) {
  ptr = hid_enumerate(vendor_id, product_id);
  if (ptr == nullptr) {
    throw std::runtime_error("Unable to find any requested device.");
  }
}

HidApi::Enumerate::~Enumerate() {
  if (ptr != nullptr) {
    hid_free_enumeration(ptr);
  }
}

const struct hid_device_info *HidApi::Enumerate::device_info() const {
  return ptr;
}


HidApi::HidApi(const struct hid_device_info *device_info) {
  ptr = hid_open_path(device_info->path);
  if (ptr == nullptr) {
    throw std::runtime_error(error());
  }
}

HidApi::HidApi(const HidApi::Enumerate &info): HidApi(info.device_info()) {
}

HidApi::HidApi(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number) {
  ptr = hid_open(vendor_id, product_id, serial_number);
  if (ptr == nullptr) {
    throw std::runtime_error(error());
  }
}

HidApi::~HidApi(){
  if (ptr != nullptr) {
    hid_close(ptr);
  }
}


int HidApi::write(size_t length, const uint8_t *data) {
  int ret = hid_write(ptr, data, length);
  if (ret < 0) {
    throw std::runtime_error(error());
  }
  return ret;
}



int HidApi::read(size_t length, uint8_t *data) {
  int ret = hid_read(ptr, data, length);
  if (ret < 0) {
    throw std::runtime_error(error());
  }
  return ret;
}



int HidApi::read_timeout(size_t length, uint8_t *data, int milliseconds) {
  int ret = hid_read_timeout(ptr, data, length, milliseconds);
  if (ret < 0) {
    throw std::runtime_error(error());
  }
  return ret;
}



std::array<uint8_t, 0x400> HidApi::exchange(size_t length, const uint8_t *data_to_write, bool timed, int milliseconds) {
  write(length, data_to_write);

  std::array<uint8_t, 0x400> ret;
  ret.fill(0);

  if (timed) {
    int val = read_timeout(ret.size(), ret.data(), milliseconds);
    if (val == 0) {
      throw std::runtime_error("Didn't receive exchange packet after " + std::to_string(milliseconds) + " milliseconds.");
    }
  }
  else {
    read(ret.size(), ret.data());
  }

  return ret;
}


void HidApi::set_non_blocking() {
  if (hid_set_nonblocking(ptr, 1) < 0) {
    throw std::runtime_error("Couldn't set non-blocking mode.");
  }
  blocking = false;
}

void HidApi::set_blocking() {
  if (hid_set_nonblocking(ptr, 0) < 0) {
    throw std::runtime_error("Couldn't set blocking mode.");
  }
  blocking = true;
}


void HidApi::get_manufacturer(wchar_t *string, size_t maxlen) const {
  if (hid_get_manufacturer_string(ptr, string, maxlen) < 0) {
    throw std::runtime_error("Couldn't get manufacturer string.");
  }
}

void HidApi::get_product(wchar_t *string, size_t maxlen) const {
  if (hid_get_product_string(ptr, string, maxlen) < 0) {
    throw std::runtime_error("Couldn't get product string.");
  }
}

void HidApi::get_serial_number(wchar_t *string, size_t maxlen) const {
  if (hid_get_serial_number_string(ptr, string, maxlen) < 0) {
    throw std::runtime_error("Couldn't get serial number string.");
  }
}

void HidApi::get_indexed(int string_index, wchar_t *string, size_t maxlen) const {
  if (hid_get_indexed_string(ptr, string_index, string, maxlen) < 0) {
    throw std::runtime_error("Couldn't get ndexed string.");
  }
}


void HidApi::init() {
  if (hid_init() < 0) {
    throw std::runtime_error("Hid init error");
  }
}

void HidApi::exit() {
  if (hid_exit() < 0) {
    throw std::runtime_error("Hid exit error");
  }
}


std::string HidApi::wide_to_string(const wchar_t *wide) {
  //size_t len = wcslen(wide);

  std::mbstate_t state/* = std::mbstate_t()*/;
  std::size_t len = 1 + std::wcsrtombs(nullptr, &wide, 0, &state);
  std::vector<char> mbstr(len);
  std::wcsrtombs(&mbstr[0], &wide, mbstr.size(), &state);

  return std::string(&mbstr[0]);
}
std::string HidApi::error() {
  return wide_to_string(hid_error(ptr));
}
