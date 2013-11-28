
#ifndef RECORDER_HPP
#define RECORDER_HPP


#include <stdio.h>
#include <stdlib.h>
#include <comedilib.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

template <class Type>

class Recorder{

	public:

		Recorder():
			_sample_rate(0), _channel_amount(0),_device(""){
			}


		int setup_Device(char* arg_device, int arg_channel_amount, 
				float arg_sample_rate ){

			_channel_amount = arg_channel_amount;

			unsigned int chanlist[_channel_amount];
			unsigned int convert_arg = (unsigned int) 1e9 / _sample_rate;
			int ret;

			comedi_cmd *cmd = (comedi_cmd *) calloc(1, sizeof(comedi_cmd));
			_device = comedi_open(arg_device);

			if(!_device){
				errx(1, "unable to open device");
				comedi_perror("/dev/comedi0");
				return -1;
			}

			for(int i = 0; i < _channel_amount; i++)	
				chanlist[i] = i;

			comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

			if((ret = comedi_get_cmd_generic_timed(_device, 0, cmd, _channel_amount,0)) < 0){
				printf("comedi_get_cmd_generic_timed failed\n");
				return ret;
			}

			cmd->chanlist 	= chanlist;
			cmd->stop_src	= TRIG_NONE;
			cmd->stop_arg	= 0;
			cmd->convert_arg = convert_arg;

			/* call test twice because different things are tested?
			 * if tests are successful run sampling command */
			if(   (ret = comedi_command_test(_device, cmd)) != 0
					|| (ret = comedi_command_test(_device, cmd)) != 0
					|| (ret = comedi_command(_device, cmd))       < 0){
				fprintf(stderr, "err: %d\n", ret);
				exit(1);
			}

			return 0;
		}


		void start_Sampling(int arg_buffer_size, Type *arg_start_address){
			int ret;
			unsigned int index;
			FILE* dux_fp;
			Type* write_address = arg_start_address;

			if((dux_fp = fopen(comedi_fileno(_device), "r"))<= 0)
				comedi_perror("fdopen");

			while((ret = fread(arg_start_address, 1, sizeof(Type),dux_fp)) >= 0){
				write_address++;
				index++;
				if(index > arg_buffer_size)
					write_address = arg_start_address;
				
			}
		}


	private:

		float _sample_rate;
		int _channel_amount;
		FILE* _dux_fp;
		comedi_t* _device;
};
#endif // RECORDER_HPP