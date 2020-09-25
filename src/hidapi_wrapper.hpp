#pragma once
#ifndef HIDAPI_WRAPPER__HPP
#define HIDAPI_WRAPPER__HPP

#include <hidapi/hidapi.h>

#include <array>
#include <string>
#include <stdexcept>


class HidApi{
public:
  static constexpr size_t default_length{0x400};

  class HidApiError: public std::runtime_error {
  public:
    HidApiError();
    HidApiError(const std::string& what_arg);
    HidApiError(const char* what_arg);
    HidApiError(hid_device *ptr);
    HidApiError(hid_device *ptr, const char* what_arg);

    ~HidApiError();

    const char *what() const noexcept;
  
  private:
    char *str = nullptr;
  };

  class Enumerate{
  public:
    Enumerate(uint16_t vendor_id, uint16_t product_id);

    ~Enumerate();

    const struct hid_device_info *device_info() const;

    static constexpr uint16_t any_vendor{0};
    static constexpr uint16_t any_product{0};

  private:
    struct hid_device_info *ptr = nullptr;
  };


  HidApi(const struct hid_device_info *device_info);

  HidApi(const HidApi::Enumerate &info);

  HidApi(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number);

  ~HidApi();


  size_t write(size_t len, const uint8_t *data);

  template <size_t len>
  size_t write(const std::array<uint8_t, len> &data) {
    return write(len, data.data());
  }


  size_t read(size_t len, uint8_t *data, int milliseconds=-1);

  template <size_t len>
  size_t read(std::array<uint8_t, len> &data, int milliseconds=-1) {
    return read(len, data.data());
  }

  std::array<uint8_t, HidApi::default_length> read(int milliseconds=-1);


  size_t exchange(size_t read_len, uint8_t *buf, size_t write_len, const uint8_t *data_to_write, int milliseconds=-1);

  template <size_t read_len, size_t write_len>
  size_t exchange(std::array<uint8_t, read_len> &buf, const std::array<uint8_t, write_len> &data_to_write, int milliseconds=-1) {
    return exchange(read_len, buf.data(), write_len, data_to_write.data(), milliseconds);
  }

  std::array<uint8_t, HidApi::default_length> exchange(size_t write_len, const uint8_t *data_to_write, int milliseconds=-1);

  template <size_t len>
  std::array<uint8_t, HidApi::default_length> exchange(const std::array<uint8_t, len> &data_to_write, int milliseconds=-1) {
    return exchange(len, data_to_write.data(), milliseconds);
  }

  void set_non_blocking();

  void set_blocking();


  std::string get_manufacturer() const;

  std::string get_product() const;

  std::string get_serial_number() const;

  std::string get_indexed(int string_index) const;


  static void init();

  static void exit();

private:
  static std::string wide_to_string(const wchar_t *wide);

  hid_device *ptr = nullptr;
  bool blocking = true;
};

#endif
