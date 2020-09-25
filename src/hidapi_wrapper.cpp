#include "hidapi_wrapper.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>

constexpr size_t maxlen = 1024;


void copy_string_to_char(char **dst, const std::string &src) {
  size_t sz = src.size() + 1;
  *dst = (char *)malloc(sz);
  strncpy(*dst, src.c_str(), sz);
}

HidApi::HidApiError::HidApiError(): std::runtime_error("Unspecified error") {
  std::string aux = std::string("HidApi: ") + "Unspecified error";
  copy_string_to_char(&str, aux);
}

HidApi::HidApiError::HidApiError(const std::string& what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("HidApi: ") + what_arg;
  copy_string_to_char(&str, aux);
}
HidApi::HidApiError::HidApiError(const char* what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("HidApi: ") + what_arg;
  copy_string_to_char(&str, aux);
}
HidApi::HidApiError::HidApiError(hid_device *ptr): std::runtime_error("") {
  const wchar_t *er = hid_error(ptr);
  if (er == nullptr) {
    copy_string_to_char(&str, "HidApi: Unknown error");
    return;
  }
  copy_string_to_char(&str, std::string("HidApi: ") + wide_to_string(er));
}
HidApi::HidApiError::HidApiError(hid_device *ptr, const char* what_arg): std::runtime_error(what_arg) {
  const wchar_t *er = hid_error(ptr);
  if (er == nullptr) {
    copy_string_to_char(&str, std::string("HidApi: ") + what_arg);
    return;
  }
  copy_string_to_char(&str, std::string("HidApi: ") + what_arg + "\n" + wide_to_string(er));
}


HidApi::HidApiError::~HidApiError() {
  free(str);
}

const char *HidApi::HidApiError::what() const noexcept {
  return str;
}


HidApi::Enumerate::Enumerate(uint16_t vendor_id, uint16_t product_id) {
  ptr = hid_enumerate(vendor_id, product_id);
  if (ptr == nullptr) {
    throw HidApi::HidApiError("Unable to find any requested device.");
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
    throw HidApi::HidApiError(ptr);
  }
}

HidApi::HidApi(const HidApi::Enumerate &info): HidApi(info.device_info()) {
}

HidApi::HidApi(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number) {
  ptr = hid_open(vendor_id, product_id, serial_number);
  if (ptr == nullptr) {
    throw HidApi::HidApiError(ptr);
  }
}

HidApi::~HidApi(){
  if (ptr != nullptr) {
    hid_close(ptr);
  }
}


size_t HidApi::write(size_t len, const uint8_t *data) {
  int ret = hid_write(ptr, data, len);
  if (ret < 0) {
    throw HidApi::HidApiError(ptr);
  }
  return ret;
}


size_t HidApi::read(size_t len, uint8_t *data, int milliseconds) {
  int ret;
  if (milliseconds < 0) {
    ret = hid_read(ptr, data, len);
  }
  else {
    ret = hid_read_timeout(ptr, data, len, milliseconds);
  }

  if (ret < 0) {
    throw HidApi::HidApiError(ptr);
  }
  return ret;
}

std::array<uint8_t, HidApi::default_length> HidApi::read(int milliseconds) {
  std::array<uint8_t, HidApi::default_length> ret;
  read(HidApi::default_length, ret.data(), milliseconds);
  return ret;
}


size_t HidApi::exchange(size_t read_len, uint8_t *buf, size_t write_len, const uint8_t *data_to_write, int milliseconds) {
  write(write_len, data_to_write);

  size_t ret = read(read_len, buf, milliseconds);
  if (milliseconds >= 0 && ret == 0) {
    throw HidApi::HidApiError("Didn't receive exchange packet after " + std::to_string(milliseconds) + " milliseconds.");
  }

  return ret;
}

std::array<uint8_t, HidApi::default_length> HidApi::exchange(size_t write_len, const uint8_t *data_to_write, int milliseconds) {
  std::array<uint8_t, HidApi::default_length> ret;
  ret.fill(0);
  exchange(HidApi::default_length, ret.data(), write_len, data_to_write, milliseconds);

  return ret;
}


void HidApi::set_non_blocking() {
  if (hid_set_nonblocking(ptr, 1) < 0) {
    throw HidApi::HidApiError("Couldn't set non-blocking mode.");
  }
  blocking = false;
}

void HidApi::set_blocking() {
  if (hid_set_nonblocking(ptr, 0) < 0) {
    throw HidApi::HidApiError("Couldn't set blocking mode.");
  }
  blocking = true;
}


std::string HidApi::get_manufacturer() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_manufacturer_string(ptr, buf.data(), maxlen) < 0) {
    throw HidApi::HidApiError("Couldn't get manufacturer string.");
  }
  return HidApi::wide_to_string(buf.data());
}

std::string HidApi::get_product() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_product_string(ptr, buf.data(), maxlen) < 0) {
    throw HidApi::HidApiError("Couldn't get product string.");
  }
  return HidApi::wide_to_string(buf.data());
}

std::string HidApi::get_serial_number() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_serial_number_string(ptr, buf.data(), maxlen) < 0) {
    throw HidApi::HidApiError("Couldn't get serial number string.");
  }
  return HidApi::wide_to_string(buf.data());
}

std::string HidApi::get_indexed(int string_index) const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_indexed_string(ptr, string_index, buf.data(), maxlen) < 0) {
    throw HidApi::HidApiError("Couldn't get ndexed string.");
  }
  return HidApi::wide_to_string(buf.data());
}


void HidApi::init() {
  if (hid_init() < 0) {
    throw HidApi::HidApiError("Hid init error");
  }
}

void HidApi::exit() {
  if (hid_exit() < 0) {
    throw HidApi::HidApiError("Hid exit error");
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
