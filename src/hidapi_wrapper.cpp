#include "hidapi_wrapper.hpp"
using namespace HidApi;

#include <cstring>
#include "utils.hpp"

constexpr size_t maxlen = 1024;


HidApiError::HidApiError(): std::runtime_error("Unspecified error") {
  std::string aux = std::string("HidApi: ") + "Unspecified error";
  Utils::Str::copy_string_to_char(str, aux);
}

HidApiError::HidApiError(const std::string& what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("HidApi: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}
HidApiError::HidApiError(const char* what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("HidApi: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}
HidApiError::HidApiError(hid_device *ptr): std::runtime_error("") {
  const wchar_t *er = hid_error(ptr);
  if (er == nullptr) {
    Utils::Str::copy_string_to_char(str, "HidApi: Unknown error");
    return;
  }
  Utils::Str::copy_string_to_char(str, std::string("HidApi: ") + Utils::Str::wide_to_string(er));
}
HidApiError::HidApiError(hid_device *ptr, const std::string& what_arg): std::runtime_error(what_arg) {
  const wchar_t *er = hid_error(ptr);
  if (er == nullptr) {
    Utils::Str::copy_string_to_char(str, std::string("HidApi: ") + what_arg);
    return;
  }
  Utils::Str::copy_string_to_char(str, std::string("HidApi: ") + what_arg + "\n" + Utils::Str::wide_to_string(er));
}
HidApiError::HidApiError(hid_device *ptr, const char* what_arg): std::runtime_error(what_arg) {
  const wchar_t *er = hid_error(ptr);
  if (er == nullptr) {
    Utils::Str::copy_string_to_char(str, std::string("HidApi: ") + what_arg);
    return;
  }
  Utils::Str::copy_string_to_char(str, std::string("HidApi: ") + what_arg + "\n" + Utils::Str::wide_to_string(er));
}


HidApiError::~HidApiError() {
  if (str != nullptr) {
    free(str);
    str = nullptr;
  }
}

const char *HidApiError::what() const noexcept {
  return str;
}


Enumerate::Enumerate(uint16_t vendor_id, uint16_t product_id) {
  ptr = hid_enumerate(vendor_id, product_id);
  if (ptr == nullptr) {
    throw EnumerateError("EnumerateError: Unable to find any requested device.");
  }
}
Enumerate::Enumerate(Enumerate &&other) noexcept: ptr(nullptr) {
  std::swap(ptr, other.ptr);
}

Enumerate::~Enumerate() noexcept {
  if (ptr != nullptr) {
    hid_free_enumeration(ptr);
  }
}

Enumerate &Enumerate::operator=(Enumerate &&other) noexcept {
  std::swap(ptr, other.ptr);
  return *this;
}


const struct hid_device_info *Enumerate::device_info() const noexcept {
  return ptr;
}


Device::Device(const struct hid_device_info *device_info) {
  ptr = hid_open_path(device_info->path);
  if (ptr == nullptr) {
    throw OpenError(ptr, "OpenError: open_path()");
  }
}
Device::Device(const Enumerate &info): Device(info.device_info()) {
}
Device::Device(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number) {
  ptr = hid_open(vendor_id, product_id, serial_number);
  if (ptr == nullptr) {
    throw OpenError(ptr, "OpenError: open()");
  }
}
Device::Device(Device &&other) noexcept: ptr(nullptr) {
  std::swap(ptr, other.ptr);
  std::swap(blocking, other.blocking);
}

Device::~Device(){
  if (ptr != nullptr) {
    hid_close(ptr);
    ptr = nullptr;
  }
}

Device &Device::operator=(Device &&other) noexcept {
  std::swap(ptr, other.ptr);
  std::swap(blocking, other.blocking);
  return *this;
}


size_t Device::write(size_t len, const uint8_t *data) {
  int ret = hid_write(ptr, data, len);
  if (ret < 0) {
    throw WriteError(ptr, "WriteError: write() returned " + std::to_string(ret) + ".");
  }
  if (len != (size_t)ret) {
    throw WriteError(ptr, "WriteError: Couldn't write " + std::to_string(len) + " bytes. Wrote " + std::to_string(ret) + " bytes instead.");
  }
  return ret;
}


size_t Device::read(size_t len, uint8_t *data, int milliseconds) {
  int ret;
  memset(data, 0, len);
  if (milliseconds < 0) {
    ret = hid_read(ptr, data, len);
  }
  else {
    ret = hid_read_timeout(ptr, data, len, milliseconds);
  }

  if (ret < 0) {
    throw ReadError(ptr, "ReadError: read() returned " + std::to_string(ret));
  }
  return ret;
}

DefaultPacket Device::read(int milliseconds) {
  DefaultPacket ret;
  read(default_length, ret.data(), milliseconds);
  return ret;
}


size_t Device::exchange(size_t read_len, uint8_t *buf, size_t write_len, const uint8_t *data_to_write, int milliseconds) {
  write(write_len, data_to_write);

  size_t ret = read(read_len, buf, milliseconds);
  if (milliseconds >= 0 && ret == 0) {
    throw IOError("IOError: Didn't receive exchange packet after " + std::to_string(milliseconds) + " milliseconds.");
  }

  return ret;
}

DefaultPacket Device::exchange(size_t write_len, const uint8_t *data_to_write, int milliseconds) {
  DefaultPacket ret;
  ret.fill(0);
  exchange(default_length, ret.data(), write_len, data_to_write, milliseconds);

  return ret;
}


void Device::set_non_blocking() {
  if (hid_set_nonblocking(ptr, 1) < 0) {
    throw StateChangeError("StateChangeError: Couldn't set non-blocking mode.");
  }
  blocking = false;
}

void Device::set_blocking() {
  if (hid_set_nonblocking(ptr, 0) < 0) {
    throw StateChangeError("StateChangeError: Couldn't set blocking mode.");
  }
  blocking = true;
}

bool Device::IsBlocking() const noexcept {
  return blocking;
}


std::string Device::get_manufacturer() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_manufacturer_string(ptr, buf.data(), maxlen) < 0) {
    throw GetterError("GetterError: Couldn't get manufacturer string.");
  }
  return Utils::Str::wide_to_string(buf.data());
}

std::string Device::get_product() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_product_string(ptr, buf.data(), maxlen) < 0) {
    throw GetterError("GetterError: Couldn't get product string.");
  }
  return Utils::Str::wide_to_string(buf.data());
}

std::string Device::get_serial_number() const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_serial_number_string(ptr, buf.data(), maxlen) < 0) {
    throw GetterError("GetterError: Couldn't get serial number string.");
  }
  return Utils::Str::wide_to_string(buf.data());
}

std::string Device::get_indexed(int string_index) const {
  std::array<wchar_t, maxlen+1> buf;
  if (hid_get_indexed_string(ptr, string_index, buf.data(), maxlen) < 0) {
    throw GetterError("GetterError: Couldn't get ndexed string.");
  }
  return Utils::Str::wide_to_string(buf.data());
}


void HidApi::init() {
  if (hid_init() < 0) {
    throw InitError("InitError: Hid init error");
  }
}

void HidApi::exit() {
  if (hid_exit() < 0) {
    throw ExitError("ExitError: Hid exit error");
  }
}
