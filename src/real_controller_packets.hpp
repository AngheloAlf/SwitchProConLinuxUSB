#pragma once
#ifndef PRO__REAL_CONTROLLER_PACKETS_HPP
#define PRO__REAL_CONTROLLER_PACKETS_HPP


namespace RealController {
  enum PacketType {
    unknown = -1,           /// For unrecognized packets.
    standard_input_report,  /// Standard input report format. [0] == x21 || x30 || x31
    normal_ctrl_report,     /// [0] == x3F
    packet_req,             /// ?
    packet_none,
  };


  enum Protocols {
    zero_one      = 0x01,
    one_zero      = 0x10,
    nintendo      = 0x80,
    nin_response  = 0x81,
  };

  enum Uart {
    status        = 0x01,
    handshake     = 0x02,
    inc_baudrate  = 0x03,
    hid_only      = 0x04,
    turn_off_hid  = 0x05,
    reset         = 0x06,
    //prehand_cmd   = 0x91,
    uart_cmd      = 0x92,
  };

  enum Cmd {
    sub_command   = 0x01,
    rumble_only   = 0x10,
    //nfc_ir_req    = 0x11,
    get_input     = 0x1f, // ?
  };

  enum SubCmd {
    zero          = 0x00, // ??
    //req_dev_info  = 0x02,
    set_in_report = 0x03, /// Set input report mode
    set_leds      = 0x30,
    get_leds      = 0x31,
    //set_home_led  = 0x38,
    en_imu        = 0x40,
    set_imu       = 0x41,
    en_rumble     = 0x48,
    //get_voltage   = 0x50, /// Get regullated voltage.
  };

};

#endif
