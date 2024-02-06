#include <DNNRT.h>
#include <SDHCI.h>
#include <Camera.h>
#include <ELTRES.h>

#define LFOUR_ID_LOWER     0x000C
#define BAUDRATE           (115200)
#define OUT_WIDTH          (28)
#define OUT_HEIGHT         (28)

void send_eltres(uint8_t index) {
  Eltres *eltres = new Eltres();

  eltres->init(0);
  uint32_t val;
  eltres->eeprom_read(LFOUR_ID_LOWER, &val);
  Serial.print("LFOUR ID: 0x");
  Serial.println(val, HEX);
//
//  Serial.println("Waiting for ELTRES interrupt");
  eltres->wait_interrupt();
  eltres->get_gga_position();
  eltres->get_gnss_status();
  
  uint32_t lat = (uint32_t)((eltres->pos.lat * 3600 * 1000 + 324000000) / 32);
  uint32_t lon = (uint32_t)((eltres->pos.lon * 3600 * 1000 + 648000000) / 32);
  uint32_t alt = (uint32_t)((eltres->pos.alt + 1000 + 0.5) / 1);
  eltres->set_bit_value(eltres->payload, lat, 8, 25);
  eltres->set_bit_value(eltres->payload, lon, 8 + 25, 26);
  eltres->set_bit_value(eltres->payload, alt, 8 + 25 + 26, 14);
  eltres->set_bit_value(eltres->payload, index, 8 + 25 + 26 + 14, 8);

  char s[16 * 2] = {0};
  Serial.print("Eltres Payload: ");
  for (int i = 0; i < 16; i++) {
    sprintf(&s[i * 2], "%02X", eltres->payload[i]);
  }
  
  Serial.println(s);

  eltres->send();

  delete eltres;
}

void get_image(CamImage *dest) {
  theCamera.begin();
  theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QUADVGA_H,
    CAM_IMGSIZE_QUADVGA_V,
    CAM_IMAGE_PIX_FMT_JPG);

  CamImage img = theCamera.takePicture();
  if (img.isAvailable()) {
    Serial.println("Picture taken!");
  }
  theCamera.end();

  img.resizeImageByHW(*dest, OUT_WIDTH, OUT_HEIGHT);
}

uint8_t get_inference(CamImage *img) {
  SDClass theSD;
  DNNRT dnnrt;
  DNNVariable *input = new DNNVariable(OUT_WIDTH * OUT_HEIGHT);

  while (!theSD.begin());

  // Load Neural Network
  File nnbfile = theSD.open("model.nnb");
//  if (!nnbfile) {
//    Serial.print("nnb not found");
//    return;
//  }
  dnnrt.begin(nnbfile);
//  if (ret < 0) {
//    Serial.println("Runtime initialization failure.");
//    if (ret == -16) {
//      Serial.print("Please install bootloader!");
//      Serial.println(" or consider memory configuration!");
//    } else {
//      Serial.println(ret);
//    }
//    return;
//  }
  nnbfile.close();

  // Inference
  uint16_t* imgbuf = (uint16_t*)(img->getImgBuff());
  float *dnnbuf = input->data();
  // Grayscale then Normalize
  int n = 0;
  for (n = 0; n < OUT_WIDTH * OUT_HEIGHT; ++n) {
    dnnbuf[n] = (float)(((imgbuf[n] & 0xf000) >> 8)
                        | ((imgbuf[n] & 0x00f0) >> 4)) / 255.0;
  }

  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);
  uint8_t index = output.maxIndex();
  Serial.print("Inference: ");
  Serial.println(index);
  
  dnnrt.end();
  delete input;
  
  return index;
}

/**
   @brief Initialize camera
*/void setup()
{
  Serial.begin(BAUDRATE);

  CamImage *img = new CamImage();
  get_image(img);
  uint8_t index = get_inference(img);
  delete img;

  // Eltres library consumes a lot of memory
  send_eltres(index);
}

void loop()
{

}