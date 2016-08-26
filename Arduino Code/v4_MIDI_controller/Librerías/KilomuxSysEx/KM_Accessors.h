
/* * * * * * * * * * * * * * * * * * * * * * *
  Code by Martin Sebastian Wain for YAELTEX
  contact@2bam.com                     2016
* * * * * * * * * * * * * * * * * * * * * * */

#ifndef _KM_ACCESSORS_H_
#define _KM_ACCESSORS_H_

namespace KMS {
class GlobalData {
    //0-2  3                 4-11          12                 13         14                        15
    //YTX, protocol_version, [8 reserved], [output_matrix:1], num_banks, num_input_norms per bank, num_outputs per bank
    byte *_p;
  public:
    static const int length = 16;
    GlobalData(byte *ptr) : _p(ptr) {}

    //Check if the loaded data is valid
    bool isValid() const {
      //Eventually a CRC check could be here (saved in the reserved bytes)
      return _p[0] == 'Y' && _p[1] == 'T' && _p[2] == 'X';
    }

    //Protocol version. Should be the same as KMS::PROTOCOL_VERSION in KM_Data.h
    byte protocolVersion() const {
      return _p[3];
    }

    //Has an output LED matrix?
    bool hasOutputMatrix() const {
      return (_p[12] & 1) != 0;
    }

    //Number of banks
    byte numBanks() const {
      return _p[13];
    }

    //Number of normal inputs per bank
    byte numInputsNorm() const {
      return _p[14];
    }

    //Number of outputs per bank
    byte numOutputs() const {
      return _p[15];
    }
};

class InputBase {
    // BYTE 0 -----------------//BYTE 1 -----------------------------//BYTE 2 -------------//BYTE 3 --//BYTE 4 --//BYTE 5 ---------------//
    // 0-2              3-6      8-14                                  16-23                 24-30      32-38      40              41
    // tipo de entrada, channel, num parametro(note,cc,nrprn,shifter), num parametro(nrprn), valor_min, valor_max, analog/digital, toggle
    friend class InputNorm;
    friend class InputUS;
  protected:
    byte *_p;
    InputBase(byte *ptr) : _p(ptr) {}
  public:

    byte mode() const {
      return _p[0] & B111;
    }

    byte channel() const {
      return (_p[0] & B1111000) >> 3;
    }

    //Param (For Note, CC, NRPN, Shifter (bank))
    byte param_coarse() const {
      return _p[1];
    }

    //Param fine part For NRPN
    byte param_fine() const {
      return _p[2];
    }

    //Full param for NRPN (convenience method)
    int param_nrpn() const {
      param_coarse() << 7 | param_fine();
    }

    //Param min (coarse part if NRPN, should shift 7 bits)
    byte param_min_coarse() const {
      return _p[3];
    }

    //Param max (coarse part if NRPN, should shift 7 bits)
    byte param_max_coarse() const {
      return _p[4];
    }

};

class InputNorm : public InputBase {
  public:
    static const int length = 6;
    InputNorm(byte *ptr) : InputBase(ptr) {}

    bool analog() const {
      return (_p[5] & 1) != 0;
    }
};

class InputUS : public InputBase {
    // BYTE 0 -----------------//BYTE 1 -----------------------------//BYTE 2 -------------//BYTE 3 --//BYTE 4 --//
    // 0-2              3-6      8-14                                  16-23                 24-30      32-38      
    // tipo de entrada, channel, num parametro(note,cc,nrprn,shifter), num parametro(nrprn), valor_min, valor_max, 

    // BYTE 4 --------//BYTE 5 --------//BYTE 7 --------//BYTE 8 --------//
    // 40-46            48-54            55-61            64-70
    // distancia_min_H, distancia_min_L, distancia_max_H, distancia_max_L
  public:
    static const int length = 9;
    InputUS(byte *ptr) : InputBase(ptr) {}

    int dist_min() const {
      _p[5] << 7 | _p[6];
    }
    int dist_max() const {
      _p[7] << 7 | _p[8];
    }
};


class Output {
    // BYTE 0 -----------//BYTE 1 -//BYTE 2 -//BYTE 3 -//
    // 0      3-6      8-14      16-23      24-30      
    // blink, channel, num note, blink_min, blink_max
    byte *_p;
  public:
    static const int length = 4;
    Output(byte *ptr) : _p(ptr) {}

    bool blink() const {
      return (_p[0] & 1) != 0;
    }
    byte channel() const {
      return (_p[0] & B1111000) >> 3;
    }
    byte note() const {
      return _p[1];
    }
    byte blink_min() const {
      return _p[2];
    }
    byte blink_max() const {
      return _p[3];
    }
};
} //namespace KMS
#endif // _KM_ACCESSORS_H_
