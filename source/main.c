#include <stdio.h>
#include <stdlib.h>

#include <errordialog.h>
#include <kernel.h>
#include <pad.h>
#include <sysmodule.h>
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
      vibra.largeMotor = 0;
      vibra.smallMotor = 0;
      flag = 0;
    }

    if (orbisPadGetButtonPressed(ORBISPAD_UP)) {
      if (vibra.largeMotor < 255) {
        vibra.largeMotor++;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_DOWN)) {
      if (vibra.largeMotor > 0) {
        vibra.largeMotor--;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_RIGHT)) {
      if (vibra.smallMotor < 255) {
        vibra.smallMotor++;
      }
    }

    if (orbisPadGetButtonPressed(ORBISPAD_LEFT)) {
      if (vibra.smallMotor > 0) {
        vibra.smallMotor--;
      }
    }

    if (orbisPadGetButtonHold(ORBISPAD_UP)) {
      if (vibra.largeMotor < 255) {
        vibra.largeMotor++;
      }
    }

    if (orbisPadGetButtonHold(ORBISPAD_DOWN)) {
      if (vibra.largeMotor > 0) {
        vibra.largeMotor--;
      }
    }

    if (orbisPadGetButtonHold(ORBISPAD_RIGHT)) {
        if (vibra.smallMotor < 255) {
          vibra.smallMotor++;
        }
    }

    if (orbisPadGetButtonHold(ORBISPAD_LEFT)) {
      if (vibra.smallMotor > 0) {
        vibra.smallMotor--;
      }
    }
  }
}

void errorDialog(SceUserServiceUserId userId, int32_t errorCode) {
  int ret;

  sceSystemServiceHideSplashScreen();

  ret = sceSysmoduleLoadModule(SCE_SYSMODULE_ERROR_DIALOG);
  if (ret != 0) {
    return;
  }

  sceErrorDialogInitialize();
  if (ret != 0 && ret != 0x80ED0002) {
    return;
  }

  SceErrorDialogParam param;
  sceErrorDialogParamInitialize(&param);

  param.errorCode = errorCode;
	param.userId = userId;

  ret = sceErrorDialogOpen(&param);
  if (ret != 0) {
    return;
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

  char title[64] = "PSVibe: PS4 Edition by Al Azif";

  char line1[64] = "Press UP/DOWN to control large motor speed";
  char line2[64] = "Press LEFT/RIGHT to control small motor speed";

  char line3[64];
  char line4[64];

  char line5[64] = "Press OPTIONS + L3 to exit";

  c1 = 0xFFFFFFFF;
	c2 = 0xFFFFFFFF;
	update_gradient(&c1, &c2);

  conf->bgColor = 0xFF000000;

  while (flag) {
    updateController();

    orbis2dStartDrawing();
    orbis2dClearBuffer(0);

    sprintf(line3, "Large Motor Speed: %i", vibra.largeMotor);
    sprintf(line4, "Small Motor Speed: %i", vibra.smallMotor);
    print_text(20, 20, title);
    print_text(20, 60, line1);
    print_text(20, 80, line2);
    print_text(20, 120, line3);
    print_text(20, 140, line4);
    print_text(20, 180, line5);

    scePadSetVibration(myConf->confPad->padHandle, &vibra);

    orbis2dFinishDrawing(flipArg);
    orbis2dSwapBuffers();
    flipArg++;

    sceKernelUsleep(1000);
  }

  scePadSetVibration(myConf->confPad->padHandle, &vibra);

  orbis2dStartDrawing();
  orbis2dClearBuffer(1);
  orbis2dFinishDrawing(flipArg);
  orbis2dSwapBuffers();

  finishApp();
  myConf->orbisLinkFlag = 1;
  exit(EXIT_SUCCESS);
}
