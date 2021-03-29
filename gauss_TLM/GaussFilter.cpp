#include <cmath>
#include <iomanip>

#include "GaussFilter.h"

GaussFilter::GaussFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &GaussFilter::blocking_transport);
}

// gauss mask
const int mask[MASK_X][MASK_Y] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

void GaussFilter::do_filter() {
  { wait(CLOCK_PERIOD, SC_NS); }
  int x, y, a, b;
  while (true) {
    unsigned char r_buf[256][256];
    unsigned char g_buf[256][256];
    unsigned char b_buf[256][256];
    unsigned int cnt = 0;
    for (y = 0; y != 256 ; ++y) {
      for (x = 0; x != 256; ++x) {
        r_buf[x][y] = i_r.read();
        g_buf[x][y] = i_g.read();
        b_buf[x][y] = i_b.read();
// std::cout << "in_fifo = " << (int)r_buf[x][y] << ", " << (int)g_buf[x][y] << ", " << (int)b_buf[x][y] << std::endl;
        wait(CLOCK_PERIOD, SC_NS);
      }
    } 
    for (b = 0; b != 256; ++b) {
      for (a = 0; a != 256; ++a) {
// std::cout << "(a, b) = " << a << ", " << b << std::endl;
        if ((a >= 1) && (b >= 1) && (a <= 254) && (b <= 254)) {
          r_val = r_buf[a - 1][b - 1] * mask[0][0] + r_buf[a][b - 1] * mask[1][0] + r_buf[a + 1][b - 1] * mask[2][0]
                + r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1]
                + r_buf[a - 1][b + 1] * mask[0][2] + r_buf[a][b + 1] * mask[1][2] + r_buf[a + 1][b + 1] * mask[2][2];
          g_val = g_buf[a - 1][b - 1] * mask[0][0] + g_buf[a][b - 1] * mask[1][0] + g_buf[a + 1][b - 1] * mask[2][0]
                + g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1]
                + g_buf[a - 1][b + 1] * mask[0][2] + g_buf[a][b + 1] * mask[1][2] + g_buf[a + 1][b + 1] * mask[2][2]; 
          b_val = b_buf[a - 1][b - 1] * mask[0][0] + b_buf[a][b - 1] * mask[1][0] + b_buf[a + 1][b - 1] * mask[2][0]
                + b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1]
                + b_buf[a - 1][b + 1] * mask[0][2] + b_buf[a][b + 1] * mask[1][2] + b_buf[a + 1][b + 1] * mask[2][2];                
          cnt = 16;
        } else if ((a == 0) && (b == 0)) {
          r_val = r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1]
                + r_buf[a][b + 1] * mask[1][2] + r_buf[a + 1][b + 1] * mask[2][2];
          g_val = g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1]
                + g_buf[a][b + 1] * mask[1][2] + g_buf[a + 1][b + 1] * mask[2][2]; 
          b_val = b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1]
                + b_buf[a][b + 1] * mask[1][2] + b_buf[a + 1][b + 1] * mask[2][2];                
          cnt = 9;
        } else if ((a == 255) && (b == 0)) {
          r_val = r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1]
                + r_buf[a - 1][b + 1] * mask[0][2] + r_buf[a][b + 1] * mask[1][2];
          g_val = g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1]
                + g_buf[a - 1][b + 1] * mask[0][2] + g_buf[a][b + 1] * mask[1][2]; 
          b_val = b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1]
                + b_buf[a - 1][b + 1] * mask[0][2] + b_buf[a][b + 1] * mask[1][2]; 
          cnt = 9;
        } else if ((a == 0) && (b == 255)) {
          r_val = mask[0][0] + r_buf[a][b - 1] * mask[1][0] + r_buf[a + 1][b - 1] * mask[2][0]
                + mask[0][1] + r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1];
          g_val = mask[0][0] + g_buf[a][b - 1] * mask[1][0] + g_buf[a + 1][b - 1] * mask[2][0]
                + mask[0][1] + g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1]; 
          b_val = mask[0][0] + b_buf[a][b - 1] * mask[1][0] + b_buf[a + 1][b - 1] * mask[2][0]
                + mask[0][1] + b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1];                
          cnt = 9;          
        } else if ((a == 255) && (b == 255)) {
          r_val = r_buf[a - 1][b - 1] * mask[0][0] + r_buf[a][b - 1] * mask[1][0]
                + r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1];
          g_val = g_buf[a - 1][b - 1] * mask[0][0] + g_buf[a][b - 1] * mask[1][0]
                + g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1];
          b_val = b_buf[a - 1][b - 1] * mask[0][0] + b_buf[a][b - 1] * mask[1][0]
                + b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1];
          cnt = 9;          
        } else if (b == 0) {
          r_val = r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1]
                + r_buf[a - 1][b + 1] * mask[0][2] + r_buf[a][b + 1] * mask[1][2] + r_buf[a + 1][b + 1] * mask[2][2];
          g_val = g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1]
                + g_buf[a - 1][b + 1] * mask[0][2] + g_buf[a][b + 1] * mask[1][2] + g_buf[a + 1][b + 1] * mask[2][2]; 
          b_val = b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1]
                + b_buf[a - 1][b + 1] * mask[0][2] + b_buf[a][b + 1] * mask[1][2] + b_buf[a + 1][b + 1] * mask[2][2];                
          cnt = 12;
        } else if (a == 0) {
          r_val = r_buf[a][b - 1] * mask[1][0] + r_buf[a + 1][b - 1] * mask[2][0]
                + r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1]
                + r_buf[a][b + 1] * mask[1][2] + r_buf[a + 1][b + 1] * mask[2][2];
          g_val = g_buf[a][b - 1] * mask[1][0] + g_buf[a + 1][b - 1] * mask[2][0]
                + g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1]
                + g_buf[a][b + 1] * mask[1][2] + g_buf[a + 1][b + 1] * mask[2][2]; 
          b_val = b_buf[a][b - 1] * mask[1][0] + b_buf[a + 1][b - 1] * mask[2][0]
                + b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1]
                + b_buf[a][b + 1] * mask[1][2] + b_buf[a + 1][b + 1] * mask[2][2];                
          cnt = 12;          
        } else if (b == 255) {
          r_val = r_buf[a - 1][b - 1] * mask[0][0] + r_buf[a][b - 1] * mask[1][0] + r_buf[a + 1][b - 1] * mask[2][0]
                + r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1] + r_buf[a + 1][b]     * mask[2][1];
          g_val = g_buf[a - 1][b - 1] * mask[0][0] + g_buf[a][b - 1] * mask[1][0] + g_buf[a + 1][b - 1] * mask[2][0]
                + g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1] + g_buf[a + 1][b]     * mask[2][1];
          b_val = b_buf[a - 1][b - 1] * mask[0][0] + b_buf[a][b - 1] * mask[1][0] + b_buf[a + 1][b - 1] * mask[2][0]
                + b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1] + b_buf[a + 1][b]     * mask[2][1];
          cnt = 12;
        } else if (a == 255) {
          r_val = r_buf[a - 1][b - 1] * mask[0][0] + r_buf[a][b - 1] * mask[1][0]
                + r_buf[a - 1][b]     * mask[0][1] + r_buf[a][b]     * mask[1][1]
                + r_buf[a - 1][b + 1] * mask[0][2] + r_buf[a][b + 1] * mask[1][2];
          g_val = g_buf[a - 1][b - 1] * mask[0][0] + g_buf[a][b - 1] * mask[1][0]
                + g_buf[a - 1][b]     * mask[0][1] + g_buf[a][b]     * mask[1][1]
                + g_buf[a - 1][b + 1] * mask[0][2] + g_buf[a][b + 1] * mask[1][2]; 
          b_val = b_buf[a - 1][b - 1] * mask[0][0] + b_buf[a][b - 1] * mask[1][0]
                + b_buf[a - 1][b]     * mask[0][1] + b_buf[a][b]     * mask[1][1]
                + b_buf[a - 1][b + 1] * mask[0][2] + b_buf[a][b + 1] * mask[1][2];                
          cnt = 12;          
        } else;
        wait(CLOCK_PERIOD, SC_NS);
        o_result_r.write(r_val / cnt);    
        o_result_g.write(g_val / cnt);
        o_result_b.write(b_val / cnt);
// std::cout << "sum = " << (r_val / cnt) << ", " << (g_val / cnt)  << ", " << (b_val / cnt) << std::endl;    
      }
    }
  }
}

void GaussFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_RESULT_ADDR:
// std::cout << "WRITE" << std::endl;
      buffer.uc[0] = (char)(o_result_r.read());
      buffer.uc[1] = (char)(o_result_g.read());
      buffer.uc[2] = (char)(o_result_b.read());
      buffer.uc[3] = 0;
// std::cout << "output = " << int(buffer.uc[0]) << ", " << int(buffer.uc[1]) << ", " << int(buffer.uc[2]) << std::endl;
      break;
    default:
      buffer.uc[0] = 0;
      buffer.uc[1] = 0;
      buffer.uc[2] = 0;
      buffer.uc[3] = 0;    
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];    
    break;

  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_R_ADDR:
// std::cout << "READ" << std::endl;
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
// std::cout << "write = " << int(data_ptr[0]) << ", ";
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
// std::cout << int(data_ptr[1]) << ", ";
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
// std::cout << int(data_ptr[2]) << std::endl;
      }
      break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
