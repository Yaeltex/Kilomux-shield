
/* * * * * * * * * * * * * * * * * * * * * * *
* Code by Martin Sebastian Wain for YAELTEX *
* contact@2bam.com                     2016 *
* * * * * * * * * * * * * * * * * * * * * * */

#include "KM_Data.h"


namespace KMS {

	namespace priv {
		int _inputsOffset;
		int _outputsOffset;
		byte _currBank;
		byte _maxBank;
		int _bankLen;
		int _bankOffset;

		byte _data[KMS_MAX_DATA];		//RAM copy of data
	};
	using namespace priv;

	EEPROM_IO io;

	void setBank(byte bank) {
		if(bank >= _maxBank)
			bank = _maxBank - 1;	//clamp
		_bankOffset = GlobalData::length + bank * _bankLen;
		_currBank = bank;
	}

	void initialize() {
		/*byte b;
		io.read(0, &b, 1);
		isOutputMatrix = b == 1;
		io.read(1, input_us._p, InputUS::length);
		int i;
		for(i = 0; i<NUM_INPUTS; i++)
			io.read(1 + InputUS::length + i*InputNorm::length, inputs_norm[i]._p, InputNorm::length);
		for(i = 0; i<NUM_OUTPUTS; i++)
			io.read(1 + InputUS::length + NUM_INPUTS * InputNorm::length + i * Output::length, outputs[i]._p, Output::length);
		*/


		//First load the header, and then load the banks appropriately
		io.read(0, _data, GlobalData::length);
		GlobalData gd = globalData();

		//If data is present and ok...
		if(gd.isValid() && gd.protocolVersion() == PROTOCOL_VERSION) {
			//Calculate offsets and sizes from the variable-size bank configuration header
			_inputsOffset = InputUS::length;									//Inputs offset: After an input for ultrasound
			_outputsOffset = _inputsOffset + gd.numInputsNorm() * InputNorm::length;	//Outputs offset: After all inputs
			_bankLen = _outputsOffset + gd.numOutputs() * Output::length;
			DEBUG_PRINT("Bank len", _bankLen);

			byte userBanks = gd.numBanks();

			//Clamp to safe real banks value
			int maxBank = (KMS_MAX_DATA - KMS_DATA_START - GlobalData::length) / _bankLen;

			//Clamp to 0-255
			if(maxBank > 0xff)
				maxBank = 0xff;

			//Clamp to user-selected banks if less than phyisical limit
			if(maxBank > userBanks)
				maxBank = userBanks;

			_maxBank = maxBank;

			//Read the rest of the configuration to RAM
			int restBytes = maxBank * _bankLen;
			DEBUG_PRINT("Rest", restBytes);
			io.read(GlobalData::length, _data + GlobalData::length, restBytes);

			//Set the default bank
			setBank(0);
		}
		else {
			_inputsOffset = _outputsOffset = _bankLen = _bankOffset = 0;
			_maxBank = _currBank = 0;
		}
	}


} //namespace KMS
