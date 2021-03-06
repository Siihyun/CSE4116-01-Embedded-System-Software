
#ifndef __FPGA_NUMBER__
#define __FPGA_NUMBER__

unsigned char fpga_number[3][10] = {
	{0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63}, // A
	{0x0c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3f,0x3f}, // 1
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // 0
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};

unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
unsigned char map[5][10][7] = { 
    {
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1},
    {1,0,0,0,0,0,1}
  },
  { 
    {1,0,1,1,1,1,1},
    {1,0,0,0,0,0,1},
    {1,0,1,0,0,0,1},
    {1,0,1,0,0,0,1},
    {1,0,1,0,0,0,1},
    {1,0,1,0,0,0,1},
    {1,0,1,1,1,0,1},
    {1,0,0,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1}
  },
  { 
    {1,0,0,0,0,0,1},
    {1,0,1,1,1,1,1},
    {1,0,0,0,0,0,1},
    {1,1,1,1,1,0,1},
    {1,0,0,0,0,0,1},
    {1,0,1,1,1,1,1},
    {1,0,0,0,0,0,1},
    {1,0,1,1,1,0,1},
    {1,0,0,0,1,0,1},
    {1,1,1,1,1,0,1}
  },
  { 
    {1,0,1,1,1,1,1},
    {1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1},
    {1,0,0,0,1,0,1},
    {1,1,1,1,1,0,1}
  },
  { 
    {1,1,1,0,1,1,1},
    {1,1,1,0,1,1,1},
    {1,0,0,0,0,0,1},
    {1,1,1,1,1,0,1},
    {1,0,0,0,0,0,1},
    {1,0,1,1,1,1,1},
    {1,0,1,1,0,0,1},
    {1,0,1,1,0,0,1},
    {1,0,0,0,0,0,1},
    {1,1,1,1,1,0,1}
  }
};

#endif
