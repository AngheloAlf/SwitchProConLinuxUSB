#pragma once
#ifndef HIDAPI_WRAPPER__HPP
#define HIDAPI_WRAPPER__HPP

#include <hidapi.h>
#include <array>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <vector>


class HidApi{
public:
  HidApi(const struct hid_device_info *device_info) {
    ptr = hid_open_path(device_info->path);
    if (ptr == nullptr) {
      throw std::runtime_error(error());
    }
  }

  HidApi(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number) {
    ptr = hid_open(vendor_id, product_id, serial_number);
    if (ptr == nullptr) {
      throw std::runtime_error(error());
    }
  }

  ~HidApi(){
    if (ptr != nullptr) {
      hid_close(ptr);
    }
  }


  int write(size_t length, const uint8_t *data) {
    int ret = hid_write(ptr, data, length);
    if (ret < 0) {
      throw std::runtime_error(error());
    }
    return ret;
  }

  template <size_t length>
  int write(const std::array<uint8_t, length> &data) {
    return write(length, data.data());
  }


  int read(size_t length, uint8_t *data) {
    int ret = hid_read(ptr, data, length);
    if (ret < 0) {
      throw std::runtime_error(error());
    }
    return ret;
  }

  template <size_t length>
  int read(std::array<uint8_t, length> &data) {
    return read(length, data.data());
  }


  int read_timeout(size_t length, uint8_t *data, int milliseconds) {
    int ret = hid_read_timeout(ptr, data, length, milliseconds);
    if (ret < 0) {
      throw std::runtime_error(error());
    }
    return ret;
  }

  template <size_t length>
  int read_timeout(std::array<uint8_t, length> &data, int milliseconds) {
    return read_timeout(length, data.data(), milliseconds);
  }


  std::array<uint8_t, 0x400> exchange(size_t length, const uint8_t *data_to_write, bool timed=false, int milliseconds=100) {
    write(length, data_to_write);

    std::array<uint8_t, 0x400> ret;
    ret.fill(0);

    if (timed) {
      int val = read_timeout(ret.size(), ret.data(), milliseconds);
      if (val == 0) {
        throw std::runtime_error("TODO.");
      }
    }
    else {
      read(ret.size(), ret.data());
    }

    return ret;
  }

  template <size_t length>
  std::array<uint8_t, 0x400> exchange(const std::array<uint8_t, length> &data_to_write, bool timed=false, int milliseconds=100) {
    return exchange(data_to_write, timed, milliseconds);
  }

  void set_non_blocking() {
    if (hid_set_nonblocking(ptr, 1) < 0) {
      throw std::runtime_error("Couldn't set non-blocking mode.");
    }
    blocking = false;
  }

  void set_blocking() {
    if (hid_set_nonblocking(ptr, 0) < 0) {
      throw std::runtime_error("Couldn't set blocking mode.");
    }
    blocking = true;
  }


  void get_manufacturer(wchar_t *string, size_t maxlen) const {
    if (hid_get_manufacturer_string(ptr, string, maxlen) < 0) {
      throw std::runtime_error("Couldn't get manufacturer string.");
    }
  }

  void get_product(wchar_t *string, size_t maxlen) const {
    if (hid_get_product_string(ptr, string, maxlen) < 0) {
      throw std::runtime_error("Couldn't get product string.");
    }
  }

  void get_serial_number(wchar_t *string, size_t maxlen) const {
    if (hid_get_serial_number_string(ptr, string, maxlen) < 0) {
      throw std::runtime_error("Couldn't get serial number string.");
    }
  }

  void get_indexed(int string_index, wchar_t *string, size_t maxlen) const {
    if (hid_get_indexed_string(ptr, string_index, string, maxlen) < 0) {
      throw std::runtime_error("Couldn't get ndexed string.");
    }
  }


  static void init() {
    if (hid_init() < 0) {
      throw std::runtime_error("Hid init error");
    }
  }

  static void exit() {
    if (hid_exit() < 0) {
      throw std::runtime_error("Hid exit error");
    }
  }


  class Enumerate{
  public:
    Enumerate(uint16_t vendor_id, uint16_t product_id) {
      ptr = hid_enumerate(vendor_id, product_id);
      if (ptr == nullptr) {
        throw std::runtime_error("Hid enumerate error");
      }
    }

    ~Enumerate() {
      if (ptr != nullptr) {
        hid_free_enumeration(ptr);
      }
    }

    const struct hid_device_info *device_info() const {
      return ptr;
    }

  private:
    struct hid_device_info *ptr = nullptr;
  };

  static constexpr uint16_t any_vendor{0};
  static constexpr uint16_t any_product{0};

private:
  static std::string wide_to_string(const wchar_t *wide) {
    //size_t len = wcslen(wide);

    std::mbstate_t state/* = std::mbstate_t()*/;
    std::size_t len = 1 + std::wcsrtombs(nullptr, &wide, 0, &state);
    std::vector<char> mbstr(len);
    std::wcsrtombs(&mbstr[0], &wide, mbstr.size(), &state);

    return std::string(&mbstr[0]);
  }
  std::string error() {
    return wide_to_string(hid_error(ptr));
  }

  hid_device *ptr = nullptr;
  bool blocking = true;
};

#endif
