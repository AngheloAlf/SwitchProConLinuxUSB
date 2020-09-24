#pragma once
#ifndef HIDAPI_WRAPPER__HPP
#define HIDAPI_WRAPPER__HPP

#include <hidapi.h>
#include <array>
#include <string>


class HidApi{
public:
  class Enumerate{
  public:
    Enumerate(uint16_t vendor_id, uint16_t product_id);

    ~Enumerate();

    const struct hid_device_info *device_info() const;

  private:
    struct hid_device_info *ptr = nullptr;
  };


  HidApi(const struct hid_device_info *device_info);

  HidApi(const HidApi::Enumerate &info);

  HidApi(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number);

  ~HidApi();


  int write(size_t length, const uint8_t *data);

  template <size_t length>
  int write(const std::array<uint8_t, length> &data) {
    return write(length, data.data());
  }


  int read(size_t length, uint8_t *data);

  template <size_t length>
  int read(std::array<uint8_t, length> &data) {
    return read(length, data.data());
  }


  int read_timeout(size_t length, uint8_t *data, int milliseconds);

  template <size_t length>
  int read_timeout(std::array<uint8_t, length> &data, int milliseconds) {
    return read_timeout(length, data.data(), milliseconds);
  }


  std::array<uint8_t, 0x400> exchange(size_t length, const uint8_t *data_to_write, bool timed=false, int milliseconds=100);

  template <size_t length>
  std::array<uint8_t, 0x400> exchange(const std::array<uint8_t, length> &data_to_write, bool timed=false, int milliseconds=100) {
    return exchange(length, data_to_write.data(), timed, milliseconds);
  }

  void set_non_blocking();

  void set_blocking();


  void get_manufacturer(wchar_t *string, size_t maxlen) const;

  void get_product(wchar_t *string, size_t maxlen) const;

  void get_serial_number(wchar_t *string, size_t maxlen) const;

  void get_indexed(int string_index, wchar_t *string, size_t maxlen) const;


  static void init();

  static void exit();

  static constexpr uint16_t any_vendor{0};
  static constexpr uint16_t any_product{0};

private:
  static std::string wide_to_string(const wchar_t *wide);
  std::string error();

  hid_device *ptr = nullptr;
  bool blocking = true;
};

#endif
