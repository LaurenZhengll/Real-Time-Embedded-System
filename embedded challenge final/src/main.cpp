#include "mbed.h"

#define MAX 60  // 20s * 95 / 32
#define PI 3.14159

SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1);
InterruptIn int2(PA_2);  // set up int2

volatile int flag = 0;

void readFIFO() {  // if int2 is triggered, set flag to 1 to read data from the gyroscope output registers
	flag = 1;
}

// Documents
// Manual for dev board: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// gyroscope datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf

// Read a register on the gyroscope
int main() {
  // Chip must be deselected
  cs = 1;
 
  // Setup the spi for 8 bit data, high steady state clock,
  // second edge capture, with a 1MHz clock rate
  spi.format(8,3);  //mode 3
  spi.frequency(1000000);  //clock change 1000000 times per second

  

  // A FIFO mode application hint is given below:
  // 1. Set FIFO_EN = 1: Enable FIFO
  // 2. Set FM[2:0] = (0,0,1): Enable FIFO mode
  // 3. Wait for the OVRN or WTM interrupt
  // 4. Read data from the gyroscope output registers
  // 5. Set FM[2:0] = (0,0,0): Enable Bypass mode
  // 6. Repeat from step 2

  //Enable FIFO
  cs = 0;
  spi.write(0x24); //write Reg5
  spi.write(0xC0); //set FIFO_EN=1 BOOT=1
  cs = 1;

  cs = 0;
  spi.write(0x20);  // write:0 0 PD: 20h:100000
  spi.write(0x0F);  //change PD from power-down 00000111 to normal mode 00001111
  cs = 1;

  //set FIFO Bypass mode
  cs = 0;
  spi.write(0x2E); //FIFO_control_reg
  spi.write(0x0F); //set as Bypass mode 00001111
  cs = 1;

  int2.rise(&readFIFO); // set monitor on int2, if int2 is triggered, call readFIFO()
  int count = 0;
  float w[MAX];
  float degree[MAX];  // degree changed
  float d[MAX - 1];
  float peak[MAX] = {0};  // find peak value of degree
  float disOnePace = 0;
  float distanceTotal = 0;

  while(count < MAX){
    //set FIFO as FIFO mode
    cs = 0;
    spi.write(0x2E); //FIFO_control_reg
    spi.write(0x2F); //set as FIFO mode 0010 1111
    cs = 1;

    //Wait for the OVRN from FIFO_SRC_REG
    cs = 0;
    spi.write(0xAF);  //write FIFO_SRC_REG (2Fh)
    int FIFO_src_reg = spi.write(0x00);  // read from FIFO_SRC_REG  
    cs = 1;

    // if overrun, means FIFO is completely filled
    if(FIFO_src_reg & (1<<6)){  //bit 6: Overrun bit status. 1:FIFO is completely filled
      //set reg3 to trigger a FIFO overrun interrupt on INT2
      cs = 0;
      spi.write(0x22);  //write reg3 (22h) 
      spi.write(0x02);  //set up bit 2 I2_ORun to trigger a FIFO overrun interrupt on INT2
      cs = 1;

      if(flag == 1){  //if interrupt is triggered, read from output registers
        int xl[32];
        int xh[32];
        int16_t x[32];
        int yl[32];
        int yh[32];
        int16_t y[32];
        int zl[32];
        int zh[32];
        int16_t z[32];
        int32_t zSum = 0;
        for(int i = 0; i<32; i++){
          //read OUT_X_L and OUT_X_H
          cs = 0;
          spi.write(0xA8); //OUT_X_L:1 0 28h:101000
          xl[i] = spi.write(0x00);
          cs = 1;
          cs = 0;
          spi.write(0xA9); //OUT_X_H:1 0 29h:101001                
          xh[i] = spi.write(0x00);        
          cs = 1;
          x[i] = xl[i] | (xh[i] << 8); //combine high and low

          //read OUT_Y_L and OUT_Y_H
          cs = 0;
          spi.write(0xAA); //OUT_Y_L:1 0 2Ah:101010
          yl[i] = spi.write(0x00);
          cs = 1;
          cs = 0;
          spi.write(0xAB); //OUT_Y_H:1 0 2Bh:101011                
          yh[i] = spi.write(0x00);        
          cs = 1;
          y[i] = yl[i] | (yh[i] << 8); //combine high and low

          //read OUT_Z_L and OUT_Z_H
          cs = 0;
          spi.write(0xAC); //OUT_Z_L:1 0 2Ch:101100
          zl[i] = spi.write(0x00);
          cs = 1;
          cs = 0;
          spi.write(0xAD); //OUT_Z_H: 10 2Dh:101101              
          zh[i] = spi.write(0x00);        
          cs = 1;
          z[i] = zl[i] | (zh[i] << 8); //combine high and low
          printf("%f, ", 0.00875 * z[i]);  // angular velocity
          zSum = zSum + z[i];  // sum of z of 32 data
        }
        w[count] = 0.01015 * 0.00875 * zSum;  //when FS = 250dps 8.75mdps/digit, so angular velocity is 0.00875*z
        //printf("%d",count);
        //printf("%f, ", w[count]);  // degree in 32/95 seconds
        //printf("\n");
        count++;

        flag = 0;
      }
      //enable FIFO Bypass mode
      cs = 0;
      spi.write(0x2E); //FIFO_control_reg
      spi.write(0x0F); //set as Bypass mode 0000 1111
      cs = 1;
    }
  }
  printf("\n\n");

  for (int i = 0; i < MAX; i++){
    for(int j = 0; j < i; j++){
        degree[i] = degree[i] + w[j];  // degree in 20 seconds
    }
  }
  for(int i = 0; i < MAX; i++){
      printf("%f, ", degree[i]);  // degree every 0.3s
  }
  printf("\n");

  for(int i = 0; i < MAX - 1; i++){
      d[i] = degree[i+1] - degree[i]; // difference in degree between every 0.3s
  }

  //calculate degree in one pace by extract peak degree values
  int m = 0;
  int legLength = 90;  //leg length centimeter 
  int flg = d[0]<0?1:0; //find min peak
  
  for(int i = 0; i < MAX - 1; i++){
    //find min peak
    if(flg){
      if(d[i]<=5){
        continue;
      }  
      else{  //found min degree
        peak[m] = degree[i];
        printf("peak[%d] = %f\n", m, peak[m]);
        m++;
        flg = 0;
      }
    }
    //find max peak
    else{
      if (d[i]>=5){
        continue;
      }
      else{
        peak[m] = degree[i];
        printf("peak[%d] = %f\n", m, peak[m]);
        m++;
        flg = 1;
      }
    }
  }

  for(int i = 0; i < MAX; i++){
    if(peak[i]){
      double arg = (peak[i]-peak[i+1])/180*PI;  //changed degree presented in PI
      disOnePace = 2*pow(legLength,2)-2*pow(legLength,2)*cos(arg);  // distance of every step according to changed degree
      disOnePace = sqrt(disOnePace); 
      distanceTotal = distanceTotal + disOnePace;  //accumulate total distant
      printf("distance: %f\n",1.33*distanceTotal);
    }
  }
}
