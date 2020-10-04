#pragma once
#ifndef HIDAPI_WRAPPER__HPP
#define HIDAPI_WRAPPER__HPP

#include <hidapi/hidapi.h>

#include <array>
#include <string>
#include <stdexcept>


namespace HidApi{
  template <size_t length>
  using generic_packet = std::array<uint8_t, length>;

  static constexpr size_t default_length{0x400};
  using default_packet = generic_packet<default_length>;

  class HidApiError: public std::runtime_error {
  public:
    HidApiError();
    HidApiError(const std::string& what_arg);
    HidApiError(const char* what_arg);
    HidApiError(hid_device *ptr);
    HidApiError(hid_device *ptr, const std::string& what_arg);
    HidApiError(hid_device *ptr, const char* what_arg);

    ~HidApiError();

    const char *what() const noexcept;
  
  private:
    char *str = nullptr;
  };

  class InitError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class ExitError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class EnumerateError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class OpenError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class StateChangeError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class GetterError: public HidApiError {
    using HidApiError::HidApiError;
  };

  class IOError: public HidApiError {
    using HidApiError::HidApiError;
  };
  class WriteError: public IOError {
    using IOError::IOError;
  };
  class ReadError: public IOError {
    using IOError::IOError;
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

  class Device {
  public:
    Device(const struct hid_device_info *device_info);
    Device(const HidApi::Enumerate &info);
    Device(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number);

    ~Device();

    size_t write(size_t len, const uint8_t *data);

    template <size_t len>
    size_t write(const generic_packet<len> &data) {
      return write(len, data.data());
    }


    size_t read(size_t len, uint8_t *data, int milliseconds=-1);

    template <size_t len>
    size_t read(generic_packet<len> &data, int milliseconds=-1) {
      return read(len, data.data(), milliseconds);
    }

    default_packet read(int milliseconds=-1);


    size_t exchange(size_t read_len, uint8_t *buf, size_t write_len, const uint8_t *data_to_write, int milliseconds=-1);

    template <size_t read_len, size_t write_len>
    size_t exchange(generic_packet<read_len> &buf, const generic_packet<write_len> &data_to_write, int milliseconds=-1) {
      return exchange(read_len, buf.data(), write_len, data_to_write.data(), milliseconds);
    }

    default_packet exchange(size_t write_len, const uint8_t *data_to_write, int milliseconds=-1);

    template <size_t len>
    default_packet exchange(const generic_packet<len> &data_to_write, int milliseconds=-1) {
      return exchange(len, data_to_write.data(), milliseconds);
    }

    void set_non_blocking();
    void set_blocking();
    bool IsBlocking() const;

    std::string get_manufacturer() const;
    std::string get_product() const;
    std::string get_serial_number() const;
    std::string get_indexed(int string_index) const;

  private:
    hid_device *ptr = nullptr;
    bool blocking = true;
  };

  void init();
  void exit();
};

#endif
