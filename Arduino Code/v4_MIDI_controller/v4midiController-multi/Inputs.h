#ifndef _INPUTS_H_
#define _INPUTS_H_

#include <Kilomux.h>
#include <KM_Data.h>

static const char * const modeLabels[] = {
    "Off"
  , "Note"
  , "CC"
  , "NRPN"
  , "Shifter"
};
#define MODE_LABEL(m)   ((m) <= KMS::M_SHIFTER ? modeLabels[m] : "???")

// AJUSTABLE - Si hay ruido que varía entre valores (+- 1, +- 2, +- 3...) colocar el umbral en (1, 2, 3...)
#define NOISE_THRESHOLD             1                      // Ventana de ruido para las entradas analógicas. Si entrada < entrada+umbral o 
#define NOISE_THRESHOLD_NRPN        1                      // Ventana de ruido para las entradas analógicas. Si entrada < entrada+umbral o 

#define ANALOGO_UP     1
#define ANALOGO_DOWN   0

bool digitalInputState[NUM_MUX*NUM_MUX_CHANNELS];   // Estado de los botones
extern Kilomux KMShield;
extern byte numInputs;

void ReadInputs(void);
void InputChanged(const KMS::InputNorm &input, unsigned int );
unsigned int IsNoise(unsigned int, unsigned int, unsigned int);

#endif
