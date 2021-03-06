#include <cassert>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "Testbench.h"

unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), initiator("initiator"), output_rgb_raw_data_offset(54) {
  SC_THREAD(do_gauss);
}

int Testbench::read_bmp(string infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "rb");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }
  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&input_rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bits_per_pixel, sizeof(unsigned short), 1, fp_s));
  bytes_per_pixel = bits_per_pixel / 8;

  // move offset to input_rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, input_rgb_raw_data_offset, SEEK_SET);

  source_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }

  target_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    return -1;
  }

  assert(fread(source_bitmap, sizeof(unsigned char),
               (size_t)(long)width * height * bytes_per_pixel, fp_s));
  fclose(fp_s);
  return 0;
}

int Testbench::write_bmp(string outfile_name) {
  FILE *fp_t = NULL;      // target file handler
  unsigned int file_size; // file size

  fp_t = fopen(outfile_name.c_str(), "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // file size
  file_size = width * height * bytes_per_pixel + output_rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bits_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), output_rgb_raw_data_offset, fp_t);

  // write image
  fwrite(target_bitmap, sizeof(unsigned char),
         (size_t)(long)width * height * bytes_per_pixel, fp_t);

  fclose(fp_t);
  return 0;
}

void Testbench::do_gauss() {
  int x, y, a, b;               // for loop counter
  unsigned char R, G, B; // color of R, G, B
//  int adjustX, adjustY, xBound, yBound;
  int r_total, g_total, b_total;

  word data;
  unsigned char mask[4];
  wait(5 * CLOCK_PERIOD, SC_NS);
  for (y = 0; y != height; ++y) {
    for (x = 0; x != width; ++x) {
      R = *(source_bitmap + bytes_per_pixel * (width * y + x) + 2);
      G = *(source_bitmap + bytes_per_pixel * (width * y + x) + 1);
      B = *(source_bitmap + bytes_per_pixel * (width * y + x) + 0); 
      data.uc[0] = R;
      data.uc[1] = G;
      data.uc[2] = B;
      mask[0] = 0xff;
      mask[1] = 0xff;
      mask[2] = 0xff;
      mask[3] = 0;
      initiator.write_to_socket(GAUSS_FILTER_R_ADDR, mask, data.uc, 4);
// std::cout << "sent = " << int(R) << ", " << int(G) << ", " << int(B) << std::endl;
    }
  }
  for (b = 0; b != height; ++b) {
    for (a = 0; a != width; ++a) {  
      initiator.read_from_socket(GAUSS_FILTER_RESULT_ADDR, mask, data.uc, 4);
      r_total = (int)(data.uc[0]);
      g_total = (int)(data.uc[1]);
      b_total = (int)(data.uc[2]);
// std::cout << "total = " << r_total << ", " << g_total  << ", " << b_total << std::endl;    

      *(target_bitmap + bytes_per_pixel * (width * b + a) + 2) = r_total;
      *(target_bitmap + bytes_per_pixel * (width * b + a) + 1) = g_total;
      *(target_bitmap + bytes_per_pixel * (width * b + a) + 0) = b_total;  
    }
  }
  sc_stop();
}
