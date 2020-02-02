#include <stdio.h>
#include <stdlib.h>

#include <kernel.h>
#include <pad.h>
#include <systemservice.h>

#include <orbis2d.h>
#include <orbisAudio.h>
#include <orbisPad.h>
#include <orbisKeyboard.h>
#include <orbisXbmFont.h>

#include <debugnet.h>
#include <ps4link.h>

int flag = 0;
int64_t flipArg = 0;

Orbis2dConfig *conf;
OrbisPadConfig *confPad;

typedef struct OrbisGlobalConf {
  Orbis2dConfig *conf;
  OrbisPadConfig *confPad;
  OrbisAudioConfig *confAudio;
  OrbisKeyboardConfig *confKeyboard;
  ps4LinkConfiguration *confLink;
  int orbisLinkFlag;
} OrbisGlobalConf;

OrbisGlobalConf *myConf;
ScePadVibrationParam vibra;

void updateController() {
  int ret;

  ret = orbisPadUpdate();
  if (ret == 0) {
    if (orbisPadGetButtonPressed(ORBISPAD_OPTIONS | ORBISPAD_L3) || orbisPadGetButtonHold(ORBISPAD_OPTIONS | ORBISPAD_L3)) {
      flag = 0;
    }

    if (orbisPadGetButtonPressed(ORBISPAD_UP) || orbisPadGetButtonHold(ORBISPAD_UP)) {
      if (vibra.largeMotor < 255) {
        vibra.largeMotor++;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_DOWN) || orbisPadGetButtonHold(ORBISPAD_DOWN)) {
      if (vibra.largeMotor > 0) {
        vibra.largeMotor--;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_RIGHT) || orbisPadGetButtonHold(ORBISPAD_RIGHT)) {
      if (vibra.smallMotor < 255) {
        vibra.smallMotor++;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_LEFT) || orbisPadGetButtonHold(ORBISPAD_LEFT)) {
      if (vibra.smallMotor > 0) {
        vibra.smallMotor--;
      }
    }
  }
}

void finishApp() {
  orbisPadFinish();
  orbis2dFinish();
  ps4LinkFinish();
}

void initApp() {
  int ret;

  ret = orbisPadInitWithConf(myConf->confPad);
  if (ret == 1) {
    confPad = orbisPadGetConf();
    ret = orbis2dInitWithConf(myConf->conf);
    if (ret == 1) {
      conf = orbis2dGetConf();
      flag = 1;
    }
  }

  sceSystemServiceHideSplashScreen();
}

int main(int argc, char *argv[]) {
  int ret;
  uint32_t c1, c2;

  uintptr_t intptr = 0;
  sscanf(argv[1], "%p", &intptr);
  myConf = (OrbisGlobalConf *)intptr;
  ret = ps4LinkInitWithConf(myConf->confLink);
  if (!ret) {
    ps4LinkFinish();
    return 0;
  }
  debugNetPrintf(DEBUG, "Initializing App\n");
  initApp();

  char largeMotorStatus[24];
  char smallMotorStatus[24];

  c1 = 0xFFFFFFFF;
  c2 = 0xFFFFFFFF;
  update_gradient(&c1, &c2);

  conf->bgColor = 0xFF000000;
  ScePadLightBarParam lightbarColor;
  lightbarColor.r = 190;
  lightbarColor.g = 122;
  lightbarColor.b = 169;
  scePadSetLightBar(myConf->confPad->padHandle, &lightbarColor);

  while (flag) {
    updateController();

    orbis2dStartDrawing();
    orbis2dClearBuffer(0);

    sprintf(largeMotorStatus, "Large Motor Speed: %i", vibra.largeMotor);
    sprintf(smallMotorStatus, "Small Motor Speed: %i", vibra.smallMotor);

    print_text(20, 20, "PSVibe: PS4 Edition by Al Azif");
    print_text(20, 60, "Press UP/DOWN to control large motor speed");
    print_text(20, 80, "Press LEFT/RIGHT to control small motor speed");
    print_text(20, 120, largeMotorStatus);
    print_text(20, 140, smallMotorStatus);
    print_text(20, 180, "Press OPTIONS + L3 to exit");

    scePadSetVibration(myConf->confPad->padHandle, &vibra);

    orbis2dFinishDrawing(flipArg);
    orbis2dSwapBuffers();
    flipArg++;

    sceKernelUsleep(1000);
  }

  vibra.largeMotor = 0;
  vibra.smallMotor = 0;
  scePadSetVibration(myConf->confPad->padHandle, &vibra);

  orbis2dStartDrawing();
  orbis2dClearBuffer(1);
  orbis2dFinishDrawing(flipArg);
  orbis2dSwapBuffers();

  finishApp();
  myConf->orbisLinkFlag = 1;
  exit(EXIT_SUCCESS);
}
